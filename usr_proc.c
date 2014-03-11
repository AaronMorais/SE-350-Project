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

static int times_proc1_ran = 0;

static void strcpy(char* dst, const char* src);
void print_test_results(void);

static const char* test_phrase_one = "The quick brown fox jumped over the lazy dog\n\r";
static const char* test_phrase_two = "The quick brown fox jumped over the lazy dog\n\r";
		//struct msgbuf* message_envelope = (struct msgbuf*)request_memory_block();
		//message_envelope->mtype = 0x22222;
		//strcpy(message_envelope->mtext, test_phrase_two);

static void proc1(void)
{
	while (1) {
		release_processor();
	}
}

static void proc2(void)
{
	while (1) {
		release_processor();
	}
}

static void proc3(void)
{
	while (1) {
		release_processor();
	}
}

static void proc4(void)
{
	while (1) {
		release_processor();
	}
}

static void proc5(void)
{
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
