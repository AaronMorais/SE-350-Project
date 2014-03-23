#include "../syscall.h"

static const char* read_int(const char* buf, int* result) {
	int n = 0;
	char c = *buf;
	bool valid = false;
	while (c >= '0' && c <= '9') {
		n = n*10 + (c - '0');
		buf++;
		c = *buf;
		valid = true;
	}
	if (valid) {
		*result = n;
	}
	return buf;
}

static const char* handle_set_priority_request(const char* buf) {
	int process_id = -1;
	buf = read_int(buf, &process_id);
	if (process_id == -1) {
		return "Invalid process id!\n\r";
	}
	int space = *buf++;
	if (space != ' ') {
		return "Invalid parameter format\n\r";
	}
	int new_priority = -1;
	buf = read_int(buf, &new_priority);
	if (new_priority == -1) {
		return "Invalid process priority!\n\r";
	}
	int null = *buf++;
	if (null != '\0') {
		return "Missing null terminator!\n\r";
	}

	int result = set_process_priority(process_id, new_priority);
	if (result == RTX_OK) {
		return "Process priority set!\n\r";
	} else {
		return "Error: Could not set priority!\n\r";
	}
}

void set_priority_process() {
	struct msgbuf* register_msg = (struct msgbuf*)request_memory_block();
	register_msg->mtype = MESSAGE_TYPE_KCD_COMMAND_REGISTRATION;
	strcpy(register_msg->mtext, "C");
	send_message(PROCESS_ID_KCD, (void*)register_msg);

	while (1) {
		struct msgbuf* msg = (struct msgbuf*)receive_message(NULL);

		const char* result = handle_set_priority_request(&msg->mtext[3]);

		msg->mtype = MESSAGE_TYPE_CRT_DISPLAY_REQUEST;
		strcpy(msg->mtext, result);
		send_message(PROCESS_ID_CRT, (void*)msg);
	}
}
