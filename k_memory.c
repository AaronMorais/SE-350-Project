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
#include "linked_list.h"

#ifdef DEBUG_0
#include "printf.h"
#endif

#define RAM_END_ADDR 0x10008000
#define MEM_BLOCK_SIZE 512 // 128 * 4 bytes

// The last allocated stack low address. 8 bytes aligned
// The first stack starts at the RAM high address
// stack grows down. Fully decremental stack.
static U32* s_current_stack_allocations_end = NULL;
static PCB* s_current_pcb_allocations_end = NULL;
PCB* s_current_pcb_allocations_start = NULL;
unsigned int g_pcb_counter = 0;

void memory_init(void)
{
	// This symbol is defined in the scatter file (see RVCT Linker User Guide)
	extern unsigned int Image$$RW_IRAM1$$ZI$$Limit;
	U8* p_begin = (U8*)&Image$$RW_IRAM1$$ZI$$Limit;
	
	// 8 bytes padding
	p_begin += 32;

	s_current_pcb_allocations_start = (PCB*)p_begin;

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
	gpStartBlock = (MemBlock*)s_current_pcb_allocations_end;
	gpEndBlock = (MemBlock*)s_current_pcb_allocations_end;

	U32* endHeap = s_current_stack_allocations_end - 32;
	while ((U32*)gpEndBlock <= endHeap) {
		PushMemBlock(gpEndBlock);
		gpEndBlock += MEM_BLOCK_SIZE;
	}
}

// Clearing them so that more processes and  stacks can't be made later
void clear_pcb_stack_allocation_ptrs() {
	s_current_pcb_allocations_end = NULL;
	s_current_stack_allocations_end = NULL;
}

void* k_request_memory_block(void) {
	MemBlock* ret;
	LOG("k_request_memory_block: entering...\n");
	
	ret = NULL;
	while (NULL == ret) {
		ret = PopMemBlock();
	}
	LOG("request memory block ret %x", ret);
	return (void*) ret;
}

int k_release_memory_block(void* p_mem_blk) {
	LOG("k_release_memory_block: releasing block @ 0x%x\n", p_mem_blk);

	// TODO we may need to release it to the highest priority
	// We don't clean because by C convention, everything is instantiated
	PushMemBlock((MemBlock*)p_mem_blk);
	return RTX_OK;
}
