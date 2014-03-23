#include <LPC17xx.h>
#include "../syscall.h"
#include "../uart_polling.h"
#include "process_id.h"

void null_process() {
	while (1) {
		release_processor();
		LOG("Running null process");
	}
}

void set_priority_process() {
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

void crt_process() {
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
