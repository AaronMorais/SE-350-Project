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

#include "priority_queue.h"

#ifdef DEBUG_0
#include "printf.h"
#endif

#define LOG(str) printf(str "\r\n")

// TODO: We should actually include the header files for these...
// Allocate stack for a process
extern U32 *alloc_stack(U32 size_b);
// Pop exception stack frame
extern void __rte(void);
// Test process initial set up
extern void set_test_procs(void);

// Array of PCBs
PCB **gp_pcbs;
// Running process
PCB *gp_current_process = NULL;

// Process initialization table
PROC_INIT g_proc_table[NUM_TEST_PROCS];
extern PROC_INIT g_test_procs[NUM_TEST_PROCS];

/**
 * @biref: initialize all processes in the system
 * NOTE: We assume there are only two user processes in the system in this example.
 */
void process_init() 
{
	// Fill out the initialization table
	set_test_procs();
	for (int i = 0; i < NUM_TEST_PROCS; i++) {
		g_proc_table[i].m_pid = g_test_procs[i].m_pid;
		g_proc_table[i].m_stack_size = g_test_procs[i].m_stack_size;
		g_proc_table[i].mpf_start_pc = g_test_procs[i].mpf_start_pc;
		g_proc_table[i].m_priority = g_test_procs[i].m_priority;
	}
  
	// initilize exception stack frame (i.e. initial context) for each process
	for (int i = 0; i < NUM_TEST_PROCS; i++) {
		(gp_pcbs[i])->m_pid = (g_proc_table[i]).m_pid;
		(gp_pcbs[i])->m_state = PROC_STATE_NEW;
		
		U32* sp = alloc_stack((g_proc_table[i]).m_stack_size);
		*(--sp) = INITIAL_xPSR;      // user process initial xPSR
		*(--sp) = (U32)((g_proc_table[i]).mpf_start_pc); // PC contains the entry point of the process
		for (int j = 0; j < 6; j++) { // R0-R3, R12 are cleared with 0
			*(--sp) = 0x0;
		}
		(gp_pcbs[i])->mp_sp = sp;
		(gp_pcbs[i])->m_priority = (g_proc_table[i]).m_priority;
	}
	for (int i = 0; i < NUM_TEST_PROCS; i++) {
		priority_queue_insert(gp_pcbs[i]);
	}
}

/*@brief: scheduler, pick the pid of the next to run process
 *@return: PCB pointer of the next to run process
 *         NULL if error happens
 */
PCB* scheduler(void)
{
	PCB* next_process = priority_queue_pop();
	
	if (next_process == NULL) {
		printf("Warning: No processes on ready queue.\n");
	} else {
	#ifdef DEBUG_0
		printf("next_process id is: %d\r\n", next_process->m_pid);
	#endif
	}
	
	return next_process;
}

/*@brief: switch out old pcb (p_pcb_old), run the new pcb (gp_current_process)
 *@param: p_pcb_old, the old pcb that was in PROC_STATE_RUN
 *@return: RTX_OK upon success
 *         RTX_ERR upon failure
 *PRE:  p_pcb_old and gp_current_process are pointing to valid PCBs.
 *POST: if gp_current_process was NULL, then it gets set to pcbs[0].
 *      No other effect on other global variables.
 */
int process_switch(PCB* new_proc)
{
	LOG("About to proccess_switch");
	if (new_proc == NULL) {
		LOG("NULL passed to process_switch!");
		return RTX_ERR;
	}
	
	ProcState state = new_proc->m_state;
	
	if (gp_current_process != NULL) {
		priority_queue_insert(gp_current_process);
	}
	
	if (state == PROC_STATE_READY || state == PROC_STATE_NEW) {
		if (gp_current_process && gp_current_process->m_state != PROC_STATE_NEW) {
			gp_current_process->m_state = PROC_STATE_READY;
			gp_current_process->mp_sp = (U32*) __get_MSP();
		}
		
		new_proc->m_state = PROC_STATE_RUN;

		// switch to the new proc's stack
		__set_MSP((U32) new_proc->mp_sp);
		
		gp_current_process = new_proc;
		
		if (state == PROC_STATE_NEW) {
			// pop exception stack frame from the stack for a new processes
			__rte();
		}
		return RTX_OK;
	}
	
	return RTX_ERR;
}

/**
 * @brief release_processor(). 
 * @return RTX_ERR on error and zero on success
 * POST: gp_current_process gets updated to next to run process
 */
int k_release_processor(void)
{
	// Slow things down to make them easier to debug
	for (volatile int i = 0; i < 10000000; i++) {}
		
	PCB *new_proc = scheduler();
	
	if (new_proc == NULL) {
		LOG("No process to switch to.");
		return RTX_ERR;
	}
	
	return process_switch(new_proc);
}
