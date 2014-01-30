// This file contains userland processes to test the
// various functionalities of the kernel.

#include "usr_proc.h"
#include "syscall.h"
#include "uart_polling.h"
#include "k_process.h"

// Global variables for processes to change and check
#define TOTAL_NUMBER_OF_TESTS         	 7
#define GET_PRIORITY_TEST_0              0
#define SET_PRIORITY_TEST_1              1
#define RELEASE_PROCESSOR_TEST_2         2
#define RELEASE_MEMORY_TEST_3            3
#define REQUEST_MEMORY_TEST_4            4
#define	RELEASE_MEMORY_UNBLOCKED_TEST_5  5
#define	REQUEST_MEMORY_BLOCKED_TEST_6    6

static int s_passing_tests = 0;
static int s_failing_tests = 0;
static int test_results[TOTAL_NUMBER_OF_TESTS] = {0};

static int times_proc1_ran = 0;
static int times_proc2_ran = 0;
static int times_proc3_ran = 0;

/* 
 * ##### Proc1 and proc2 test the get and set process priority functionality. #####
 * Proc1 starts at MEDIUM priority and proc2 starts at LOWEST priority while everything else is LOW
 * Proc1 asserts that its priority is MEDIUM (testing get process priority)
 * Proc1 then changes priority of proc2 to be HIGH triggering preemption on proc1 and running proc2
 * Proc2 asserts that proc1 ran only 1 time before proc2 ran implying that the set_process_priority worked (second test)
 */

// Starts as priority Medium
// Sets process 2 to be higher priority so it should preempt 
// and process 2 should start running
static void proc1(void)
{
	while (1) {
		times_proc1_ran++;
		if (times_proc1_ran == 1 && get_process_priority(1) == PROCESS_PRIORITY_MEDIUM) {			
			test_results[GET_PRIORITY_TEST_0] = 1;
		}
		printf("proc1\r\n");
		set_process_priority(2, PROCESS_PRIORITY_HIGH);
	}
}

static int after_set_priority_before_release = 0;
// Starts as priority LOWEST
static void proc2(void)
{
	while (1) {
		printf("proc2\r\n");
		times_proc2_ran++;
		//it should have it's priority set by proc1
		if (times_proc1_ran == 1 && get_process_priority(2) == PROCESS_PRIORITY_HIGH) {
			test_results[SET_PRIORITY_TEST_1] = 1;
		}
		set_process_priority(1, PROCESS_PRIORITY_LOWEST);
		
		
//#### Testing release processor
//#### waits till proc2 runs twice, sets its priority to be the same level as the other processes
//#### proc3 should run after releasing processor		
		if (times_proc2_ran == 3) {
			set_process_priority(2, PROCESS_PRIORITY_MEDIUM);
			after_set_priority_before_release = 1;
			release_processor();
		}
	}
}

// Starts as priority LOW
static void proc3(void)
{
	while (1) {
		printf("proc3\r\n");
		times_proc3_ran++;
//#### asserts that proc2 ran the 3 times and that this is running because of the release processor call
		if (times_proc2_ran == 3 && times_proc3_ran == 1 && after_set_priority_before_release) {
			test_results[RELEASE_PROCESSOR_TEST_2] = 1;
		}
		set_process_priority(1, PROCESS_PRIORITY_LOWEST);
		set_process_priority(2, PROCESS_PRIORITY_LOWEST);
		set_process_priority(3, PROCESS_PRIORITY_LOWEST);
//		release_processor();
	}
}

static int s_requested_all_blocks = 0;

static void proc4(void)
{
	void* mem_block_2 = NULL;
	void* mem_block = request_memory_block();
	mem_block_2 = request_memory_block();
	
	if (mem_block_2 != NULL) {
		test_results[REQUEST_MEMORY_TEST_4] = 1;
	}
	
	// Setting proc5 to have higher priority so it should 
	// preempt and start running
	set_process_priority(5, PROCESS_PRIORITY_HIGH); 
	
	// Coming back from proc5 after proc5 blocked
	// Proc4 should preempty and go to proc5 since 
	// since it is no longer blocked and has a memory block
	s_requested_all_blocks = 1;
	test_results[REQUEST_MEMORY_BLOCKED_TEST_6] = 1;
	release_memory_block(mem_block);
	
	// Proc 5 receives the memory block and uses it
	// Then it sets its priority to be lowest, in which
	// the function comes here, and we set ourself to be
	// lowest so proc 6 can take over.
	set_process_priority(4, PROCESS_PRIORITY_LOWEST);
	release_processor();
}

static void proc5(void)
{
	void* want_all_blocks;
	
	while( !s_requested_all_blocks ) {
		want_all_blocks = request_memory_block();
	}
	
	// want_all_blocks unblocks after proc4 
	// releases a block of memory and sets the 
	// s_request_all_blocks to be true
	test_results[RELEASE_MEMORY_UNBLOCKED_TEST_5] = 1;
	int* num_ptr = (int*)want_all_blocks;
	*num_ptr = 5;
	
	int status = release_memory_block(num_ptr);
	if (status == RTX_OK) {
		test_results[RELEASE_MEMORY_TEST_3] = 1;
	}
	
	set_process_priority(5, PROCESS_PRIORITY_LOWEST);
}

static void proc6(void)
{
	run_all_tests();
}

void create_test_procs()
{
	ProcessInitialState test_proc = {0};
	for (int i = 0; i < NUM_TEST_PROCS; i++) {
		test_proc.pid = (U32)(i+1);
		test_proc.priority = PROCESS_PRIORITY_MEDIUM;
		test_proc.stack_size = 0x200;
		switch (i) {
		case 0: 
			test_proc.entry_point = &proc1; 
			test_proc.priority = PROCESS_PRIORITY_MEDIUM;
			break;
		case 1: 
			test_proc.entry_point = &proc2; 
			test_proc.priority = PROCESS_PRIORITY_LOW; 
			break;
		case 2: test_proc.entry_point = &proc3; break;
		case 3: test_proc.entry_point = &proc4; break;
		case 4: test_proc.entry_point = &proc5; break;
		case 5: 
			test_proc.entry_point = &proc6; 
			test_proc.priority = PROCESS_PRIORITY_LOWEST;
			break;
		}
		process_create(&test_proc);
	}
}

void run_all_tests()
{
	printf("G002_test: START\r\n");
	printf("G002_test: total %d tests\r\n", TOTAL_NUMBER_OF_TESTS);
	for (int i = 0; i < TOTAL_NUMBER_OF_TESTS; i++) {
		if (test_results[i]){
			printf("G002_test: test %d OK\r\n", i+1);
			s_passing_tests++;
		} else {
			printf("G002_test: test %d FAIL\r\n", i+1);
			s_failing_tests++;
		}
	}
	printf("G002_test: %d/%d tests OK\r\n", s_passing_tests, TOTAL_NUMBER_OF_TESTS);
	printf("G002_test: %d/%d tests FAIL\r\n", s_failing_tests, TOTAL_NUMBER_OF_TESTS);
	printf("G002_test: END\n");
}
