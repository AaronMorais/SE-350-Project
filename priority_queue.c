#include "priority_queue.h"

PriorityStatus priority_queue_insert(PCB** queue, PCB* proc) {
	if (proc == NULL) {
		return PRIORITY_STATUS_INVALID_PCB;
	}

	if (proc->priority > PROCESS_PRIORITY_NUM) {
		return PRIORITY_STATUS_INVALID_PRIORITY;
	}

	if (queue[proc->priority] == NULL) {
		queue[proc->priority] = proc;
	} else {
		PCB* priority_list = queue[proc->priority];

		while (priority_list->p_next != NULL) {
				priority_list = priority_list->p_next;
		}

		proc->p_next = NULL;
		priority_list->p_next = proc;

	}

	return PRIORITY_STATUS_OK;
}

PCB* priority_queue_pop(PCB** queue) {
  PCB* pcb = priority_queue_top(queue);
  if (pcb) {
    queue[pcb->priority] = pcb->p_next;
    pcb->p_next = NULL;
    return pcb;
  }
  return NULL;
}

PCB* priority_queue_top(PCB** queue) {
  PCB* priority_list = NULL;
  for (unsigned int priority = 0; priority < PROCESS_PRIORITY_NUM; priority++) {
    priority_list = queue[priority];
    if(priority_list == NULL) continue;
    return priority_list;
  }
  return NULL;
}

PriorityStatus priority_queue_remove(PCB** queue, PCB* pcb) {
	for (ProcessPriority priority = PROCESS_PRIORITY_HIGH; priority < PROCESS_PRIORITY_NUM; priority++) {
		PCB* node = queue[priority];
		PCB* prev = NULL;
		while (node != NULL) {
			if (node != pcb) {
				prev = node;
				node = node->p_next;
				continue;
			}
			node->p_next = NULL;
			if (prev) {
				prev->p_next = NULL;
			}
			return PRIORITY_STATUS_OK;
		}
	}
	return PRIORITY_STATUS_INVALID_PCB;
}

PriorityStatus priority_queue_reprioritize(PCB** queue, PCB* pcb, ProcessPriority new_priority) {
	PriorityStatus status = PRIORITY_STATUS_OK;
	status = priority_queue_remove(queue, pcb);
	if (status != PRIORITY_STATUS_OK) return status;
	pcb->priority = new_priority;
	return priority_queue_insert(queue, pcb);
}

// Returns 0 if process was not found
// Returns 1 if process was found
int priority_change(PCB** ppReady, int id, int prev_priority) {
	PCB* priority_list = ppReady[prev_priority];

	if (priority_list == NULL)
		return 0;

	if (priority_list->pid == id ) {
		ppReady[prev_priority] = priority_list->p_next;
		priority_list->p_next = NULL;
		priority_queue_insert(ppReady, priority_list);
		return 1;
	}

	while (priority_list->p_next != NULL) {
		if (priority_list->p_next->pid == id) {
			PCB* target_pcb = priority_list->p_next;
			priority_list->p_next = target_pcb->p_next;
			priority_queue_insert(ppReady, target_pcb);
			return 1;
		}
		priority_list = priority_list->p_next;
	}

	return 0;
}

