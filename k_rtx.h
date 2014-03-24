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
	U32*            sp;

	ProcessID       pid;
	ProcessState    state;
	ProcessPriority priority;

	struct PCB*     p_next;
	// Incoming messages, waiting to be processed.
	HeapBlock*      message_queue;
} PCB;

typedef void (*ProcessEntryPoint)();

// Use this one for system procs.
typedef struct {
	int               stack_size;
	ProcessID         pid;
	ProcessPriority   priority;
	ProcessEntryPoint entry_point;
} ProcInit;
