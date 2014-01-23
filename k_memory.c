/**
 * @file:   k_memory.c
 * @brief:  kernel memory managment routines
 * @author: Yiqing Huang
 * @date:   2014/01/17
 */

#include "k_memory.h"

#ifdef DEBUG_0
#include "printf.h"
#endif
// The last allocated stack low address. 8 bytes aligned
// The first stack starts at the RAM high address
// stack grows down. Fully decremental stack.
U32 *gp_stack;

/**
 * @brief: Initialize RAM as follows:

0x10008000+---------------------------+ High Address
          |    Proc 1 STACK           |
          |---------------------------|
          |    Proc 2 STACK           |
          |---------------------------|<--- gp_stack
          |                           |
          |        HEAP               |
          |                           |
          |---------------------------|
          |        PCB 2              |
          |---------------------------|
          |        PCB 1              |
          |---------------------------|
          |        PCB pointers       |
          |---------------------------|<--- gp_pcbs
          |        Padding            |
          |---------------------------|  
          |Image$$RW_IRAM1$$ZI$$Limit |
          |...........................|          
          |       RTX  Image          |
          |                           |
0x10000000+---------------------------+ Low Address

*/

void memory_init(void)
{
	U8* p_end = (U8*)&Image$$RW_IRAM1$$ZI$$Limit;
	int i;
  U32* endHeap;
	
	// 8 bytes padding. Just to be parinoid.
	p_end += 32;

	// Allocate memory for pcb pointers
	gp_pcbs = (PCB**)p_end;
	p_end += NUM_TEST_PROCS * sizeof(PCB*);
  
	for (i = 0; i < NUM_TEST_PROCS; i++) {
		gp_pcbs[i] = (PCB*)p_end;
		p_end += sizeof(PCB); 
	}

#ifdef DEBUG_0  
	printf("gp_pcbs[0] = 0x%x \n", gp_pcbs[0]);
	printf("gp_pcbs[1] = 0x%x \n", gp_pcbs[1]);
#endif

	// allocate memory for stacks
	gp_stack = (U32*)RAM_END_ADDR;
	if ((U32)gp_stack & 0x04) { // 8 byte alignment
		--gp_stack; 
	}

	// Allocate memory for heap
  gpStartBlock = (MemBlock*)p_end;
	gpEndBlock = (MemBlock*)p_end;
	endHeap = gp_stack - 32;
	while ((U32*)p_end <= endHeap) {
		PushMemBlock( (MemBlock*)p_end );
		p_end += MEM_BLOCK_SIZE;
	}
}

/**
 * @brief: allocate stack for a process, align to 8 bytes boundary
 * @param: size, stack size in bytes
 * @return: The top of the stack (i.e. high address)
 * POST:  gp_stack is updated.
 */
U32* alloc_stack(U32 size_b)
{
	U32* sp = gp_stack; /* gp_stack is always 8 bytes aligned */
	
	/* update gp_stack */
	gp_stack = (U32*)((U8*)sp - size_b);
	
	/* 8 bytes alignement adjustment to exception stack frame */
	if ((U32)gp_stack & 0x04) {
		--gp_stack; 
	}
	return sp;
}

void* k_request_memory_block(void) {
	MemBlock* ret;
#ifdef DEBUG_0 
	printf("k_request_memory_block: entering...\n");
#endif /* ! DEBUG_0 */
	
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
#endif /* ! DEBUG_0 */
	// TODO we may need to release it to the highest priority
	// We don't clean because by C convention, everything is instantiated
	PushMemBlock((MemBlock*)p_mem_blk);
	return RTX_OK;
}
