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

#ifdef DEBUG_0
#include "printf.h"
#endif

// Currently running process
PCB* g_current_process = NULL;
PCB* g_ready_process_priority_queue[PROCESS_PRIORITY_NUM];

void null_process() {
	while (1) {
		k_release_processor();	
		LOG("Running null process");
	}
}

void process_init()
{
	// Test process initial set up
	extern void create_test_procs(void);
	create_test_procs();
	
	// Set up NULL process
	ProcessInitialState null_state;
	null_state.pid = (U32)(0);
	null_state.priority = PROCESS_PRIORITY_NULL_PROCESS;
	null_state.stack_size = 0x100;
	null_state.entry_point = &null_process;
	process_create(&null_state);
}

// Note: This must be called during system initialization, before
// heap_init() is called (we don't yet have dynamic processes :(.
int process_create(ProcessInitialState* initial_state)
{
	PCB* pcb = memory_alloc_pcb();
	if (!pcb) {
		return RTX_ERR;
	}

	pcb->pid = initial_state->pid;
	pcb->state = PROCESS_STATE_NEW;
	pcb->priority = initial_state->priority;

	// initilize exception stack frame (i.e. initial context)
	U32* sp = memory_alloc_stack(initial_state->stack_size);
	if (!sp) {
		return RTX_ERR;
	}

	// user process initial xPSR
	*(--sp) = INITIAL_xPSR;
	// PC contains the entry point of the process
	*(--sp) = (U32)initial_state->entry_point;
	// R0-R3, R12 are cleared with 0
	for (int j = 0; j < 6; j++) {
		*(--sp) = 0x0;
	}
	pcb->sp = sp;

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
		printf("Warning: No processes on ready queue.\n");
	} else {
		LOG("next_process id is: %d", next_process->pid);
	}

	return next_process;
}

// WARNING: This currently uses __get_MSP() and __set_MSP(), which
// means the user processes run in privileged mode (not really ideal...),
// and it will need to change to support interrupts.
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

	if (g_current_process && g_current_process->state != PROCESS_STATE_NEW) {
		g_current_process->state = PROCESS_STATE_READY;
		g_current_process->sp = (U32*) __get_MSP();
	}

	new_proc->state = PROCESS_STATE_RUN;
	__set_MSP((U32) new_proc->sp);

	g_current_process = new_proc;

	if (state == PROCESS_STATE_NEW) {
		// pop exception stack frame from the stack for a new processes
		extern void __rte(void);
		// Note: This actually causes us to start executing the procees
		// with crazy assembly magic (See HAL.c)!
		__rte();
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
	priority_queue_insert(g_ready_process_priority_queue, g_current_process);
	PCB *new_proc = scheduler();
	
	if (new_proc == NULL) {
		LOG("No process to switch to.");
		return RTX_ERR;
	}
	
	return switch_to_process(new_proc);
}

int k_set_process_priority(int id, int priority) {
	int valid = 0;
	int previous_priority = 0;
	if (priority < 0 || priority >= PROCESS_PRIORITY_NULL_PROCESS)
		return -1;

	for (unsigned int i = 0; i < g_pcb_counter; i++) {
		if (s_pcb_allocations_start[i].pid == id) {
			previous_priority = s_pcb_allocations_start[i].priority;
			s_pcb_allocations_start[i].priority = priority;
			valid = 1;
		}
	}

	if (valid == 0) {
		LOG("k_set_process_priority: Attempted to set a priority id that doesn't exist!");
		return -1;
	}

	// change the ready queue.... if we can find it
	int found = priority_change(g_ready_process_priority_queue, id, previous_priority);

	if (found == 0) // We try to find it in the other queue and change the priority there
		priority_change(g_blocked_process_priority_queue, id, previous_priority);

	PCB* top_priority_running = priority_queue_top(g_ready_process_priority_queue);

	// Preempting if the id isn't the same and the priority given is higher than the current running process
	// OR if the process is trying to change itself (which it wont find itself in the blocked or running queue)
	// and the top of the running queue has higher priority
	if(g_current_process->priority > top_priority_running->priority) {
		LOG("k_set_process_priority: setting a priority to be higher than itself. Preempting." );
		k_release_processor();
	}

	return 0;
}

int k_get_process_priority(int id) {
	for( unsigned int i = 0; i < g_pcb_counter; i++ ){
		if( s_pcb_allocations_start[i].pid == id ) {
			return s_pcb_allocations_start[i].priority;
		}
	}
	
	return -1;
}
