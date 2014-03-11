// This file contains the "user" facing API (i.e. the API provided to
// userland processes that wish to interact with the kernel).
#pragma once

#include "rtx_shared.h"

void rtx_init(void);
int release_processor(void);

void* request_memory_block(void);
int release_memory_block(void*);

int get_process_priority(int process_id);
int set_process_priority(int process_id, int priority);

void* receive_message(int* sender_pid);
int send_message(int dest_pid, void* message_envelope);
int delayed_send(int process_id, void *message_envelope, int delay);
