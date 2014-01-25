/**
 * @file:   usr_proc.c
 * @brief:  Two user processes: proc1 and proc2
 * @author: Yiqing Huang
 * @date:   2014/01/17
 * NOTE: Each process is in an infinite loop. Processes never terminate.
 */

#include "rtx.h"
#include "uart_polling.h"
#include "usr_proc.h"
#include "k_process.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

/* initialization table item */
PROC_INIT g_test_procs[NUM_TEST_PROCS];

void create_test_procs() {
	PROC_INIT test_proc = {0};
	for (int i = 0; i < NUM_TEST_PROCS; i++) {
		test_proc.m_pid = (U32)(i+1);
		test_proc.m_priority = LOWEST;
		test_proc.m_stack_size = 0x100;
		switch (i) {
		case 0: test_proc.mpf_start_pc = &proc1; break;
		case 1: test_proc.mpf_start_pc = &proc2; break;
		case 2: test_proc.mpf_start_pc = &proc3; break;
		}
		process_create(&test_proc);
	}
}


/**
 * @brief: a process that prints five uppercase letters
 *         and then yields the cpu.
 */
void proc1(void)
{
	int i = 0;
	int ret_val = 10;
	while ( 1) {
		if ( i != 0 && i%5 == 0 ) {
			uart0_put_string("\n\r");
			ret_val = release_processor();
#ifdef DEBUG_0
			printf("proc1: ret_val=%d\r\n", ret_val);
#endif /* DEBUG_0 */
		}
		uart0_put_char('A' + i%26);
		i++;
	}
}

/**
 * @brief: a process that prints five numbers
 *         and then yields the cpu.
 */
void proc2(void)
{
	int i = 0;
	int ret_val = 20;
	while ( 1) {
		if ( i != 0 && i%5 == 0 ) {
			uart0_put_string("\n\r");
			ret_val = release_processor();
#ifdef DEBUG_0
			printf("proc2: ret_val=%d\r\n", ret_val);
#endif /* DEBUG_0 */
		}
		uart0_put_char('0' + i%10);
		i++;
	}
}

/**
 * @brief: a process that prints five numbers
 *         and then yields the cpu.
 */
void proc3(void)
{
	int i = 0;
	int ret_val = 20;
	while ( 1) {
		if ( i != 0 && i%5 == 0 ) {
			uart0_put_string("\n\r");
			ret_val = release_processor();
#ifdef DEBUG_0
			printf("proc3: ret_val=%d\r\n", ret_val);
#endif /* DEBUG_0 */
		}
		uart0_put_char('a' + i%10);
		i++;
	}
}
