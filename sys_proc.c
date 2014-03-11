#include <LPC17xx.h>
#include "syscall.h"
#include "sys_proc.h"
#include "k_rtx.h"
#include "k_process.h"
#include "uart.h"
#include "heap.h"
#include "uart_polling.h"
#include "timer.h"

static void null_process(void);
static void crt_process(void);
static void kcd_process(void);
static void wall_clock_process(void);

struct RegisteredCommand g_registered_commands[NUM_COMMANDS];
char g_command_buffer[COMMAND_CHAR_NUM] = {COMMAND_NULL};
int g_cur_command_buffer_index = 0;

void sys_proc_init() {
	// Set up NULL process
	PROC_INIT null_state;
	null_state.pid = (U32)PROCESS_ID_NULL;
	null_state.priority = PROCESS_PRIORITY_NULL_PROCESS;
	null_state.stack_size = 0x200;
	null_state.entry_point = &null_process;
	process_create(&null_state);

	// Set up CRT process
	PROC_INIT crt_state;
	crt_state.pid = (U32)PROCESS_ID_CRT;
	crt_state.priority = PROCESS_PRIORITY_SYSTEM_PROCESS;
	crt_state.stack_size = 0x200;
	crt_state.entry_point = &crt_process;
	process_create(&crt_state);

	// Set up KCD process
	PROC_INIT kcd_state;
	kcd_state.pid = (U32)PROCESS_ID_KCD;
	kcd_state.priority = PROCESS_PRIORITY_SYSTEM_PROCESS;
	kcd_state.stack_size = 0x200;
	kcd_state.entry_point = &kcd_process;
	process_create(&kcd_state);
	
	// Set up UART-i process
	PROC_INIT uart_state;
	uart_state.pid = (U32)PROCESS_ID_UART;
	uart_state.priority = PROCESS_PRIORITY_UNSCHEDULABLE;
	uart_state.stack_size = 0x200;
	uart_state.entry_point = NULL;
	process_create(&uart_state);
	
	// Set up wall_clock process
	PROC_INIT wall_clock_state;
	wall_clock_state.pid = (U32)PROCESS_ID_WALL_CLOCK;
	wall_clock_state.priority = PROCESS_PRIORITY_SYSTEM_PROCESS;
	wall_clock_state.stack_size = 0x200;
	wall_clock_state.entry_point = &wall_clock_process;
	process_create(&wall_clock_state);
}

static void null_process(){
	while (1) {
		release_processor();
		LOG("Running null process");
	}
}

static void init_kcd_process() {
	for( int i = 0; i < NUM_COMMANDS; i++ ) {
		g_registered_commands[i].process_id = -1;
		// 65 is the ASCII character A
		g_registered_commands[i].command = (char)(i + ASCII_START);
	}
}

// TODO move this to a shared space one day.. but not today
static void strcpy(char* dst, const char* src)
{
	while (*src) {
		*dst++ = *src++;
	}
}

static char *mystrcpy(char *dst, const char *src)
{
	char *ptr;
	ptr = dst;
	while(*dst++=*src++);
	return(ptr);
}

static void kcd_process_clear_command_buffer() {
	for (int i = 0; i < NUM_TEST_PROCS; i++) {
		g_command_buffer[i] = COMMAND_NULL;
	}
	g_cur_command_buffer_index = 0;
}

static void kcd_process_send_message(int pid, struct msgbuf* message) {
	message->mtype = MESSAGE_TYPE_CRT_DISPLAY_REQUEST;
	send_message(pid, (void*)message);
}

static void push_to_command_buffer(struct msgbuf* message) {
	if (g_cur_command_buffer_index >= COMMAND_CHAR_NUM) {
		printf( "ERROR: push_to_command_buffer buffer is filled%c", message->mtext[0]);
		return;
	}
	g_command_buffer[g_cur_command_buffer_index] = message->mtext[0];
	g_cur_command_buffer_index++;
}

static void kcd_process_clear_message_block(struct msgbuf* message) {
	if (release_memory_block(message) != RTX_OK) {
		printf("ERROR: KCD_Proc tried to release the received message, but it's invalid");
	}
}

static void backspace() {
	g_cur_command_buffer_index--;
	g_command_buffer[g_cur_command_buffer_index] = COMMAND_NULL;
}

static void kcd_process_character_input(struct msgbuf* message) {

	if (g_cur_command_buffer_index == 0) {
		if(message->mtext[0] == '%') {
			// Start the buffer wait
			push_to_command_buffer(message);
		}
		// Pass the message to CRT tp print and clear block
		kcd_process_send_message(PROCESS_ID_CRT, message);
	} else if (g_cur_command_buffer_index == 1) {
		// If it second character isn't a command, we send a message to CRT to print shit
		if(message->mtext[0] == '\b') {
			backspace();
		} else if (g_registered_commands[message->mtext[0] - ASCII_START].process_id == -1) {
			// Modifies the message to be "%" + mtext[0] in the message passed along
			char second_char = message->mtext[0];
			message->mtext[0] = g_command_buffer[0];
			message->mtext[1] = second_char;
			kcd_process_clear_command_buffer(); // Clear the buffer for next command
		} else {
			// This is valid for the buffer and we add to the command buffe
			push_to_command_buffer(message);
		}
		kcd_process_send_message(PROCESS_ID_CRT, message);
	} else {
		// It is in the middle of a buffer wait on newline
		if (message->mtext[0] == '\b') {
			backspace();
		} else if (message->mtext[0] == '\r') {
			// Newline means command submit without the newline
			mystrcpy(message->mtext, g_command_buffer);
			int command_index = g_command_buffer[1] - ASCII_START;
			kcd_process_send_message(g_registered_commands[command_index].process_id, message);
			kcd_process_clear_command_buffer();
		} else {
			// No newline means we're still buffering incoming letters
			push_to_command_buffer(message);
			kcd_process_send_message(PROCESS_ID_CRT, message);
		}
	}
}

static void kcd_process_command_registration(struct msgbuf* message) {
	// Message data should be of the format 'C' where C is a capital character
		int index = message->mtext[0] - ASCII_START;
		if (index < 0 || index > 25) {
			LOG( "ERROR: KCD_Proc command is not in range%c", message->mtext[0]);
			return;
		}

		if (g_registered_commands[index].process_id != -1) {
			LOG( "ERROR: KCD_Proc received multiply registration for charater %c\n", message->mtext[0]);
			return;
		}

		HeapBlock* block = heap_block_from_user_block( (void*)message );
		g_registered_commands[index].process_id = block->header.source_pid;
}

static void kcd_process() {
	init_kcd_process();
	while (1) {
		struct msgbuf* message = NULL;
		message = receive_message(NULL);

		if (message == NULL) {
			LOG("ERROR: KCD_Proc received a null message\n");
			return;
		}

		if (message->mtype == MESSAGE_TYPE_KCD_KEYPRESS_EVENT) {
			kcd_process_character_input(message);
		} else if (message->mtype == MESSAGE_TYPE_KCD_COMMAND_REGISTRATION) {
			kcd_process_command_registration(message);
			kcd_process_clear_message_block(message);
		} else {
			LOG("ERROR: KCD_Proc received a message that was not of type MESSAGE_TYPE_KCD");
		}
	}
}

static void crt_process() {
	while (1) {
		struct msgbuf* message = receive_message(NULL);
		if (message == NULL || message->mtype != MESSAGE_TYPE_CRT_DISPLAY_REQUEST) {
			LOG("ERROR: CRT_Proc received a message that was not of type CRT_DISPLAY_REQUEST");
			continue;
		}
		if (message->mtext[0] == '\r') {
			message->mtext[1] = '\n';
		}
		LOG("=======Crt process running...");
		send_message(PROCESS_ID_UART, message);
		LPC_UART_TypeDef *pUart = (LPC_UART_TypeDef*) LPC_UART0;
		pUart->IER = IER_RBR | IER_THRE | IER_RLS;
		pUart->THR = '\0';
	}
}

static void wall_clock_print_time(char* buf, int seconds) {
	int s0 = seconds % 10;
	int s1 = seconds % 60 / 10;
	int m0 = seconds / 60 % 10;
	int m1 = seconds / 60 / 10;
	int h0 = seconds / 60 / 60 % 10;
	int h1 = seconds / 60 / 60 / 10;
	*buf++ = h1 + '0';
	*buf++ = h0 + '0';
	*buf++ = ':';
	*buf++ = m1 + '0';
	*buf++ = m0 + '0';
	*buf++ = ':';
	*buf++ = s1 + '0';
	*buf++ = s0 + '0';
	*buf++ = '\n';
	*buf++ = '\r';
	*buf++ = '\0';
}

static void wall_clock_process() {
	static const char CLOCK_RESET     = 'R';
	static const char CLOCK_SET       = 'S';
	static const char CLOCK_TERMINATE = 'T';

	static uint32_t wall_clock_time_offset = 0;
	static uint32_t wall_clock_time = 0;
	static int is_running = 0;

	struct msgbuf* register_message_envelope = (struct msgbuf*)request_memory_block();
	register_message_envelope->mtype = MESSAGE_TYPE_KCD_COMMAND_REGISTRATION;
	register_message_envelope->mtext[0] = 'W';
	send_message(PROCESS_ID_KCD, (void*)register_message_envelope);

	while (1) {
		struct msgbuf* message = receive_message(NULL);
		if (message == NULL) {
			LOG("ERROR: Wall_clock_proc received a NULL message? wtf.");
			continue;
		}

		switch (message->mtext[2]) {
		case CLOCK_RESET:
			is_running = 1;
			wall_clock_time = 0;
			wall_clock_time_offset = g_timer_count;
			break;

		case CLOCK_SET: {
			// h0h1:m0m1:s0s1
			// TODO check for hh:mm:ss where h or m or s is a character rather than number
			if (message->mtext[12] != '\0' || message->mtext[6] != ':' || message->mtext[9] != ':'){
				LOG("ERROR: incorrect message structure for wall_clock_proc (%WT)");
				continue;
			}
			is_running = 1;

			int h0 = message->mtext[4] - '0';
			int h1 = message->mtext[5] - '0';
			int m0 = message->mtext[7] - '0';
			int m1 = message->mtext[8] - '0';
			int s0 = message->mtext[10] - '0';
			int s1 = message->mtext[11] - '0';

			wall_clock_time
				= h0 * 60 * 60 * 10
				+ h1 * 60 * 60
				+ m0 * 60 * 10
				+ m1 * 60
				+ s0 * 10
			  + s1;

			wall_clock_time_offset = g_timer_count;
			break;
		}
		case CLOCK_TERMINATE:
			is_running = 0;
			release_memory_block(message);
			continue;

		default:
			if (message->mtype != MESSAGE_TYPE_WALL_CLOCK) {
				LOG("ERROR: Wall clock got unrecognized command %d", message->mtype);
				continue;
			}
			break;
		}

		if (is_running) {
			struct msgbuf* message_envelope = (struct msgbuf*)request_memory_block();
			message_envelope->mtype = MESSAGE_TYPE_WALL_CLOCK;
			delayed_send(PROCESS_ID_WALL_CLOCK, (void*)message_envelope, 1000);

			int display_time = (g_timer_count - wall_clock_time_offset) + wall_clock_time*1000;
			mem_clear((char*)message, sizeof(*message));
			message->mtype = MESSAGE_TYPE_CRT_DISPLAY_REQUEST;
			wall_clock_print_time(message->mtext, display_time/1000);
			send_message(PROCESS_ID_CRT, (void*)message);
			LOG("printing time: %s\n", message->mtext);
		}
	}
}


