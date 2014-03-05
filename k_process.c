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
#include "timer.h"

#ifdef DEBUG_0
#include "printf.h"
#endif

// Currently running process
PCB* g_current_process = NULL;
PCB* g_ready_process_priority_queue[PROCESS_PRIORITY_NUM] = {NULL};
// User process initial xPSR value
#define INITIAL_xPSR 0x01000000

static void null_process()
{
	while (1) {
		k_release_processor();
		LOG("Running null process");
	}
}

void process_init()
{
	// Test process initial set up
	extern void set_test_procs(void);
	set_test_procs();

	// Set up NULL process
	PROC_INIT null_state;
	null_state.pid = (U32)(0);
	null_state.priority = PROCESS_PRIORITY_NULL_PROCESS;
	null_state.stack_size = 0x200;
	null_state.entry_point = &null_process;
	process_create(&null_state);

	extern PROC_INIT g_test_procs[NUM_TEST_PROCS];
	for (int i = 0; i < NUM_TEST_PROCS; i++) {
		process_create(&g_test_procs[i]);
	}
}

// Note: This must be called during system initialization, before
// heap_init() is called (we don't yet have dynamic processes :(.
int process_create(PROC_INIT* initial_state)
{
	PCB* pcb = memory_alloc_pcb();
	if (!pcb) {
		return RTX_ERR;
	}

	pcb->pid = initial_state->pid;
	pcb->state = PROCESS_STATE_NEW;
	pcb->priority = initial_state->priority;
	pcb->p_next = NULL;
	pcb->message_queue = NULL;

	// initilize exception stack frame (i.e. initial context)
	U32* sp = memory_alloc_stack(initial_state->stack_size);
	if (!sp) {
		return RTX_ERR;
	}

	// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0553a/Babefdjc.html
	// user process initial xPSR
	*(--sp) = INITIAL_xPSR;
	// PC contains the entry point of the process
	*(--sp) = (U32)initial_state->entry_point;
	// Our processes never exit. Set LR to 0x00000000
	// so accidental returns end up in the
	// HardFault_Handler.
	*(--sp) = 0x00000000;
	// R0-R3, R12 are cleared with 0
	for (int j = 0; j < 5; j++) {
		*(--sp) = 0x0;
	}
	pcb->sp = sp;

	LOG("Created process! SP: %x", pcb->sp);

	return priority_queue_insert(g_ready_process_priority_queue, pcb);
}

/*@brief: scheduler, pick the pid of the next to run process
 *@return: PCB pointer of the next to run process
 *         NULL if error happens
 */
static PCB* scheduler(void)
{
	PCB* next_process = priority_queue_pop(g_ready_process_priority_queue);

	if (next_process->pid == 9) {
		LOG("Unth");
	}
	if (next_process == NULL) {
		LOG("Warning: No processes on ready queue.\n");
	} else {
		LOG("next_process id is: %d", next_process->pid);
	}

	return next_process;
}

static int switch_to_process(PCB* new_proc)
{
	LOG("About to proccess_switch");
	if (new_proc == NULL) {
		LOG("NULL passed to switch_to_process!");
		return RTX_ERR;
	}

	ProcessState state = new_proc->state;
	if (state != PROCESS_STATE_READY && state != PROCESS_STATE_NEW) {
		LOG("Invalid process state!");
		return RTX_ERR;
	}

	LOG("before g_current_process if");
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
		LOG("About to __rte");
		__rte();
	}

	LOG("About to return");
	if (g_current_process->pid == 9) {
		LOG("Testing.");
	}

	// Note: This return returns to the switched-to processes call stack,
	// not the calling processes call stack (although, when the original
	// process gets scheduled again, this will return to that process's
	// stack again). This gives us an illusion of processes, that most
	// code (even kernel code) doesn't have to worry about, but can be
	// a bit confusing when reading this function.
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

	LOG("Premepting %d", g_current_process->pid);
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
	LOG("About to return from k_release_processor().");
	return rtx_status;
}

int k_set_process_priority(int id, int priority)
{
	if (priority < PROCESS_PRIORITY_HIGH || priority > PROCESS_PRIORITY_LOWEST) {
		LOG("Attempted to set priority to invalid value!");
		return RTX_ERR;
	}
	if (id == g_current_process->pid) {
		g_current_process->priority = priority;
		return process_prempt_if_necessary();
	}

	// TODO: We need to use process_find() here, since processes
	// that are BLOCKED_ON_MESSAGE won't be in either of these
	// queues.
	PCB** queue = g_ready_process_priority_queue;
	PCB* pcb = priority_queue_find(queue, id);
	if (pcb == NULL) {
		queue = g_blocked_process_priority_queue;
		pcb = priority_queue_find(queue, id);
		if (pcb == NULL) {
			LOG("Attempted to set process priority of nonexistent process!");
			return RTX_ERR;
		}
	}

	PriorityStatus status = priority_queue_reprioritize(queue, pcb, (ProcessPriority)priority);
	int preempt_status = process_prempt_if_necessary();
	if (status == PRIORITY_STATUS_OK && preempt_status == RTX_OK) {
		return RTX_OK;
	}
	return RTX_ERR;
}

int k_get_process_priority(int pid)
{
	PCB* proc = process_find(pid);
	if (proc != NULL) {
		return proc->priority;
	}
	return RTX_ERR;
}

int k_send_message(int dest_pid, void* msg)
{
	PCB* dest = process_find(dest_pid);
	if (!dest) {
		LOG("Destination process %d not found!", dest_pid);
		return RTX_ERR;
	}

	HeapBlock* block = heap_block_from_user_block(msg);
	// TODO: This is wrong when called via delayed_send.
	block->header.source_pid = g_current_process->pid;
	heap_queue_push(&dest->message_queue, block);
	if (dest->state != PROCESS_STATE_BLOCKED_ON_MESSAGE) {
		return RTX_OK;
	}

	dest->state = PROCESS_STATE_READY;
	priority_queue_insert(g_ready_process_priority_queue, dest);

	return process_prempt_if_necessary();
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

int k_delayed_send(int dest_pid, void *message_envelope, int delay)
{
	HeapBlock* full_env = heap_block_from_user_block(message_envelope);
	full_env->header.send_time = g_timer_count + delay;
	full_env->header.dest_pid = dest_pid;
	full_env->header.source_pid = g_current_process->pid;
  HeapQueueStatus status = sorted_heap_queue_push(&g_delayed_msg_list, full_env);

  if (status == QUEUE_STATUS_OK) {
  	return RTX_OK;
  } else {
  	return RTX_ERR;
  }
}
