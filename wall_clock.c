#include "process_id.h"
#include "k_process.h"
#include "syscall.h"
#include "timer.h"

static void wall_clock_process(void);

void wall_clock_create(void) {
	process_create((ProcInit) {
		.pid         = (U32)PROCESS_ID_WALL_CLOCK,
		.priority    = PROCESS_PRIORITY_SYSTEM_PROCESS,
		.stack_size  = 0x200,
		.entry_point = &wall_clock_process,
	});
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
