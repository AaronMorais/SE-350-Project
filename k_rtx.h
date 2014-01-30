#pragma once

#include "rtx_shared.h"

#ifdef DEBUG_0
#include "printf.h"
#define LOG(format, ...) printf(format "\r\n", ##__VA_ARGS__)
#else
#define LOG(...)
#endif

typedef enum {
	PROCESS_STATE_NEW     = 0,
	PROCESS_STATE_READY   = 1,
	PROCESS_STATE_RUNNING  = 2,
	PROCESS_STATE_BLOCKED = 3
} ProcessState;

typedef struct PCB {
	// Stack pointer
	U32 *sp;
	// Process ID
	U32 pid;
	ProcessState state;
	U32 priority;
	struct PCB* p_next;
} PCB;

/* initialization table item */
typedef struct {
	int pid;
	int priority;
	int stack_size;
	void (*entry_point) ();
} ProcessInitialState;
