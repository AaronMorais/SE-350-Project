#pragma once

#include "k_rtx.h"
#include "linked_list.h"

#define RAM_END_ADDR 0x10008000
#define MEM_BLOCK_SIZE 512 // 128 * 4 bytes

// This symbol is defined in the scatter file (see RVCT Linker User Guide)
extern unsigned int Image$$RW_IRAM1$$ZI$$Limit; 
extern PCB **gp_pcbs;
extern PROC_INIT g_proc_table[NUM_TEST_PROCS];

void memory_init(void);
U32 *alloc_stack(U32 size_b);
void *k_request_memory_block(void);
int k_release_memory_block(void *);
