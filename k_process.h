#pragma once

#include "k_rtx.h"

extern PCB* g_current_process;
extern PCB* g_ready_process_priority_queue[PROCESS_PRIORITY_NUM];

void process_init(void);
int process_create(PROC_INIT* initial_state);
int process_prempt_if_necessary(void);
int k_release_processor(void);
