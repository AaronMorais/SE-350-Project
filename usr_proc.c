// This file contains userland processes to test the
// various functionalities of the kernel.

#include "usr_proc.h"
#include "syscall.h"
#include "uart_polling.h"

typedef enum {
	REQUEST_MEMORY_TEST   = 0,
	RELEASE_MEMORY_TEST  	= 1,
	SET_PRIORITY_TEST     = 2,
	SEND_MESSAGE_TEST     = 3,
	RECEIVE_MESSAGE_TEST  = 4,
	DELAYED_SEND_TEST     = 5,
	TEST_NUM              = 6
} Test;

static int test_results[TEST_NUM] = {0};

static void strcpy(char* dst, const char* src);
void print_test_results(void);

static const char* test_phrase = "The quick brown fox jumped over the lazy dog\n\r";
static int s_iteration_count = 0;

static void proc1(void)
{
	static const int memory_block_count = 30;
	void** memory_blocks[memory_block_count] = request_memory_block();
	for (int i = 0; i < memory_block_count; i++) {
		memory_blocks[i] = request_memory_block();
	}
	test_results[REQUEST_MEMORY_TEST] = 1;
	for (int i = 0; i < memory_block_count; i++) {
		release_memory_block(memory_blocks[i]);
	}
	test_results[RELEASE_MEMORY_TEST] = 1;
	set_process_priority(1, USER_PROCESS_PRIORITY_LOWEST);
	set_process_priority(2, USER_PROCESS_PRIORITY_HIGH);
	while (1) {
		s_iteration_count++;
		release_processor();
	}
}

static void proc2(void)
{
	test_results[SET_PRIORITY_TEST] = 1;
	set_process_priority(3, USER_PROCESS_PRIORITY_HIGH);
	set_process_priority(4, USER_PROCESS_PRIORITY_MEDIUM);
	set_process_priority(2, USER_PROCESS_PRIORITY_LOWEST);
	// proc 2 should be pre-empted and this should never be set to 0
	test_results[SET_PRIORITY_TEST] = 0;
	while (1) {
		release_processor();
	}
}

static int const required_messages = 50;

static void proc3(void)
{
	int received_messages = 0;
	while (1) {
		test_results[RECEIVE_MESSAGE_TEST] = 0;
		void* message = receive_message(NULL);
		received_messages++;
		release_memory_block(message);
		if (received_messages == required_messages) {
			test_results[RECEIVE_MESSAGE_TEST] = 1;
			set_process_priority(5, USER_PROCESS_PRIORITY_HIGH);
			set_process_priority(4, USER_PROCESS_PRIORITY_LOWEST);
			set_process_priority(3, USER_PROCESS_PRIORITY_LOWEST);
			release_processor();
		}
	}
}

static void proc4(void)
{
	int sent_messages = 0;
	while (1) {
		struct msgbuf* message_envelope = (struct msgbuf*)request_memory_block();
		message_envelope->mtype = 10;
		strcpy(message_envelope->mtext, test_phrase);
		sent_messages++;
		if (sent_messages == required_messages) {
			test_results[SEND_MESSAGE_TEST] = 1;
			release_processor();
		} else {
			send_message(3, message_envelope);
		}
	}
}

static void proc5(void)
{
	set_process_priority(1, USER_PROCESS_PRIORITY_MEDIUM);
	LOG("Current Iteration Count: %d", s_iteration_count);
	
	struct msgbuf* message_envelope = (struct msgbuf*)request_memory_block();
	delayed_send(5, message_envelope, 1000);
	void* message = receive_message(NULL);
	
	if (s_iteration_count > 2) {
			test_results[DELAYED_SEND_TEST] = 1;
			LOG("Final Iteration Count: %d", s_iteration_count);
	}
	release_memory_block(message);
	
	set_process_priority(1, USER_PROCESS_PRIORITY_LOWEST);
	set_process_priority(5, USER_PROCESS_PRIORITY_LOWEST);
	set_process_priority(6, USER_PROCESS_PRIORITY_HIGH);
	while (1) {
		release_processor();
	}
}

static void proc6(void)
{
	print_test_results();
	while (1) {
		release_processor();
	}
}

static void strcpy(char* dst, const char* src)
{
	while (*src) {
		*dst++ = *src++;
	}
}

PROC_INIT g_test_procs[NUM_TEST_PROCS];
void set_test_procs()
{
	PROC_INIT test_proc = {0};
	for (int i = 0; i < NUM_TEST_PROCS; i++) {
		test_proc.pid = (U32)(i+1);
		test_proc.priority = USER_PROCESS_PRIORITY_LOWEST;
		test_proc.stack_size = 0x200;
		switch (i) {
		case 0: 
			test_proc.entry_point = &proc1;	
			test_proc.priority = USER_PROCESS_PRIORITY_HIGH;
			break;
		case 1: test_proc.entry_point = &proc2; break;
		case 2: test_proc.entry_point = &proc3; break;
		case 3: test_proc.entry_point = &proc4; break;
		case 4: test_proc.entry_point = &proc5; break;
		case 5: test_proc.entry_point = &proc6; break;
		default: continue;
		}
		g_test_procs[i] = test_proc;
	}
}

void print_test_results()
{
	uart0_put_string("G002_test: START\r\n");
	uart0_put_string("G002_test: total ");
	uart0_put_char(TEST_NUM + '0');
	uart0_put_string(" tests\r\n");
	int s_passing_tests = 0;
	int s_failing_tests = 0;
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
