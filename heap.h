#pragma once

#include "k_rtx.h"

#define HEAP_BLOCK_SIZE 128

typedef unsigned char byte;

typedef struct HeapBlock {
	byte data[HEAP_BLOCK_SIZE];
} HeapBlock;

typedef enum {
	HEAP_STATUS_OK = 0,
	HEAP_STATUS_INVALID_MEMORY_BLOCK = 1,
	HEAP_STATUS_DOUBLE_FREE = 2,
} HeapStatus;

void heap_init(byte* start_address, byte* end_address);
HeapStatus heap_free_block(HeapBlock* memory_block);
HeapBlock* heap_alloc_block(void);
