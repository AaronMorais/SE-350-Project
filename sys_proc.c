#include <LPC17xx.h>
#include "syscall.h"
#include "sys_proc.h"
#include "k_rtx.h"
#include "k_process.h"
#include "uart.h"
#include "heap.h"

static void null_process(void);
static void crt_process(void);
static void kcd_process(void);

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
		} else if (message->mtext[0] == '\n') {
			// Newline means command submit without the newline
			strcpy(message->mtext, g_command_buffer);
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

static void uart_write(char* str) {
	if (g_send_char == 1) {
		LOG("Warning: UART write scheduled when UART busy! Ignoring...");
		return;
	}
	g_send_char = 1;
	extern uint8_t *gp_buffer;
	gp_buffer = (uint8_t*)str;

	set_process_priority_no_preempt(PROCESS_ID_CRT, PROCESS_PRIORITY_LOWEST);
	LOG("-----Setting interrupt");
	LPC_UART_TypeDef *pUart = (LPC_UART_TypeDef*) LPC_UART0;
// 	while (1) {
// 		if (g_send_char) {
			pUart->IER = IER_RBR | IER_THRE | IER_RLS;
			pUart->THR = '\0';
// 		} else break;
// 	}\
	
	release_processor();
	pUart->IER = IER_RBR | IER_RLS;
// 	pUart->THR = '\0';
	LOG("-----Completed");
}

static void crt_process() {
	while (1) {
		struct msgbuf* message = receive_message(NULL);
		if (message == NULL || message->mtype != MESSAGE_TYPE_CRT_DISPLAY_REQUEST) {
			LOG("ERROR: CRT_Proc received a message that was not of type CRT_DISPLAY_REQUEST");
			continue;
		}
		LOG("=======Crt process running...");
		uart_write(message->mtext);
		release_memory_block(message);
	}
}
