/**
 * @file:   k_process.c
 * @brief:  process management C file
 * @author: Yiqing Huang
 * @author: Thomas Reidemeister
 * @date:   2014/01/17
 * NOTE: The example code shows one way of implementing context switching.
 *       The code only has minimal sanity check. There is no stack overflow check.
 *       The implementation assumes only two simple user processes and NO HARDWARE INTERRUPTS.
 *       The purpose is to show how context switch could be done under stated assumptions.
 *       These assumptions are not true in the required RTX Project!!!
 *       If you decide to use this piece of code, you need to understand the assumptions and
 *       the limitations.
 */

#include <LPC17xx.h>
#include <system_LPC17xx.h>
#include "uart_polling.h"
#include "k_process.h"
#include "k_memory.h"
#include "heap.h"
#include "heap_queue.h"
#include "procs/timer.h"
#include "hot_key_helper.h"

#ifdef DEBUG_0
#include "printf.h"
#endif

// Currently running process
PCB* g_current_process = NULL;
PCB* g_ready_process_priority_queue[PROCESS_PRIORITY_NUM] = {NULL};
// User process initial xPSR value
#define INITIAL_xPSR 0x01000000

// Note: This must be called during system initialization, before
// heap_init() is called (we don't yet have dynamic processes :(.
int process_create(ProcInit initial_state)
{
	PCB* pcb = memory_alloc_pcb();
	if (!pcb) {
		return RTX_ERR;
	}

	pcb->pid = initial_state.pid;
	pcb->state = PROCESS_STATE_NEW;
	pcb->priority = initial_state.priority;
	pcb->p_next = NULL;
	pcb->message_queue = NULL;

	if (initial_state.stack_size == 0) {
		// i-procs don't need their own stack, they just share the stack
		// with whatever process is currently running.
		pcb->sp = (U32*)0xFFFFFFFF;
		return RTX_OK;
	}

	// initilize exception stack frame (i.e. initial context)
	U32* sp = memory_alloc_stack(initial_state.stack_size);
	if (!sp) {
		return RTX_ERR;
	}

	// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0553a/Babefdjc.html
	// user process initial xPSR
	*(--sp) = INITIAL_xPSR;
	// PC contains the entry point of the process
	*(--sp) = (U32)initial_state.entry_point;
	// Our processes never exit. Set LR to 0x00000000
	// so accidental returns end up in the
	// HardFault_Handler.
	*(--sp) = 0x00000000;
	// R0-R3, R12 are cleared with 0
	for (int j = 0; j < 5; j++) {
		*(--sp) = 0x0;
	}
	pcb->sp = sp;

	// LOG("Created process! SP: %x", pcb->sp);

	return priority_queue_insert(g_ready_process_priority_queue, pcb);
}

/*@brief: scheduler, pick the pid of the next to run process
 *@return: PCB pointer of the next to run process
 *         NULL if error happens
 */
static PCB* scheduler(void)
{
 	PCB* next_process = priority_queue_pop(g_ready_process_priority_queue);

	if (next_process == NULL) {
		LOG("Warning: No processes on ready queue.\n");
	} else {
		// LOG("next_process id is: %d", next_process->pid);
	}

	return next_process;
}

static int switch_to_process(PCB* new_proc)
{
	//LOG("About to proccess_switch");
	if (new_proc == NULL) {
		LOG("NULL passed to switch_to_process!");
		return RTX_ERR;
	}

	ProcessState state = new_proc->state;
	if (state != PROCESS_STATE_READY && state != PROCESS_STATE_NEW) {
		LOG("Invalid process state!");
		return RTX_ERR;
	}

	// LOG("before g_current_process if");
	if (g_current_process != NULL) {
		g_current_process->sp = (U32*) __get_MSP();
	}

	new_proc->state = PROCESS_STATE_RUNNING;
	__set_MSP((U32) new_proc->sp);

	g_current_process = new_proc;

	if (state == PROCESS_STATE_NEW) {
		// pop exception stack frame from the stack for a new processes
		extern void __rte(void);
		// Note: This actually causes us to start executing the procees
		// with crazy assembly magic (See HAL.c)!
		// LOG("About to __rte");
		__rte();
	}

	// Note: This return returns to the switched-to processes call stack,
	// not the calling processes call stack (although, when the original
	// process gets scheduled again, this will return to that process's
	// stack again). This gives us an illusion of processes, that most
	// code (even kernel code) doesn't have to worry about, but can be
	// a bit confusing when reading this function.
	// LOG("About to return");
	return RTX_OK;
}

// Preempts the currently running process if there is a higher
// priority process on the ready queue.
int process_prempt_if_necessary(void)
{
	PCB* top = priority_queue_top(g_ready_process_priority_queue);
	if (top == NULL) {
		return RTX_OK;
	}
	// Priorities have the inverse ordering of normal numbers,
	// so higher priority numbers are actually lower priority
	// processes.
	if (top->priority >= g_current_process->priority) {
		return RTX_OK;
	}

//	LOG("Premepting %d with %d", g_current_process->pid, top->pid);
	return k_release_processor();
}

PCB* process_find(int pid) {
	for (unsigned int i = 0; i < g_pcb_counter; i++) {
		if (s_pcb_allocations_start[i].pid == pid) {
			return &s_pcb_allocations_start[i];
		}
	}
	return NULL;
}

PCB** process_find_queue(int id) {
	PCB** queue = g_ready_process_priority_queue;
	PCB* pcb = priority_queue_find(queue, id);
	if (pcb != NULL) {
		return queue;
	}
	queue = g_blocked_process_priority_queue;
	pcb = priority_queue_find(queue, id);
	if (pcb != NULL) {
		return queue;
	}
	return NULL;
}

// Change a process's priority. Does not preempt.
int process_set_priority(PCB* pcb, ProcessPriority priority)
{
	if (pcb == NULL) {
		LOG("NULL PCB passed to process_set_priority!");
		return RTX_ERR;
	}

	if (priority < PROCESS_PRIORITY_SYSTEM_PROCESS || priority > PROCESS_PRIORITY_LOWEST) {
		LOG("Attempted to set priority to invalid value!");
		return RTX_ERR;
	}

	if (pcb->pid == g_current_process->pid) {
		g_current_process->priority = priority;
		return RTX_OK;
	}

	PCB** queue = process_find_queue(pcb->pid);
	if (queue == NULL) {
		// Not in any queue, probably BLOCKED_ON_MESSAGE.
		pcb->priority = priority;
		return RTX_OK;
	}

	PriorityStatus status = priority_queue_reprioritize(queue, pcb, priority);
	if (status == PRIORITY_STATUS_OK) {
		return RTX_OK;
	}

	return RTX_ERR;
}

int process_send_message(HeapBlock* block) {
	if (!block) {
		LOG("NULL block passed to process_send_message!");
		return RTX_ERR;
	}

	PCB* dest = process_find(block->header.dest_pid);
	if (!dest) {
		LOG("Destination process %d not found!", block->header.dest_pid);
		return RTX_ERR;
	}

	heap_queue_push(&dest->message_queue, block);
	if (dest->state != PROCESS_STATE_BLOCKED_ON_MESSAGE) {
		return RTX_OK;
	}

	dest->state = PROCESS_STATE_READY;
	int result = priority_queue_insert(g_ready_process_priority_queue, dest);
	if (result != PRIORITY_STATUS_OK) {
		LOG("Shit hit the fan. Priority queue insert issue.");
		return RTX_ERR;
	}
	return RTX_OK;
}

/**
 * @brief release_processor().
 * @return RTX_ERR on error and zero on success
 * POST: g_current_process gets updated to next to run process
 */
int k_release_processor(void)
{
	if (g_current_process->state == PROCESS_STATE_RUNNING) {
		g_current_process->state = PROCESS_STATE_READY;
		priority_queue_insert(g_ready_process_priority_queue, g_current_process);
	}

	PCB *new_proc = scheduler();

	if (new_proc == NULL) {
		LOG("No process to switch to.");
		return RTX_ERR;
	}

	int rtx_status = switch_to_process(new_proc);
	// LOG("About to return from k_release_processor().");
	return rtx_status;
}

bool pid_is_system(int pid) {
	return pid <= PROCESS_ID_NULL || pid >= PROCESS_ID_KCD;
}

ProcessPriority user_priority_to_system_priority(UserProcessPriority user_priority) {
	return (ProcessPriority)(user_priority + PROCESS_PRIORITY_HIGH);
}

UserProcessPriority system_priority_to_user_priority(ProcessPriority system_priority) {
	return (UserProcessPriority)(system_priority - PROCESS_PRIORITY_HIGH);
}

int k_set_process_priority(int pid, int user_priority)
{
	// User exposed priorites should be 0-4.
	if (user_priority < USER_PROCESS_PRIORITY_HIGH || user_priority > USER_PROCESS_PRIORITY_LOWEST) {
		LOG("Attempted to set priority to invalid value!");
		return RTX_ERR;
	}

	if (pid_is_system(pid) && !pid_is_system(g_current_process->pid)) {
		LOG("User process %d attempted to set process priority of system process %d! Denied.", g_current_process->pid, pid);
		return RTX_ERR;
	}

	PCB* pcb = process_find(pid);
	if (pcb == NULL) {
		LOG("Attempted to set process priority of nonexistent process!");
		return RTX_ERR;
	}

	ProcessPriority priority = user_priority_to_system_priority((UserProcessPriority)user_priority);
	int status = process_set_priority(pcb, priority);
	if (status != RTX_OK) {
		return status;
	}

	return process_prempt_if_necessary();
}

int k_get_process_priority(int pid)
{
	PCB* proc = process_find(pid);
	if (proc == NULL) {
		return RTX_ERR;
	}

	return system_priority_to_user_priority(proc->priority);
}

void* k_receive_message(int* sender_pid)
{
	HeapBlock* block = heap_queue_pop(&g_current_process->message_queue);

	while (!block) {
		g_current_process->state = PROCESS_STATE_BLOCKED_ON_MESSAGE;
		k_release_processor();
		block = heap_queue_pop(&g_current_process->message_queue);
		if (block == NULL) {
			LOG("Warning: Blocked on message process scheduled to run when no messages in queue!");
		}
	}

	if (sender_pid) {
		*sender_pid = block->header.source_pid;
	}
	return user_block_from_heap_block(block);
}

int k_send_message(int dest_pid, void* msg)
{
	HeapBlock* block = heap_block_from_user_block(msg);
	block->header.source_pid = g_current_process->pid;
	block->header.dest_pid = dest_pid;

	int result = process_send_message(block);
	if (result != RTX_OK) {
		return RTX_ERR;
	}
	return process_prempt_if_necessary();
}

int k_delayed_send(int dest_pid, void *message_envelope, int delay_ms)
{
	PCB* pcb = process_find(dest_pid);
	if (pcb == NULL) {
		LOG("Attempted delayed_send to nonexistent process!");
		return RTX_ERR;
	}

	HeapBlock* block = heap_block_from_user_block(message_envelope);
	block->header.source_pid = g_current_process->pid;
	block->header.dest_pid = dest_pid;

	HeapQueueStatus status = timer_schedule_delayed_send(block, delay_ms);
	if (status != HEAP_QUEUE_STATUS_OK) {
		return RTX_ERR;
	}
	return RTX_OK;
}
