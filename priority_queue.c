#include "priority_queue.h"

PriorityStatus priority_queue_insert(PCB* proc, PCB** ppHead) {
	if (proc == NULL) {
		return PRIORITY_STATUS_INVALID_PCB;
	}
	if (proc->priority > PROCESS_PRIORITY_NUM) {
		return PRIORITY_STATUS_INVALID_PRIORITY;
	}

	if (ppHead[proc->priority] == NULL) {
		ppHead[proc->priority] = proc;
	} else {
		PCB* priority_list = ppHead[proc->priority];

		while (priority_list->p_next != NULL) {
				priority_list = priority_list->p_next;
		}

		proc->p_next = NULL;
		priority_list->p_next = proc;

	}

	return PRIORITY_STATUS_OK;
}


PCB* priority_queue_pop(PCB** ppHead) {
	PCB* priority_list = NULL;
	unsigned int highest_priority;

	for (unsigned int priority = 0; priority < PROCESS_PRIORITY_NUM; priority++) {
		priority_list = ppHead[priority];
		highest_priority = priority;
		if(priority_list != NULL) break;
	}

	if (priority_list == NULL) return NULL;

	PCB* ret = priority_list;
	ppHead[highest_priority] = priority_list->p_next;
	ret->p_next = NULL;

	return ret;
}

void priority_change(int id, int prev_priority, PCB** ppReady) {
	PCB* priority_list = ppReady[prev_priority];

	if (priority_list == NULL)
		return;

	if (priority_list->pid == id ) {
		ppReady[prev_priority] = priority_list->p_next;
		priority_list->p_next = NULL;
		priority_queue_insert(priority_list, ppReady);
		return;
	}

	while (priority_list->p_next != NULL) {
		if (priority_list->p_next->pid == id) {
			PCB* target_pcb = priority_list->p_next;
			priority_list->p_next = target_pcb->p_next;
			priority_queue_insert(target_pcb, ppReady);
			return;
		}
		priority_list = priority_list->p_next;
	}

	return;
}

