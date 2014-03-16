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
static void set_priority_process(void);
static void a_process(void);
static void b_process(void);
static void c_process(void);

static void* s_message_queue = NULL;

struct RegisteredCommand g_registered_commands[NUM_COMMANDS];
char g_command_buffer[COMMAND_CHAR_NUM] = {COMMAND_NULL};
int g_cur_command_buffer_index = 0;

void sys_proc_init() {
	// Set up NULL process
	ProcInit null_state;
	null_state.pid = (U32)PROCESS_ID_NULL;
	null_state.priority = PROCESS_PRIORITY_NULL_PROCESS;
	null_state.stack_size = 0x200;
	null_state.entry_point = &null_process;
	process_create(null_state);

	// Set up A process
	ProcInit a_state;
	a_state.pid = (U32)PROCESS_ID_A;
	a_state.priority = PROCESS_PRIORITY_SYSTEM_PROCESS;
	a_state.stack_size = 0x200;
	a_state.entry_point = &a_process;
	process_create(a_state);

	// Set up B process
	ProcInit b_state;
	b_state.pid = (U32)PROCESS_ID_B;
	b_state.priority = PROCESS_PRIORITY_SYSTEM_PROCESS;
	b_state.stack_size = 0x200;
	b_state.entry_point = &b_process;
	process_create(b_state);

	// Set up C process
	ProcInit c_state;
	c_state.pid = (U32)PROCESS_ID_C;
	c_state.priority = PROCESS_PRIORITY_SYSTEM_PROCESS;
	c_state.stack_size = 0x200;
	c_state.entry_point = &c_process;
	process_create(c_state);

	// Set up C process
	ProcInit set_priority_state;
	set_priority_state.pid = (U32)PROCESS_ID_SET_PRIORITY;
	set_priority_state.priority = PROCESS_PRIORITY_SYSTEM_PROCESS;
	set_priority_state.stack_size = 0x200;
	set_priority_state.entry_point = &set_priority_process;
	process_create(set_priority_state);

	// Set up wall_clock process
	ProcInit wall_clock_state;
	wall_clock_state.pid = (U32)PROCESS_ID_WALL_CLOCK;
	wall_clock_state.priority = PROCESS_PRIORITY_SYSTEM_PROCESS;
	wall_clock_state.stack_size = 0x200;
	wall_clock_state.entry_point = &wall_clock_process;
	process_create(wall_clock_state);

	// Set up KCD process
	ProcInit kcd_state;
	kcd_state.pid = (U32)PROCESS_ID_KCD;
	kcd_state.priority = PROCESS_PRIORITY_SYSTEM_PROCESS;
	kcd_state.stack_size = 0x200;
	kcd_state.entry_point = &kcd_process;
	process_create(kcd_state);

	// Set up CRT process
	ProcInit crt_state;
	crt_state.pid = (U32)PROCESS_ID_CRT;
	crt_state.priority = PROCESS_PRIORITY_SYSTEM_PROCESS;
	crt_state.stack_size = 0x200;
	crt_state.entry_point = &crt_process;
	process_create(crt_state);

	// TODO(maybe): Should we give the timer i-process a PCB?

	// Set up UART i-process
	ProcInit uart_state;
	uart_state.pid = (U32)PROCESS_ID_UART;
	uart_state.priority = PROCESS_PRIORITY_UNSCHEDULABLE;
	uart_state.stack_size = 0x200;
	uart_state.entry_point = NULL;
	process_create(uart_state);
}

static void null_process() {
	while (1) {
		release_processor();
		LOG("Running null process");
	}
}

static void a_process() {
	// assuming the KCD releases memory blocks it receives
	struct msgbuf* p = (struct msgbuf*)request_memory_block();

	p->mtype = MESSAGE_TYPE_KCD_COMMAND_REGISTRATION;
	p->mtext[0] = 'Z';
	p->mtext[1] = '\0';
	send_message(PROCESS_ID_KCD, (void*)p);

	while (1) {
		p = receive_message(NULL);
		if (p->mtext[0] == '%' && p->mtext[1] == 'Z') {
			release_memory_block(p);
			break;
		} else {
			release_memory_block(p);
		}
	}

	int num = 0;
	while (1) {
		p = (struct msgbuf*)request_memory_block();
		p->mtype = MESSAGE_TYPE_COUNT_REPORT;
		p->mtext[0] = num;
		send_message(PROCESS_ID_B, (void*)p);
		num = num + 1;
		release_processor();
	}
	// note that Process A does not de-allocate
	// any received messages in the second loop
}

static void b_process() {
	while (1) {
		void* message = receive_message(NULL);
		send_message(PROCESS_ID_C, message);
	}
}

void message_queue_push(void** pp_head, void* p_block) {
  if (p_block == NULL) {
    return;
  }

  if (*pp_head == NULL) {
    *pp_head = p_block;
  } else {
  	void* p_temp_block = *pp_head;
  	while (1) {
    	void* next_block = memory_block_next(p_temp_block);
    	if (next_block) {
    		p_temp_block = next_block;
    	} else {
    		break;
    	}
    }
    memory_block_set_next(p_temp_block, p_block);
  }
}

void* message_queue_pop(void** pp_head) {
  if (*pp_head == NULL) {
    return NULL;
  }

  HeapBlock* top = *pp_head;
  *pp_head = memory_block_next(*pp_head);

  memory_block_set_next(top, NULL);

  return top;
}

static void c_process() {
	while (1) {
		struct msgbuf* p = NULL;

		if (!s_message_queue) {
			p = receive_message(NULL);
		} else {
			p = message_queue_pop(&s_message_queue);
		}

		if (p-> mtype == MESSAGE_TYPE_COUNT_REPORT) {
			int count = (int)p->mtext[0];

			if ((count % 20) == 0) {
				strcpy(p->mtext, "Process C\n\r");

				// hibernate
				struct msgbuf* q = (struct msgbuf*)request_memory_block();
				q->mtype = MESSAGE_TYPE_WAKEUP_10;
				delayed_send(PROCESS_ID_C, q, 10000);
				while (1) {
					p = receive_message(NULL);
					if (p->mtype == MESSAGE_TYPE_WAKEUP_10) {
						break;
					} else {
						message_queue_push(&s_message_queue, p);
					}
				}
			}
		}

		release_memory_block(p);
		release_processor();
	}
}

static void set_priority_process() {
	struct msgbuf* register_message_envelope = (struct msgbuf*)request_memory_block();
	register_message_envelope->mtype = MESSAGE_TYPE_KCD_COMMAND_REGISTRATION;
	register_message_envelope->mtext[0] = 'C';
	register_message_envelope->mtext[1] = '\0';
	send_message(PROCESS_ID_KCD, (void*)register_message_envelope);

	while (1) {
		struct msgbuf* message_envelope = (struct msgbuf*)receive_message(NULL);
		char* buf = &message_envelope->mtext[3];
		int process_id = *buf++ - '0';
		int space = *buf++;
		int new_priority = *buf++ - '0';
		int null = *buf++;

		int valid = 1;
		if (process_id > NUM_TEST_PROCS || process_id <= 0) {
			strcpy(message_envelope->mtext, "Invalid process id!\n\r");
			valid = 0;
		}
		if (space != ' ') {
			strcpy(message_envelope->mtext, "Invalid parameter format\n\r");
			valid = 0;
		}
		if (new_priority >= USER_PROCESS_PRIORITY_NUM || new_priority < 0) {
			strcpy(message_envelope->mtext, "Invalid process priority!\n\r");
			valid = 0;
		}
		if (null != '\0') {
			strcpy(message_envelope->mtext, "Missing null terminator!\n\r");
			valid = 0;
		}

		if (valid) {
			int result = set_process_priority(process_id, new_priority);
			if (result == RTX_OK) {
				strcpy(message_envelope->mtext, "Process priority set!\n\r");
			} else {
				strcpy(message_envelope->mtext, "Error could not set priority!\n\r");
			}
		}

		message_envelope->mtype = MESSAGE_TYPE_CRT_DISPLAY_REQUEST;
		send_message(PROCESS_ID_CRT, (void*)message_envelope);
	}
}

static void wall_clock_print_time(char* buf, int time) {
	int seconds = time / 1000 % 60;
	int minutes = time / 1000 / 60 % 60;
	int hours   = time / 1000 / 60 / 60 % 24;

	int s1 = seconds / 10;
	int s0 = seconds % 10;
	int m1 = minutes / 10;
	int m0 = minutes % 10;
	int h1 = hours   / 10;
	int h0 = hours   % 10;

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

static int wall_clock_parse_time(char* message_buffer) {
	char* buf = &message_buffer[4];
	int h1 = *buf++ - '0';
	int h0 = *buf++ - '0';
	int colon0 = *buf++;
	int m1 = *buf++ - '0';
	int m0 = *buf++ - '0';
	int colon1 = *buf++;
	int s1 = *buf++ - '0';
	int s0 = *buf++ - '0';
	int null = *buf++;

	if (h1 > 2 || h1 < 0 || h0 > 9 || h0 < 0) {
		strcpy(message_buffer, "Invalid hour format!\n\r");
		return -1;
	}
	if (h1 == 2 && h0 > 3) {
		strcpy(message_buffer, "Invalid hour format\n\r");
		return -1;
	}
	if (colon0 != ':') {
		strcpy(message_buffer, "Missing colon\n\r");
		return -1;
	}
	if (m1 > 6 || m1 < 0 || m0 > 9 || m0 < 0) {
		strcpy(message_buffer, "Invalid minute format!\n\r");
		return -1;
	}
	if (colon1 != ':') {
		strcpy(message_buffer, "Missing colon\n\r");
		return -1;
	}
	if (s1 > 6 || s1 < 0 || s0 > 9 || s0 < 0) {
		strcpy(message_buffer, "Invalid second format!\n\r");
		return -1;
	}
	if (null != '\0') {
		strcpy(message_buffer, "Missing null terminator!\n\r");
		return -1;
	}

	return 1000 * (
		  h1 * 60 * 60 * 10
		+ h0 * 60 * 60
		+ m1 * 60 * 10
		+ m0 * 60
		+ s1 * 10
		+ s0);
}

static void wall_clock_process() {
	static const char CLOCK_RESET     = 'R';
	static const char CLOCK_SET       = 'S';
	static const char CLOCK_TERMINATE = 'T';

	// All times are stored in milliseconds
	static int time_base = 0;
	static int is_running = 0;

	struct msgbuf* register_message_envelope = (struct msgbuf*)request_memory_block();
	register_message_envelope->mtype = MESSAGE_TYPE_KCD_COMMAND_REGISTRATION;
	register_message_envelope->mtext[0] = 'W';
	register_message_envelope->mtext[1] = '\0';
	send_message(PROCESS_ID_KCD, (void*)register_message_envelope);

	struct msgbuf* timer_message = (struct msgbuf*)request_memory_block();
	timer_message->mtype = MESSAGE_TYPE_WALL_CLOCK;
	delayed_send(PROCESS_ID_WALL_CLOCK, (void*)timer_message, 1000);

	while (1) {
		struct msgbuf* message = receive_message(NULL);
		if (message == NULL) {
			LOG("ERROR: Wall_clock_proc received a NULL message? wtf.");
			continue;
		}

		switch (message->mtext[2]) {
		case CLOCK_RESET:
			is_running = 1;
			time_base = 0 - timer_elapsed_ms();
			release_memory_block(message);
			break;

		case CLOCK_SET: {
			int new_time_offset = wall_clock_parse_time(message->mtext);
			if (new_time_offset < 0) {
				is_running = 0;
				message->mtype = MESSAGE_TYPE_CRT_DISPLAY_REQUEST;
				send_message(PROCESS_ID_CRT, (void*)message);
				continue;
			}
			is_running = 1;
			time_base = new_time_offset - timer_elapsed_ms();
			release_memory_block(message);
			break;
		}

		case CLOCK_TERMINATE:
			is_running = !is_running;
			release_memory_block(message);
			break;

		default:
			if (message->mtype != MESSAGE_TYPE_WALL_CLOCK) {
				LOG("ERROR: Wall clock got unrecognized command %d", message->mtype);
				release_memory_block(message);
				continue;
			}

			struct msgbuf* timer_message = (struct msgbuf*)request_memory_block();
			timer_message->mtype = MESSAGE_TYPE_WALL_CLOCK;
			delayed_send(PROCESS_ID_WALL_CLOCK, (void*)timer_message, 1000);

			if (is_running) {
				mem_clear((char*)message, sizeof(*message));
				message->mtype = MESSAGE_TYPE_CRT_DISPLAY_REQUEST;
				wall_clock_print_time(message->mtext, timer_elapsed_ms() + time_base);
				LOG("printing time: %s\n", message->mtext);
				send_message(PROCESS_ID_CRT, (void*)message);
			} else {
				release_memory_block(message);
			}
			break;
		}
	}
}

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
		LOG("=======Crt process running...");
		send_message(PROCESS_ID_UART, message);
		LPC_UART_TypeDef *pUart = (LPC_UART_TypeDef*) LPC_UART0;
		pUart->IER = IER_RBR | IER_THRE | IER_RLS;
		pUart->THR = '\0';
	}
}
