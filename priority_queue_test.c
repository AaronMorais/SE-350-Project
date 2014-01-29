#include "priority_queue.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static PCB* test_queue[PROCESS_PRIORITY_NUM];

static void insert(PCB* pcb) {
	PriorityStatus status = priority_queue_insert(test_queue, pcb);
	if (status != PRIORITY_STATUS_OK) {
		printf("ERROR, expected status ok, got %d\n", status);
		exit(1);
	}
}

static void pop(PCB* should_be) {
	PCB* actual = priority_queue_pop(test_queue);
	if (actual != should_be) {
		printf("ERROR, expected PCB at %p, got PCB at %p.\n", should_be, actual);
		exit(1);
	}
}

// TODO: We should test some of the more advanced functions of the priority_queue.

int test_basic() {
	PCB proc_a = {0};
	proc_a.priority = 1;
	PCB proc_b = {0};
	proc_b.priority = 1;
	PCB proc_c = {0};
	proc_c.priority = 1;

	insert(&proc_a);
	insert(&proc_b);
	insert(&proc_c);

	pop(&proc_a);
	pop(&proc_b);
	pop(&proc_c);
}

int main() {
	test_basic();
	printf("OK [priority_queue]\n");
	return 0;
}
