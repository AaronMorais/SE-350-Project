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
	PROCESS_PRIORITY_SYSTEM_PROCESS = 0,
	PROCESS_PRIORITY_HIGH           = 1,
	PROCESS_PRIORITY_MEDIUM         = 2,
	PROCESS_PRIORITY_LOW            = 3,
	PROCESS_PRIORITY_LOWEST         = 4,
	PROCESS_PRIORITY_NULL_PROCESS   = 5,
	PROCESS_PRIORITY_UNSCHEDULABLE  = 6,

	PROCESS_PRIORITY_NUM            = 7
} ProcessPriority;

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
