#pragma once

#include "rtx_shared.h"

typedef enum {
	PROCESS_STATE_NEW                = 0,
	PROCESS_STATE_READY              = 1,
	PROCESS_STATE_RUNNING            = 2,
	PROCESS_STATE_BLOCKED            = 3,
	PROCESS_STATE_BLOCKED_ON_MESSAGE = 4,
} ProcessState;

struct HeapBlock;
typedef struct HeapBlock HeapBlock;

typedef struct PCB {
	// Stack pointer
	U32 *sp;
	// Process ID
	U32 pid;
	ProcessState state;
	U32 priority;
	struct PCB* p_next;
	// Incoming messages, waiting to be processed.
	HeapBlock* message_queue;
} PCB;

/* initialization table item */
typedef struct {
	int pid;
	int priority;
	int stack_size;
	void (*entry_point) ();
} PROC_INIT;
