// This file should contain userland processes to test the
// various functionalities of the kernel.
#include "syscall.h"
#include "uart_polling.h"
#include "usr_proc.h"
#include "k_process.h"

#ifdef DEBUG_0
#include "printf.h"
#endif

void create_test_procs()
{
	PROC_INIT test_proc = {0};
	for (int i = 0; i < NUM_TEST_PROCS; i++) {
		test_proc.pid = (U32)(i+1);
		test_proc.priority = PROCESS_PRIORITY_LOWEST;
		test_proc.stack_size = 0x100;
		switch (i) {
		case 0: test_proc.entry_point = &proc1; break;
		case 1: test_proc.entry_point = &proc2; break;
		case 2: test_proc.entry_point = &proc3; break;
		}
		process_create(&test_proc);
	}
}

void proc1(void)
{
	int i = 0;
	while (1) {
		uart0_put_char('0' + i%26);
		i++;
		if (i%5 != 0) continue;

		uart0_put_string("\n\r");
		int ret_val = release_processor();
#ifdef DEBUG_0
		printf("proc1: ret_val=%d\r\n", ret_val);
#endif
	}
}

void proc2(void)
{
	int i = 0;
	while (1) {
		uart0_put_char('a' + i%26);
		i++;
		if (i%5 != 0) continue;

		uart0_put_string("\n\r");
		int ret_val = release_processor();
#ifdef DEBUG_0
		printf("proc1: ret_val=%d\r\n", ret_val);
#endif
	}
}

void proc3(void)
{
	int i = 0;
	while (1) {
		uart0_put_char('A' + i%26);
		i++;
		if (i%5 != 0) continue;

		uart0_put_string("\n\r");
		int ret_val = release_processor();
#ifdef DEBUG_0
		printf("proc1: ret_val=%d\r\n", ret_val);
#endif
	}
}
