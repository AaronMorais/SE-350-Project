#include "priority_queue.h"

PriorityStatus priority_queue_insert(PCB** queue, PCB* proc)
{
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

PCB* priority_queue_pop(PCB** queue)
{
	PCB* pcb = priority_queue_top(queue);
	if (pcb) {
		queue[pcb->priority] = pcb->p_next;
		pcb->p_next = NULL;
		return pcb;
	}
	return NULL;
}

PCB* priority_queue_top(PCB** queue)
{
	for (ProcessPriority priority = PROCESS_PRIORITY_SYSTEM_PROCESS; priority < PROCESS_PRIORITY_NUM; priority++) {
		PCB* priority_list = queue[priority];
		if (priority_list != NULL) {
			return priority_list;
		}
	}
	return NULL;
}

PriorityStatus priority_queue_remove(PCB** queue, PCB* pcb)
{
	for (ProcessPriority priority = PROCESS_PRIORITY_SYSTEM_PROCESS; priority < PROCESS_PRIORITY_NUM; priority++) {
		PCB* node = queue[priority];
		PCB* prev = NULL;
		while (node != NULL) {
			if (node == pcb) {
				if (prev) {
					prev->p_next = node->p_next;
				} else {
					queue[priority] = node->p_next;
				}
				node->p_next = NULL;
				return PRIORITY_STATUS_OK;
			}
			prev = node;
			node = node->p_next;
		}
	}
	return PRIORITY_STATUS_INVALID_PCB;
}

PCB* priority_queue_find(PCB** queue, int id)
{
	for (ProcessPriority priority = PROCESS_PRIORITY_SYSTEM_PROCESS; priority < PROCESS_PRIORITY_NUM; priority++) {
		PCB* node = queue[priority];
		while (node != NULL) {
			if (node->pid == id) {
				return node;
			}
			node = node->p_next;
		}
	}
	return NULL;
}

PriorityStatus priority_queue_reprioritize(PCB** queue, PCB* pcb, ProcessPriority new_priority)
{
	PriorityStatus status = PRIORITY_STATUS_OK;
	status = priority_queue_remove(queue, pcb);
	if (status != PRIORITY_STATUS_OK) {
		return status;
	}
	pcb->priority = new_priority;
	return priority_queue_insert(queue, pcb);
}

int priority_queue_length(PCB** queue)
{
	int len = 0;
	for (ProcessPriority priority = PROCESS_PRIORITY_SYSTEM_PROCESS; priority < PROCESS_PRIORITY_NUM; priority++) {
		PCB* node = queue[priority];
		while (node != NULL) {
			len++;
			node = node->p_next;
		}
	}
	return len;
}
