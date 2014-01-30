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

// There's already a stdio function called "remove..."
static void remove_(PCB* pcb) {
	PriorityStatus status = priority_queue_remove(test_queue, pcb);
	if (status != PRIORITY_STATUS_OK) {
		printf("ERROR, expected status ok, got %d\n", status);
		exit(1);
	}
}

static void reprioritize(PCB* pcb, ProcessPriority new_priority) {
	PriorityStatus status = priority_queue_reprioritize(test_queue, pcb, new_priority);
	if (status != PRIORITY_STATUS_OK) {
		printf("ERROR, expected status ok, got %d\n", status);
		exit(1);
	}
}

static void length(int expected_length) {
	int actual_length = priority_queue_length(test_queue);
	if (actual_length != expected_length) {
		printf("ERROR, expected length %d, got %d\n", expected_length, actual_length);
		exit(1);
	}
}

int test_basic() {
	PCB proc_a = {0};
	proc_a.priority = PROCESS_PRIORITY_MEDIUM;
	PCB proc_b = {0};
	proc_b.priority = PROCESS_PRIORITY_MEDIUM;
	PCB proc_c = {0};
	proc_c.priority = PROCESS_PRIORITY_MEDIUM;

	printf("Running basic test...\n");
	insert(&proc_a);
	insert(&proc_b);
	insert(&proc_c);

	pop(&proc_a);
	pop(&proc_b);
	pop(&proc_c);

	printf("Running remove test...\n");
	insert(&proc_a);
	insert(&proc_b);
	insert(&proc_c);

	length(3);
	remove_(&proc_b);
	length(2);
	insert(&proc_b);
	length(3);

	pop(&proc_a);
	pop(&proc_c);
	pop(&proc_b);

	printf("Running reprioritize test...\n");
	insert(&proc_a);
	insert(&proc_b);
	insert(&proc_c);

	reprioritize(&proc_c, PROCESS_PRIORITY_HIGH);

	pop(&proc_c);
	pop(&proc_a);
	pop(&proc_b);
	pop(NULL);

	printf("Running remove test 2...\n");
	insert(&proc_a);
	length(1);
	remove_(&proc_a);
	length(0);

	printf("Running remove test 3...\n");
	insert(&proc_a);
	insert(&proc_b);
	length(2);
	remove_(&proc_a);
	length(1);
	remove_(&proc_b);
	length(0);
}

int main() {
	test_basic();
	printf("OK [priority_queue]\n");
	return 0;
}
