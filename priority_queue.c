#include "priority_queue.h"

static PCB* s_priority_queue[NUM_PRIORITIES];

PriorityStatus priority_queue_insert(PCB* proc) {
	if (proc == NULL) {	
		return PRIORITY_STATUS_INVALID_PCB;
	}
	if (proc->priority > NUM_PRIORITIES) {
		return PRIORITY_STATUS_INVALID_PRIORITY;
	}

	if (s_priority_queue[proc->priority] == NULL) {
		s_priority_queue[proc->priority] = proc;
	} else {
		PCB* priority_list = s_priority_queue[proc->priority];
		
		while (priority_list->p_next != NULL) {
				priority_list = priority_list->p_next;
		}
		
		proc->p_next = NULL;
		priority_list->p_next = proc;
		
	}
	
	return PRIORITY_STATUS_OK;
}


PCB* priority_queue_pop(void) {
	PCB* priority_list = NULL;
	unsigned int highest_priority;
	for (unsigned int priority = 0; priority < NUM_PRIORITIES; priority++) {
		priority_list = s_priority_queue[priority];
		highest_priority = priority;
		if(priority_list != NULL) break;
	}
	
	if (priority_list == NULL) return NULL;

	PCB* ret = priority_list;
	s_priority_queue[highest_priority] = priority_list->p_next;
	
	return ret;
}

void priority_change(int id, int prev_priority) {
	PCB* priority_list = s_priority_queue[prev_priority];
	
	if (priority_list == NULL)
		return;
	
	if (priority_list->pid == id ) {
		s_priority_queue[prev_priority] = priority_list->p_next;
		priority_list->p_next = NULL;
		priority_queue_insert(priority_list);
		return;
	}
		
	while (priority_list->p_next != NULL) {
		if (priority_list->p_next->pid == id) {
			PCB* target_pcb = priority_list->p_next;
			priority_list->p_next = target_pcb->p_next;
			priority_queue_insert(target_pcb);
			return;
		}
		priority_list = priority_list->p_next;
	}
	
	return;
}

