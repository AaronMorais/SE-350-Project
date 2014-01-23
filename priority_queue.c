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
	U32 priorityCounter;
	U32 processCounter;
	PCB* node;
	for (priorityCounter = 0; priorityCounter < NUM_PRIORITIES; priorityCounter++) {
		node = gpPriority[priorityCounter][0];
		if (NULL == node) {
			continue;
		}
		for (processCounter = 1; processCounter < NUM_TEST_PROCS; processCounter++) {
			gpPriority[priorityCounter][processCounter - 1] = gpPriority[priorityCounter][processCounter];
		}
		gpPriority[priorityCounter][processCounter - 1] = NULL;
		return node;
	}
	
	return NULL;
}
