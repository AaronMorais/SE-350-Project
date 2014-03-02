#pragma once

#include "k_rtx.h"

#define HEAP_BLOCK_SIZE 128

typedef unsigned char byte;

struct HeapBlock;

// Used by the message passing queue. Horrible hacks.
typedef struct HeapBlockHeader {
	int source_pid;
	struct HeapBlock* p_next;
} HeapBlockHeader;

typedef struct HeapBlock {
	HeapBlockHeader header;
	// data is what we return to userland.
	byte            data[HEAP_BLOCK_SIZE];
} HeapBlock;

typedef enum {
	HEAP_STATUS_OK                   = 0,
	HEAP_STATUS_INVALID_MEMORY_BLOCK = 1,
	HEAP_STATUS_DOUBLE_FREE          = 2,
} HeapStatus;

void heap_init(byte* start_address, byte* end_address);
HeapBlock* heap_alloc_block(void);
HeapStatus heap_free_block(HeapBlock* memory_block);
// Get a heap block from a user block pointer.
// TODO: Unit tests for this.
HeapBlock* heap_block_from_user_block(void* user_block);
void* user_block_from_heap_block(HeapBlock* heap_block);
