#include "../syscall.h"
#include "../k_process.h"

static void a_process(void);
static void b_process(void);
static void c_process(void);

void stress_procs_create() {
	process_create((ProcInit) {
		.pid         = PROCESS_ID_A,
		.priority    = PROCESS_PRIORITY_HIGH,
		.stack_size  = 0x200,
		.entry_point = &a_process,
	});

	process_create((ProcInit) {
		.pid         = PROCESS_ID_B,
		.priority    = PROCESS_PRIORITY_HIGH,
		.stack_size  = 0x200,
		.entry_point = &b_process,
	});

	process_create((ProcInit) {
		.pid         = PROCESS_ID_C,
		.priority    = PROCESS_PRIORITY_HIGH,
		.stack_size  = 0x200,
		.entry_point = &c_process,
	});
}

static void a_process() {
	// assuming the KCD releases memory blocks it receives
	struct msgbuf* register_message_envelope = (struct msgbuf*)request_memory_block();

	register_message_envelope->mtype = MESSAGE_TYPE_KCD_COMMAND_REGISTRATION;
	register_message_envelope->mtext[0] = 'Z';
	register_message_envelope->mtext[1] = '\0';
	send_message(PROCESS_ID_KCD, (void*)register_message_envelope);

	while (1) {
		struct msgbuf* command_message = receive_message(NULL);
		bool percent_z_command = strequal(command_message->mtext, "%Z");
		release_memory_block(command_message);
		if (percent_z_command) {
			break;
		}
	}

	int num = 0;
	while (1) {
		struct msgbuf* p = (struct msgbuf*)request_memory_block();
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
			if (next_block == NULL) {
				break;
			}
			p_temp_block = next_block;
		}
		memory_block_set_next(p_block, NULL);
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

static void* s_message_queue = NULL;

static void c_process() {
	while (1) {
		struct msgbuf* p = NULL;

		p = message_queue_pop(&s_message_queue);
		if (!p) {
			p = receive_message(NULL);
		}

		if (p->mtype == MESSAGE_TYPE_COUNT_REPORT) {
			int count = (int)p->mtext[0];

			if ((count % 20) == 0) {
				p->mtype = MESSAGE_TYPE_CRT_DISPLAY_REQUEST;
				strcpy(p->mtext, "Process C\n\r");
				send_message(PROCESS_ID_CRT, p);

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
