#pragma once

#include "k_rtx.h"

typedef enum {
	PRIORITY_STATUS_OK = 0,
	PRIORITY_STATUS_INVALID_PCB = 1,
	PRIORITY_STATUS_INVALID_PRIORITY = 2
} PriorityStatus;

PriorityStatus priority_queue_insert(PCB** queue, PCB* proc);
PCB* priority_queue_pop(PCB** queue);
PCB* priority_queue_top(PCB** queue);
PriorityStatus priority_queue_remove(PCB** queue, PCB* pcb);
PCB* priority_queue_find(PCB** queue, int id);
PriorityStatus priority_queue_reprioritize(PCB** queue, PCB* pcb, ProcessPriority new_priority);
int priority_queue_length(PCB** queue);
