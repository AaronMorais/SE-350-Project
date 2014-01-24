#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include "k_rtx.h"

#define NUM_PRIORITIES 5

typedef enum {
	PRIORITY_STATUS_OK = 0,
	PRIORITY_STATUS_INVALID_PCB = 1,
	PRIORITY_STATUS_INVALID_PRIORITY = 2,
	PRIORITY_STATUS_QUEUE_FULL = 3
} PriorityStatus;

PriorityStatus priority_queue_insert(PCB* node);

PCB* priority_queue_pop(void);

#endif
