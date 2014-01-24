#include "priority_queue.h"
#include <assert.h>
#include <stdio.h>

void insert(PCB* pcb) {
	PriorityStatus status = priority_queue_insert(pcb);
	if (status != PRIORITY_STATUS_OK) {
		printf("ERROR, expected status ok, got %d", status);
		assert(1);
	}
}

int test_basic() {
	PCB proc_a = {0};
	proc_a.m_priority = 1;
	PCB proc_b = {0};
	proc_b.m_priority = 1;
	PCB proc_c = {0};
	proc_c.m_priority = 1;

	insert(&proc_a);
	insert(&proc_b);

	PCB* tmp = NULL;
	tmp = priority_queue_pop();
	assert(tmp == &proc_a);
	tmp = priority_queue_pop();
	assert(tmp == &proc_b);
}

int main() {
	test_basic();
	return 0;
}
