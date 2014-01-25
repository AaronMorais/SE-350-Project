#pragma once

#include "rtx_shared.h"

#define LOG(str) printf(str "\r\n")

typedef enum {
	PROCESS_STATE_NEW   = 0,
	PROCESS_STATE_READY = 1,
	PROCESS_STATE_RUN   = 2,
} ProcessState;

typedef struct {
	// Stack pointer
	U32 *sp;
	// Process ID
	U32 pid;
	ProcessState state;
	U32 priority;
} PCB;

/* initialization table item */
typedef struct {
	int pid;
	int priority;
	int stack_size;
	void (*entry_point) ();
} ProcessInitialState;
