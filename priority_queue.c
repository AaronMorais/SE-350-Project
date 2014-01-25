#include "priority_queue.h"

static PCB* gp_priority_queue[NUM_PRIORITIES][NUM_TEST_PROCS] = {NULL};

PriorityStatus priority_queue_insert(PCB* proc) {
	if (proc == NULL) {
		return PRIORITY_STATUS_INVALID_PCB;
	}
	if (proc->priority < 0 || proc->priority > NUM_PRIORITIES) {
		return PRIORITY_STATUS_INVALID_PRIORITY;
	}

	PCB** processes = gp_priority_queue[proc->priority];
	for (int i = 0; i < NUM_TEST_PROCS; i++) {
		if (processes[i] == NULL) {
			processes[i] = proc;
			return PRIORITY_STATUS_OK;
		}
	}

	return PRIORITY_STATUS_QUEUE_FULL;
}

PCB* priority_queue_pop(void) {
	for (int priority = 0; priority < NUM_PRIORITIES; priority++) {
		PCB** processes = gp_priority_queue[priority];
		PCB* proc = processes[0];
		if (proc == NULL) {
			continue;
		}
		for (int process = 1; process < NUM_TEST_PROCS; process++) {
			processes[process - 1] = processes[process];
		}
		processes[NUM_TEST_PROCS - 1] = NULL;
		return proc;
	}
	
	return NULL;
}
