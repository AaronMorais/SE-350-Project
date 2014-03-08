#include "k_process.h"

// TODO: writer helper function itoa
// TODO: write one function for all the printing and pass it a list
// TODO: put blocked on message in a list and refactor our code to use that so we can iterate
// through that list
void print_ready_queue() {
	for (int i = 0; i < PROCESS_PRIORITY_NUM; i++) {
		HeapBlock* block = heap_alloc_block();
		struct msgbuf* message = (struct msgbuf*)user_block_from_heap_block(block);
		message->mtype = MESSAGE_TYPE_CRT_DISPLAY_REQUEST;
		// Set the message to be pid (using our specially made itoa) and state
		//message->mtext[0] = ;
		//message->mtext[1] = '\0';
		// Either send it to KCD or send it straight to CRT
		//k_send_message(PROCESS_ID_KCD, message);
	}
}

void print_blocked_memory_queue() {
	// similar to print_ready_queue
}

void print_blocked_receive_queue() {
	// similar to print_ready_queue
}