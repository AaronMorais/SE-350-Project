// This file contains userland processes to test the
// various functionalities of the kernel.

#include "../syscall.h"
#include "../uart_polling.h"
#include <LPC17xx.h>

typedef enum {
	REQUEST_MEMORY_TEST   = 0,
	RELEASE_MEMORY_TEST   = 1,
	SET_PRIORITY_TEST     = 2,
	SEND_MESSAGE_TEST     = 3,
	RECEIVE_MESSAGE_TEST  = 4,
	DELAYED_SEND_TEST     = 5,
	TEST_NUM              = 6
} Test;

static int test_results[TEST_NUM] = {0};

void print_test_results(void);

static const char* test_phrase = "The quick brown fox jumped over the lazy dog\n\r";
static int s_iteration_count = 0;

static LPC_TIM_TypeDef *s_test_timer;

static void timer_test_init() {
	s_test_timer = (LPC_TIM_TypeDef *) LPC_TIM1;
	s_test_timer->TCR = 1;
}

static void print_current_time() {
	int current_time = s_test_timer->TC; 
	LOG("The current time is %d", current_time);
}

static void proc1(void)
{
	timer_test_init();
	static const int memory_block_count = 50;
	static void* memory_blocks[memory_block_count];

	int start_time = 0;
	int end_time = 0;
	LOG("Started single request single release");
	for (int i = 0; i < 1000; i++) {
		start_time = s_test_timer->TC;
		memory_blocks[0] = request_memory_block();
		end_time = s_test_timer->TC;
		LOG("Request: %d", end_time - start_time);
		start_time = s_test_timer->TC;
		release_memory_block(memory_blocks[0]);
		end_time = s_test_timer->TC;
		LOG("Release: %d", end_time - start_time);
	}
	LOG("Finished single request single release");
	LOG("Started requesting");
	for (int i = 0; i < memory_block_count; i++) {
		start_time = s_test_timer->TC;
		memory_blocks[i] = request_memory_block();
		end_time = s_test_timer->TC;
		LOG("%d", end_time - start_time);
	}
	LOG("Finished requesting");
	test_results[REQUEST_MEMORY_TEST] = 1;
	LOG("Started releasing");
	for (int i = 0; i < memory_block_count; i++) {
		start_time = s_test_timer->TC;
		release_memory_block(memory_blocks[i]);
		end_time = s_test_timer->TC;
		LOG("%d", end_time - start_time);
	}
	LOG("Finished releasing");

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
	set_process_priority(4, USER_PROCESS_PRIORITY_HIGH);
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
	LOG("Started receiving messages");
	for (int i = 0; i < required_messages; i++) {
		int start_time = 0;
		int end_time = 0;
		start_time = s_test_timer->TC;
		void* message = receive_message(NULL);
		end_time = s_test_timer->TC;
		LOG("Receive: %d", end_time - start_time);
		release_memory_block(message);
	}
	LOG("Finished receiving messages");
	set_process_priority(5, USER_PROCESS_PRIORITY_HIGH);
	set_process_priority(4, USER_PROCESS_PRIORITY_LOWEST);
	set_process_priority(3, USER_PROCESS_PRIORITY_LOWEST);
	release_processor();
}

static void proc4(void)
{
	LOG("Started sending messages");
	for (int i = 0; i < required_messages; i++) {
		struct msgbuf* message_envelope = (struct msgbuf*)request_memory_block();
		message_envelope->mtype = 10;
		strcpy(message_envelope->mtext, test_phrase);
		int start_time = 0;
		int end_time = 0;
		start_time = s_test_timer->TC;
		send_message(3, message_envelope);
		end_time = s_test_timer->TC;
		LOG("Send: %d", end_time - start_time);
	}
	LOG("Finished sending messages");
	set_process_priority(3, USER_PROCESS_PRIORITY_HIGH);
	set_process_priority(4, USER_PROCESS_PRIORITY_LOWEST);
	release_processor();
}

int sample_size = 500;
static void proc5(void)
{
	set_process_priority(6, USER_PROCESS_PRIORITY_HIGH);
	void* message = receive_message(NULL);
	release_memory_block(message);
	
	while (true) {
		int start_time = 0;
		int end_time = 0;
		start_time = s_test_timer->TC;
		void* message = receive_message(NULL);
		end_time = s_test_timer->TC;
		LOG("Receive: %d", end_time - start_time);
		release_memory_block(message);
		release_processor();
	}
}

static void proc6(void)
{
	LOG("Started send-receive together");
	for (int i = 0; i < sample_size; i++) {
		struct msgbuf* message_envelope = (struct msgbuf*)request_memory_block();
		message_envelope->mtype = 10;
		strcpy(message_envelope->mtext, test_phrase);
		int start_time = 0;
		int end_time = 0;
		start_time = s_test_timer->TC;
		send_message(5, message_envelope);
		end_time = s_test_timer->TC;
		LOG("Send: %d", end_time - start_time);
		release_processor();
	}
	LOG("Finished send-receive together");
	
	LOG("Started variable message length together");
	for (int i = 0; i < sample_size; i++) {
		struct msgbuf* message_envelope = (struct msgbuf*)request_memory_block();
		message_envelope->mtype = 10;
		for (int j = 0; j < i; j++) {
			message_envelope->mtext[j] = *"a";
		}
		int start_time = 0;
		int end_time = 0;
		start_time = s_test_timer->TC;
		send_message(5, message_envelope);
		end_time = s_test_timer->TC;
		LOG("Send: %d", end_time - start_time);
		release_processor();
	}
	LOG("Finished variable message length together");
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
	uart0_put_string("G002_test: END\r\n");
}
