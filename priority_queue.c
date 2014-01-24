#include "priority_queue.h"

PCB* gpPriority[NUM_PRIORITIES][NUM_TEST_PROCS] = {NULL};

int priority_queue_insert(PCB* node) {
	U32 counter;
	if (NULL == node) {
		return PRIORITY_STATUS_NULL;
	} else if(NUM_PRIORITIES < node->m_priority) {
		return PRIORITY_STATUS_PRIORITY_ERR;
	}
	
	for (counter = 0; counter < NUM_TEST_PROCS; counter++) {
		if (NULL == gpPriority[node->m_priority][counter]) {
			gpPriority[node->m_priority][counter] = node;
			return PRIORITY_STATUS_OK;
		}
	}
	
	return PRIORITY_STATUS_ERROR;
}

PCB* priority_queue_pop(void) {
	U32 priority;
	U32 process;
	PCB* node;
	for (priority = 0; priority < NUM_PRIORITIES; priority++) {
		node = gpPriority[priority][0];
		if (NULL == node) {
			continue;
		}
		for (process = 1; process < NUM_TEST_PROCS; process++) {
			gpPriority[priority][process - 1] = gpPriority[priority][process];
		}
		gpPriority[priority][NUM_TEST_PROCS - 1] = NULL;
		return node;
	}
	
	return NULL;
}
