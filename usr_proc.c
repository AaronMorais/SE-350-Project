// This file contains userland processes to test the
// various functionalities of the kernel.

#include "usr_proc.h"
#include "syscall.h"
#include "uart_polling.h"
#include "k_process.h"

typedef enum {
	SET_PRIORITY_TEST              = 0,
	RELEASE_PROCESSOR_TEST         = 1,
	RELEASE_MEMORY_TEST            = 2,
	REQUEST_MEMORY_TEST            = 3,
	RELEASE_MEMORY_UNBLOCKED_TEST  = 4,
	REQUEST_MEMORY_BLOCKED_TEST    = 5,
	TEST_NUM											 = 6
} Test;

static int s_passing_tests = 0;
static int s_failing_tests = 0;
static int test_results[TEST_NUM] = {0};

static int times_proc1_ran = 0;
static int times_proc2_ran = 0;
static int times_proc3_ran = 0;

/* 
 * ##### Proc1 and proc2 test the set process priority functionality. #####
 * Proc1 starts at MEDIUM priority and proc2 starts at LOW priority while everything else is LOW
 * Proc1 then changes priority of proc2 to be HIGH triggering preemption on proc1 and running proc2
 * Proc2 asserts that proc1 ran only 1 time before proc2 ran implying that the set_process_priority worked (first test)
 */

// Starts as priority Medium
// Sets process 2 to be higher priority so it should preempt 
// and process 2 should start running
static void proc1(void)
{
	while (1) {
		times_proc1_ran++;
		set_process_priority(2, PROCESS_PRIORITY_HIGH);
	}
}

static int after_set_priority_before_release = 0;
// Starts as priority LOW
// Will be set to HIGH and preempt proc1
static void proc2(void)
{
	while (1) {
		times_proc2_ran++;
		//it should have it's priority set by proc1
		if (times_proc1_ran == 1 && get_process_priority(2) == PROCESS_PRIORITY_HIGH) {
			test_results[SET_PRIORITY_TEST] = 1;
		}
		set_process_priority(1, PROCESS_PRIORITY_LOWEST);
		
		
//#### Testing release processor
//#### waits until proc2 runs three times, sets its priority to be the same level as the other processes
//#### proc3 should run after releasing processor		
		if (times_proc2_ran == 3) {
			set_process_priority(2, PROCESS_PRIORITY_MEDIUM);
			after_set_priority_before_release = 1;
		}
		if (times_proc2_ran >= 3) {
			release_processor();
		}
	}
}

// Starts as priority MEDIUM
static void proc3(void)
{
	while (1) {
		times_proc3_ran++;
//#### asserts that proc2 ran the 3 times and that this is running because of the release processor call
		if (times_proc2_ran == 3 && times_proc3_ran == 1 && after_set_priority_before_release) {
			test_results[RELEASE_PROCESSOR_TEST] = 1;
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
		test_results[REQUEST_MEMORY_TEST] = 1;
	}
	
	// Setting proc5 to have higher priority so it should 
	// preempt and start running
	set_process_priority(5, PROCESS_PRIORITY_HIGH); 
	
	// Coming back from proc5 after proc5 blocked
	// Proc4 should preempty and go to proc5 since 
	// since it is no longer blocked and has a memory block
	s_requested_all_blocks = 1;
	test_results[REQUEST_MEMORY_BLOCKED_TEST] = 1;
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
	test_results[RELEASE_MEMORY_UNBLOCKED_TEST] = 1;
	int* num_ptr = (int*)want_all_blocks;
	*num_ptr = 5;
	
	int status = release_memory_block(num_ptr);
	if (status == RTX_OK) {
		test_results[RELEASE_MEMORY_TEST] = 1;
	}
	
	set_process_priority(5, PROCESS_PRIORITY_LOWEST);
}

static void proc6(void)
{
	run_all_tests();
	set_process_priority(7, PROCESS_PRIORITY_HIGH);
}

static void strcpy(char* dst, const char* src)
{
	while (*src) {
		*dst++ = *src++;
	}
}

static const char* test_phrase_one = "The quick brown fox jumped over the lazy dog";
static const char* test_phrase_two = "The quick brown fox jumped over the lazy dog";
static void proc7(void)
{
	set_process_priority(8, PROCESS_PRIORITY_HIGH);
	struct msgbuf* message_envelope = (struct msgbuf*)request_memory_block();
	message_envelope->mtype = 0xAAAAAAAA;
	strcpy(message_envelope->mtext, test_phrase_one);
	send_message(8, (void*)message_envelope);

	while (1) {
		release_processor();
		printf("proc7: %x %s\n", message_envelope->mtype, message_envelope->mtext);
	}
}

static void proc8(void)
{
	struct msgbuf* message_envelope = (struct msgbuf*)receive_message(NULL);
	
	printf("proc8: %x %s\n", message_envelope->mtype, message_envelope->mtext);
	while (1) {
		release_processor();
		printf("proc8: %x %s\n", message_envelope->mtype, message_envelope->mtext);
		set_process_priority(7, PROCESS_PRIORITY_LOWEST);
		set_process_priority(9, PROCESS_PRIORITY_HIGH);
	}
}

static void proc9(void)
{
	set_process_priority(8, PROCESS_PRIORITY_LOWEST);
	set_process_priority(10, PROCESS_PRIORITY_HIGH);
	struct msgbuf* message_envelope = NULL;
	while (!message_envelope) {
		message_envelope = (struct msgbuf*)receive_message(NULL);
	}
	printf("proc9: %x %s\n", message_envelope->mtype, message_envelope->mtext);
	while (1) {
		release_processor();
	}
}
static void proc10(void)
{
	struct msgbuf* message_envelope = (struct msgbuf*)request_memory_block();
	message_envelope->mtype = 0x22222;
	strcpy(message_envelope->mtext, test_phrase_two);
	delayed_send(9, (void*)message_envelope, 7);
	printf("proc10: %x %s\n", message_envelope->mtype, message_envelope->mtext);
	while (1) {
		release_processor();
	}
}

PROC_INIT g_test_procs[NUM_TEST_PROCS];
void set_test_procs()
{
	int ignore_num = 8;
	PROC_INIT test_proc = {0};
	for (int i = 0; i < NUM_TEST_PROCS; i++) {
		test_proc.pid = (U32)(i+1+ignore_num);
		test_proc.priority = PROCESS_PRIORITY_LOWEST;
		test_proc.stack_size = 0x200;
		switch (i+ignore_num) {
// 		case 0:
// 			test_proc.entry_point = &proc1;
// 			test_proc.priority = PROCESS_PRIORITY_MEDIUM;
// 			break;
// 		case 1:
// 			test_proc.entry_point = &proc2;
// 			test_proc.priority = PROCESS_PRIORITY_LOW;
// 			break;
// 		case 2: test_proc.entry_point = &proc3; break;
// 		case 3: test_proc.entry_point = &proc4; break;
// 		case 4: test_proc.entry_point = &proc5; break;
// 		case 5:
// 			test_proc.entry_point = &proc6;
// 			test_proc.priority = PROCESS_PRIORITY_LOWEST;
// 			break;
// 		case 6:
// 			test_proc.entry_point = &proc7;
// 			test_proc.priority = PROCESS_PRIORITY_LOWEST;
// 			break;
// 		case 7:
// 			test_proc.entry_point = &proc8;
// 			test_proc.priority = PROCESS_PRIORITY_LOWEST;
// 			break;
		case 8:
			test_proc.entry_point = &proc9;
			test_proc.priority = PROCESS_PRIORITY_HIGH;
			break;
		case 9:
			test_proc.entry_point = &proc10;
			test_proc.priority = PROCESS_PRIORITY_HIGH;
			break;
		default:
			continue;
		}
		g_test_procs[i] = test_proc;
	}
}

void run_all_tests()
{
	uart0_put_string("G002_test: START\r\n");
	uart0_put_string("G002_test: total ");
	uart0_put_char(TEST_NUM + '0');
	uart0_put_string(" tests\r\n");
	for (int i = 0; i < TEST_NUM; i++) {
		if (test_results[i]){
			uart0_put_string("G002_test: test ");
			uart0_put_char((i + 1) + '0');
			uart0_put_string(" OK\r\n");
			s_passing_tests++;
		} else {
			uart0_put_string("G002_test: test ");
			uart0_put_char((i + 1) + '0');
			uart0_put_string(" FAIL\r\n");
			s_failing_tests++;
		}
	}
	uart0_put_string("G002_test: ");
	uart0_put_char(s_passing_tests + '0');
	uart0_put_string("/");
	uart0_put_char(TEST_NUM + '0');
	uart0_put_string(" tests OK\r\n");
	uart0_put_string("G002_test: ");
	uart0_put_char(s_failing_tests + '0');
	uart0_put_string("/");
	uart0_put_char(TEST_NUM + '0');
	uart0_put_string(" tests FAIL\r\n");
	uart0_put_string("G002_test: END\n");
}
