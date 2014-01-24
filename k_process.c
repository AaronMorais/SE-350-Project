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
 *POST: if gp_current_process was NULL, then it gets set to pcbs[0].
 *      No other effect on other global variables.
 */
PCB* scheduler(void)
{
	PCB* next_process = priority_queue_pop();

#ifdef DEBUG_0
	printf("next_process id is: %d\n", next_process->m_pid);
#endif
	gp_current_process = next_process;

	if (gp_current_process == NULL) {
		printf("Warning: we shouldn't have a null current process at this point.\n");
		gp_current_process = gp_pcbs[0]; 
		return gp_pcbs[0];
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
int process_switch(PCB* p_pcb_old)
{
	printf("About to proccess_switch");
	ProcState state = gp_current_process->m_state;
	
	if (p_pcb_old != NULL) {
		priority_queue_insert(p_pcb_old);
	}

	printf("Did Queue insert");
	if (state == PROC_STATE_NEW) {
		if (gp_current_process != p_pcb_old && p_pcb_old && p_pcb_old->m_state != PROC_STATE_NEW) {
			p_pcb_old->m_state = PROC_STATE_READY;
			p_pcb_old->mp_sp = (U32*) __get_MSP();
		}
		gp_current_process->m_state = PROC_STATE_RUN;
		__set_MSP((U32) gp_current_process->mp_sp);
		// pop exception stack frame from the stack for a new processes
		__rte();
	} 
	
	/* The following will only execute if the if block above is FALSE */

	if (gp_current_process != p_pcb_old) {
		if (state == PROC_STATE_READY) {
			p_pcb_old->m_state = PROC_STATE_READY;

			// save the old process's sp
			p_pcb_old->mp_sp = (U32*) __get_MSP();
			gp_current_process->m_state = PROC_STATE_RUN;

			//switch to the new proc's stack
			__set_MSP((U32) gp_current_process->mp_sp);
		} else {
			// revert back to the old proc on error
			gp_current_process = p_pcb_old;
			return RTX_ERR;
		} 
	}
	return RTX_OK;
}

/**
 * @brief release_processor(). 
 * @return RTX_ERR on error and zero on success
 * POST: gp_current_process gets updated to next to run process
 */
int k_release_processor(void)
{
	PCB *p_pcb_old = NULL;

	p_pcb_old = gp_current_process;
	gp_current_process = scheduler();
	
	if (gp_current_process == NULL) {
		printf("WARNING: NO PROCESS TO SWITCH TO!");
		gp_current_process = p_pcb_old; // revert back to the old process
		return RTX_ERR;
	}
	
	process_switch(p_pcb_old);
	return RTX_OK;
}
