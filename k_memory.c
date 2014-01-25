/*
This file should contain everything that directly manages memory.

We lay out our RAM something like the following:

0x10000000 +---------------------------+ Low Address
           |                           |
           |       RTX  Image          |
           |...........................|
           |Image$$RW_IRAM1$$ZI$$Limit |
           |---------------------------|
           |         Padding           |
           |---------------------------|
           |          PCB 1            |
           |---------------------------|
           |          PCB 2            |
           |---------------------------|
           |           ...             |
           |---------------------------|
           |          PCB n            |
           |---------------------------|<--- gp_pcb_end
           |                           |
           |          HEAP             |
           |   (shared between all     |
           |        processes)         |
           |                           |
           |---------------------------|<--- gp_stack
           |       Proc n stack        |
           |---------------------------|
           |           ...             |
           |---------------------------|
           |       Proc 2 stack        |
           |---------------------------|
           |       Proc 1 stack        |
0x10008000 +---------------------------+ High Address

Note: PCBs and stacks must be allocated before the heap!

*/
#include "k_memory.h"
#include "linked_list.h"

#ifdef DEBUG_0
#include "printf.h"
#endif

#define RAM_END_ADDR 0x10008000
#define MEM_BLOCK_SIZE 512 // 128 * 4 bytes

// The last allocated stack low address. 8 bytes aligned
// The first stack starts at the RAM high address
// stack grows down. Fully decremental stack.
U32* gp_stack = NULL;
PCB* gp_pcb_end = NULL;


void memory_init(void)
{
	// This symbol is defined in the scatter file (see RVCT Linker User Guide)
	extern unsigned int Image$$RW_IRAM1$$ZI$$Limit;
	U8* p_begin = (U8*)&Image$$RW_IRAM1$$ZI$$Limit;
	
	// Padding. Just to be parinoid.
	p_begin += 32;

	gp_pcb_end = (PCB*)p_begin;

	// allocate memory for stacks
	gp_stack = (U32*)RAM_END_ADDR;
	if ((U32)gp_stack & 0x04) { // 8 byte alignment
		--gp_stack;
	}
}

/**
 * @brief: allocate stack for a process, align to 8 bytes boundary
 * @param: size, stack size in bytes
 * @return: The top of the stack (i.e. high address)
 * POST:  gp_stack is updated.
 */
U32* memory_alloc_stack(U32 size_b)
{
	if (!gp_stack) {
		LOG("Attempted to call memory_alloc_stack after heap has already been created, or before memory_init!");
		return NULL;
	}
	U32* sp = gp_stack; /* gp_stack is always 8 bytes aligned */

	/* update gp_stack */
	gp_stack = (U32*)((U8*)sp - size_b);

	/* 8 bytes alignement adjustment to exception stack frame */
	if ((U32)gp_stack & 0x04) {
		--gp_stack;
	}
	return sp;
}

PCB* memory_alloc_pcb(void)
{
	if (!gp_pcb_end) {
		LOG("Attempted to call memory_alloc_pcb after heap has already been created, or before memory_init!");
		return NULL;
	}
	return gp_pcb_end++;
}

// Note: Make sure this is called *AFTER* all calls to
// memory_alloc_stack and memory_alloc_pcb, otherwise those
// calls will fail!
void memory_init_heap()
{
	gpStartBlock = (MemBlock*)gp_pcb_end;
	gpEndBlock = (MemBlock*)gp_pcb_end;

	U32* endHeap = gp_stack - 32;
	while ((U32*)gp_pcb_end <= endHeap) {
		PushMemBlock((MemBlock*)gp_pcb_end);
		gp_pcb_end += MEM_BLOCK_SIZE;
	}

	gp_pcb_end = NULL;
	gp_stack = NULL;
}

void* k_request_memory_block(void) {
	MemBlock* ret;
#ifdef DEBUG_0 
	printf("k_request_memory_block: entering...\n");
#endif
	
	ret = NULL;
	while (NULL == ret) {
		ret = PopMemBlock();
	}
	printf("request memory block ret %x", ret);
	return (void*) ret;
}

int k_release_memory_block(void* p_mem_blk) {
#ifdef DEBUG_0 
	printf("k_release_memory_block: releasing block @ 0x%x\n", p_mem_blk);
#endif
	// TODO we may need to release it to the highest priority
	// We don't clean because by C convention, everything is instantiated
	PushMemBlock((MemBlock*)p_mem_blk);
	return RTX_OK;
}
