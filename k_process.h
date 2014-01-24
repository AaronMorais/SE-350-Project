#pragma once

#include "k_rtx.h"

// User process initial xPSR value
#define INITIAL_xPSR 0x01000000

void process_init(void);
PCB* scheduler(void);
int k_release_process(void);
