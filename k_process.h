#pragma once

#include "k_rtx.h"

// User process initial xPSR value
#define INITIAL_xPSR 0x01000000

extern PCB* g_current_process;
extern PCB* s_ready_process_priority_queue[PROCESS_PRIORITY_NUM];

void process_init(void);
int process_create(ProcessInitialState* initial_state);
PCB* scheduler(void);
int k_release_processor(void);
