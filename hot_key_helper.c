#include "hot_key_helper.h"
#include "printf.h"
#include "k_process.h"
#include "k_memory.h"

#define PRINTED 1
#define EMPTY 0

static void send_to_print(U32 pid, U32 priority) {
	printf("pid = %d, priority = %d \n\r", pid,priority);
}

static int print_priority_queue(PCB** priority_queue_list) {
	int printedStuff = EMPTY;
	for (int i = 0; i < PROCESS_PRIORITY_NUM; i++) {
		// Set the message to be pid (using our specially made itoa) and state
		PCB* node = priority_queue_list[i];
		while( node ) {
			send_to_print(node->pid, node->priority);
			printedStuff = PRINTED;
			node = node->p_next;
		}
	}
	
	return printedStuff;
}

void print_ready_queue() {
	int printedStuff = print_priority_queue(g_ready_process_priority_queue);
	if( printedStuff == EMPTY ) {
		printf("Ready queue is empty \n\r");
	}
}

void print_blocked_memory_queue() {
	int printedStuff = print_priority_queue(g_blocked_process_priority_queue);
	if( printedStuff == EMPTY ) {
		printf("Blocked memory queue is empty \n\r");
	}
}

void print_blocked_receive_queue() {
	int printedStuff = EMPTY;
	for (unsigned int i = 0; i < g_pcb_counter; i++) {
		if (s_pcb_allocations_start[i].state == PROCESS_STATE_BLOCKED_ON_MESSAGE) {
			send_to_print(s_pcb_allocations_start[i].pid, s_pcb_allocations_start[i].priority);
			printedStuff = PRINTED;
		}
	}
	
	if(printedStuff == EMPTY) {
		printf("Blocked message queue is empty \n\r");
	}
}
