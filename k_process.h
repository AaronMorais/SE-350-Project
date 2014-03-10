#pragma once

#include "k_rtx.h"

extern PCB* g_current_process;
extern PCB* g_ready_process_priority_queue[PROCESS_PRIORITY_NUM];

void process_init(void);
int process_create(PROC_INIT* initial_state);
int process_prempt_if_necessary(void);
int k_release_processor(void);
PCB* process_find(int pid);
int k_set_process_priority(int id, int priority);
int k_set_process_priority_no_preempt(int id, int priority);
int k_get_process_priority(int pid);
int k_process_send_message(int dest_pid, HeapBlock* block);
int k_send_message_no_preempt(int dest_pid, void* msg);
int k_send_message(int dest_pid, void* msg);
void* k_receive_message(int* sender_pid);
int k_delayed_send(int process_id, void *message_envelope, int delay);
