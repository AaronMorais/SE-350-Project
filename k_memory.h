#pragma once

#include "k_rtx.h"
#include "priority_queue.h"
#include "k_process.h"

extern PCB* s_pcb_allocations_start;
extern unsigned int g_pcb_counter;
extern PCB* g_blocked_process_priority_queue[PROCESS_PRIORITY_NUM];

void memory_init(void);
PCB* memory_alloc_pcb(void);
U32* memory_alloc_stack(U32 size_b);
void memory_init_heap(void);
int memory_release_block(HeapBlock* block);
void* k_request_memory_block(void);
int k_release_memory_block(void*);
void clear_pcb_stack_allocation_ptrs(void);

void* k_memory_block_next(void* p_mem_block);
void k_memory_block_set_next(void* p_mem_block, void* next_mem_block);
