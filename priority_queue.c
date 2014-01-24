#include "priority_queue.h"

static PCB* gp_priority_queue[NUM_PRIORITIES][NUM_TEST_PROCS] = {NULL};

int priority_queue_insert(PCB* node) {
	U32 counter;
	if (node == NULL) {
		return PRIORITY_STATUS_NULL;
	} else if (node->m_priority > NUM_PRIORITIES) {
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
	U32 priority;
	U32 process;
	PCB* node;

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
