#include "priority_queue.h"

PriorityStatus priority_queue_insert(PCB** ppHead, PCB* proc) {
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
  PCB* pcb = priority_queue_top(ppHead);
  if (pcb) {
    ppHead[pcb->priority] = pcb->p_next;
    pcb->p_next = NULL;
    return pcb;
  }
  return NULL;
}

PCB* priority_queue_top(PCB** ppHead) {
  PCB* priority_list = NULL;
  for (unsigned int priority = 0; priority < PROCESS_PRIORITY_NUM; priority++) {
    priority_list = ppHead[priority];
    if(priority_list == NULL) continue;
    return priority_list;
  }
  return NULL;
}

//void priority_queue_print(PCB** ppHead) {
//  PCB* priority_list = NULL;
//  for (unsigned int priority = 0; priority < PROCESS_PRIORITY_NUM; priority++) {
//    priority_list = ppHead[priority];
//		LOG("Entered priority #%d", priority);
//    while (priority_list != NULL) {
//			LOG("  pid:%d", priority_list->pid);
//			priority_list = priority_list->p_next;
//		}
//  }
//}

void priority_queue_print(PCB** ppHead) {
	return;
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

