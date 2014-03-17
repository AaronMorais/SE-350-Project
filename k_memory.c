/*
This file should contain everything that directly manages memory.

We lay out our RAM something like the following (see the lab manual for further details)

0x00000000 +---------------------------+ Low Address (RO microflash)
           |                           |
           |    OS Code (read-only)    |
           |                           |
0x00080000 +---------------------------|
           .                           .
                Unmapped addresses
           .                           .
0x10000000 |---------------------------|<--- Start of RAM (R/W)
           |                           |
           |       RTX  Image          |
           | (Static/global variables) |
           |                           |
           |---------------------------|<--- Image$$RW_IRAM1$$ZI$$Limit
           |         Padding           |
           |---------------------------|
           |          PCB 1            |
           |---------------------------|
           |          PCB 2            |
           |---------------------------|
           |           ...             |
           |---------------------------|
           |          PCB n            |
           |---------------------------|<--- s_current_pcb_allocations_end
           |                           |
           |          HEAP             |
           |   (shared between all     |
           |        processes)         |
           |                           |
           |---------------------------|<--- s_current_stack_allocations_end
           |       Proc n stack        |
           |---------------------------|
           |           ...             |
           |---------------------------|
           |       Proc 2 stack        |
           |---------------------------|
           |       Proc 1 stack        |
0x10008000 +---------------------------+ High Address

Note: PCBs and stacks must be allocated before the heap (because otherwise,
we can't know where the heap should go)!

*/
#include "k_memory.h"
#include "heap.h"
#include "priority_queue.h"

#ifdef DEBUG_0
#include "printf.h"
#endif

#define RAM_END_ADDR 0x10008000

// The last allocated stack low address. 8 bytes aligned
// The first stack starts at the RAM high address
// stack grows down. Fully decremental stack.
static U32* s_current_stack_allocations_end = NULL;
static PCB* s_current_pcb_allocations_end = NULL;

PCB* g_blocked_process_priority_queue[PROCESS_PRIORITY_NUM];
PCB* s_pcb_allocations_start = NULL;
unsigned int g_pcb_counter = 0;

void memory_init(void)
{
	// This symbol is defined in the scatter file (see RVCT Linker User Guide)
	extern unsigned int Image$$RW_IRAM1$$ZI$$Limit;
	U8* p_begin = (U8*)&Image$$RW_IRAM1$$ZI$$Limit;

	// 4 bytes padding
	p_begin += 4;

	s_pcb_allocations_start = (PCB*)p_begin;

	s_current_pcb_allocations_end = (PCB*)p_begin;

	// allocate memory for stacks
	s_current_stack_allocations_end = (U32*)RAM_END_ADDR;
	if ((U32)s_current_stack_allocations_end & 0x04) { // 8 byte alignment
		--s_current_stack_allocations_end;
	}
}

/**
 * @brief: allocate stack for a process, align to 8 bytes boundary
 * @param: size, stack size in bytes
 * @return: The top of the stack (i.e. high address)
 * POST:  s_current_stack_allocations_end is updated.
 */
U32* memory_alloc_stack(U32 size_b)
{
	if (!s_current_stack_allocations_end) {
		LOG("Attempted to call memory_alloc_stack after heap has already been created, or before memory_init!");
		return NULL;
	}
	// s_current_stack_allocations_end is always 8 bytes aligned
	U32* sp = s_current_stack_allocations_end;

	s_current_stack_allocations_end = (U32*)((U8*)sp - size_b);

	/* 8 bytes alignement adjustment to exception stack frame */
	if ((U32)s_current_stack_allocations_end & 0x04) {
		--s_current_stack_allocations_end;
	}
	return sp;
}

PCB* memory_alloc_pcb(void)
{
	if (!s_current_pcb_allocations_end) {
		LOG("Attempted to call memory_alloc_pcb after heap has already been created, or before memory_init!");
		return NULL;
	}
	g_pcb_counter++;
	return s_current_pcb_allocations_end++;
}

// Note: Make sure this is called *AFTER* all calls to
// memory_alloc_stack and memory_alloc_pcb, otherwise those
// calls will fail!
void memory_init_heap()
{
	heap_init((U8*)s_current_pcb_allocations_end, (U8*)s_current_stack_allocations_end);

	// Reset the stack and pcb allocation pointers to ensure we don't
	// accidentally double-allocate the memory, and fail instead.
	s_current_pcb_allocations_end = NULL;
	s_current_stack_allocations_end = NULL;
}

void* k_request_memory_block(void) {
	HeapBlock* block = heap_alloc_block();
	while (block == NULL) {
		g_current_process->state = PROCESS_STATE_BLOCKED;
		priority_queue_insert(g_blocked_process_priority_queue, g_current_process);

		// Block until a memory block becomes available
		k_release_processor();

		block = heap_alloc_block();
		if (block == NULL) {
			LOG("Warning: Blocked process %d scheduled to run when no blocks free!", g_current_process->pid);
		}
	}

	LOG("alloc'd block 0x%x to process %d", block, g_current_process->pid);
	return user_block_from_heap_block(block);
}

int k_release_memory_block(void* p_mem_blk) {
	HeapBlock* block = heap_block_from_user_block(p_mem_blk);
	LOG("k_release_memory_block: releasing block @ 0x%x from process %d", block, g_current_process->pid);

	HeapStatus status = heap_free_block(block);
	if (status != HEAP_STATUS_OK) {
		LOG("Heap returned error code %d", status);
		return RTX_ERR;
	}

	PCB* blocked_process = priority_queue_pop(g_blocked_process_priority_queue);
	if (blocked_process == NULL) {
		// Nobody waiting
		return RTX_OK;
	}

	blocked_process->state = PROCESS_STATE_READY;
	priority_queue_insert(g_ready_process_priority_queue, blocked_process);

	return process_prempt_if_necessary();
}

void* k_memory_block_next(void* p_mem_blk) {
	if (!p_mem_blk) return NULL;

	HeapBlock* block = heap_block_from_user_block(p_mem_blk);
	HeapBlock* next_block = block->header.p_next_usr;
	if (next_block == NULL) {
		return NULL;
	}
	return user_block_from_heap_block(next_block);
}

void k_memory_block_set_next(void* p_mem_blk, void* next_mem_blk) {
	if (!p_mem_blk) return;

	HeapBlock* block = heap_block_from_user_block(p_mem_blk);
	HeapBlock* next_block = heap_block_from_user_block(next_mem_blk);
	if (block) {
		block->header.p_next_usr = next_block;
	}
}
