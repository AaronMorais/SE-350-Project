#pragma once

#include "rtx_shared.h"

typedef enum {
	PROCESS_STATE_NEW                = 0,
	PROCESS_STATE_READY              = 1,
	PROCESS_STATE_RUNNING            = 2,
	PROCESS_STATE_BLOCKED            = 3,
	PROCESS_STATE_BLOCKED_ON_MESSAGE = 4,
} ProcessState;

typedef enum {
	// Avoid "pointless comparision of unsigned integer with zero" warnings
	PROCESS_PRIORITY_INVALID        = 0,
	PROCESS_PRIORITY_SYSTEM_PROCESS = 1,
	PROCESS_PRIORITY_HIGH           = 2,
	PROCESS_PRIORITY_MEDIUM         = 3,
	PROCESS_PRIORITY_LOW            = 4,
	PROCESS_PRIORITY_LOWEST         = 5,
	PROCESS_PRIORITY_NULL_PROCESS   = 6,
	PROCESS_PRIORITY_UNSCHEDULABLE  = 7,

	PROCESS_PRIORITY_NUM            = 8
} ProcessPriority;

struct HeapBlock;
typedef struct HeapBlock HeapBlock;

typedef struct PCB {
	// Stack pointer
	U32 *sp;
	// Process ID
	U32 pid;
	ProcessState state;
	ProcessPriority priority;
	struct PCB* p_next;
	// Incoming messages, waiting to be processed.
	HeapBlock* message_queue;
} PCB;

// Use this one for system procs.
typedef struct {
	int pid;
	ProcessPriority priority;
	int stack_size;
	void (*entry_point)();
} ProcInit;
