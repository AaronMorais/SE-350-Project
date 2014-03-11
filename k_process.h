#pragma once

#include "k_rtx.h"

extern PCB* g_current_process;
extern PCB* g_ready_process_priority_queue[PROCESS_PRIORITY_NUM];

void process_init(void);
int process_create(PROC_INIT* initial_state);
int process_prempt_if_necessary(void);
PCB* process_find(int pid);
int process_send_message(HeapBlock* block);

// Syscall interface
int k_release_processor(void);
int k_set_process_priority(int id, int priority);
int k_get_process_priority(int pid);
int k_send_message(int dest_pid, void* msg);
void* k_receive_message(int* sender_pid);
int k_delayed_send(int process_id, void *message_envelope, int delay);
