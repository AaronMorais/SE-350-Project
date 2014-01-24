#include "priority_queue.h"

static PCB* gp_priority_queue[NUM_PRIORITIES][NUM_TEST_PROCS] = {NULL};

int priority_queue_insert(PCB* node) {
	U32 counter = 0;
	if (node == NULL) {
		return PRIORITY_STATUS_NULL;
	}
	if (node->m_priority < 0 || node->m_priority > NUM_PRIORITIES) {
		return PRIORITY_STATUS_PRIORITY_ERR;
	}
	
	for (counter = 0; counter < NUM_TEST_PROCS; counter++) {
		if (gp_priority_queue[node->m_priority][counter] == NULL) {
			gp_priority_queue[node->m_priority][counter] = node;
			return PRIORITY_STATUS_OK;
		}
	}
	
	return PRIORITY_STATUS_ERROR;
}

PCB* priority_queue_pop(void) {
	U32 priority = 0;
	U32 process = 0;
	PCB* node = NULL;
	for (priority = 0; priority < NUM_PRIORITIES; priority++) {
		node = gp_priority_queue[priority][0];
		if (node == NULL) {
			continue;
		}
		for (process = 1; process < NUM_TEST_PROCS; process++) {
			gp_priority_queue[priority][process - 1] = gp_priority_queue[priority][process];
		}
		gp_priority_queue[priority][NUM_TEST_PROCS - 1] = NULL;
		return node;
	}
	
	return NULL;
}
