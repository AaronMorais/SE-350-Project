// This file should contain userland processes to test the
// various functionalities of the kernel.
#include "usr_proc.h"
#include "syscall.h"
#include "uart_polling.h"
#include "k_process.h"

#ifdef DEBUG_0
#include "printf.h"
#endif

static void proc1(void)
{
	while (1) {
		printf("proc1\r\n");
		void* mem = request_memory_block();
		release_memory_block(mem);
		set_process_priority(2, PROCESS_PRIORITY_HIGH);
		release_processor();
		set_process_priority(2, PROCESS_PRIORITY_LOW);
		release_processor();
	}
}

static void proc2(void)
{
	while (1) {
		printf("proc2\r\n");
		set_process_priority(1, PROCESS_PRIORITY_HIGH);
		release_processor();
		set_process_priority(1, PROCESS_PRIORITY_LOWEST);
		release_processor();
		request_memory_block();
	}
}

static void proc3(void)
{
	while (1) {
		printf("proc3\r\n");
		set_process_priority(3, PROCESS_PRIORITY_LOWEST);
		release_processor();
	}
}

void create_test_procs()
{
	ProcessInitialState test_proc = {0};
	for (int i = 0; i < NUM_TEST_PROCS; i++) {
		test_proc.pid = (U32)(i+1);
		test_proc.priority = PROCESS_PRIORITY_LOWEST;
		test_proc.stack_size = 0x200;
		switch (i) {
		case 0: test_proc.entry_point = &proc1; break;
		case 1: test_proc.entry_point = &proc2; break;
		case 2: test_proc.entry_point = &proc3; break;
		}
		process_create(&test_proc);
	}
}
