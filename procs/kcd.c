#include "process_id.h"
#include "../syscall.h"
// TODO: Remove heap.h
#include "../heap.h"

#define NUM_COMMANDS 26
#define ASCII_START 65
#define COMMAND_CHAR_NUM 20
#define COMMAND_NULL '\0'

struct RegisteredCommand {
	int  process_id;
	char command;
};

struct RegisteredCommand g_registered_commands[NUM_COMMANDS];
char g_command_buffer[COMMAND_CHAR_NUM] = {COMMAND_NULL};
int g_cur_command_buffer_index = 0;

static void init_kcd_process() {
	for( int i = 0; i < NUM_COMMANDS; i++ ) {
		g_registered_commands[i].process_id = -1;
		// 65 is the ASCII character A
		g_registered_commands[i].command = (char)(i + ASCII_START);
	}
}

static void kcd_process_clear_command_buffer() {
	for (int i = 0; i <= COMMAND_CHAR_NUM; i++) {
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
		} else if (message->mtext[0] == '\r') {
			message->mtext[1] = '\n';
			message->mtext[2] = '\0';
		}
		// Pass the message to CRT to print and clear block
		kcd_process_send_message(PROCESS_ID_CRT, message);
	} else if (g_cur_command_buffer_index == 1) {
		// If it second character isn't a command, we send a message to CRT to print shit
		if(message->mtext[0] == (char)0x7f) {
			backspace();
			kcd_process_send_message(PROCESS_ID_CRT, message);
		} else if (message->mtext[0] == '\r') {
			message->mtext[1] = '\n';
			message->mtext[2] = '\0';
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
 		if (message->mtext[0] == (char)0x7f) {
			backspace();
			kcd_process_send_message(PROCESS_ID_CRT, message);
		} else if (message->mtext[0] == '\r') {
			// Newline means command submit without the newline
			struct msgbuf* crt_msg = request_memory_block();
			crt_msg->mtext[0]='\n';
			crt_msg->mtext[1]='\r';
			crt_msg->mtext[2]='\0';
			kcd_process_send_message(PROCESS_ID_CRT, crt_msg);
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

		// TODO: We should use the value receive_message gives us
		// instead of traversing into the heap block.
		HeapBlock* block = heap_block_from_user_block( (void*)message );
		g_registered_commands[index].process_id = block->header.source_pid;
}

void kcd_process() {
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
