#include "heap.h"

static unsigned char* free_space_bitmap = NULL;
static HeapBlock* heap_start = NULL;
static int num_blocks = 0;

#define BLOCK_FREE 0x00
#define BLOCK_USED 0xFF

void heap_init(byte* start_address, byte* end_address) {
	free_space_bitmap = (unsigned char*)start_address;
	// +1 for the free_space_bitmap.
	num_blocks = (end_address - start_address) / (HEAP_BLOCK_SIZE + 1);

	for (int i = 0; i < num_blocks; i++) {
		free_space_bitmap[i] = BLOCK_FREE;
	}

	heap_start = (HeapBlock*)(start_address + num_blocks);
}

HeapStatus heap_free_block(HeapBlock* memory_block) {
	int offset = memory_block - heap_start;
	int is_aligned = (offset % HEAP_BLOCK_SIZE) == 0;
	if (!is_aligned) {
		LOG("WARNING: Invalid memory_block passed to heap_free_block.");
		return HEAP_STATUS_INVALID_MEMORY_BLOCK;
	}

	int block_number = offset / HEAP_BLOCK_SIZE;
	if (free_space_bitmap[block_number] != BLOCK_USED) {
		LOG("WARNING: Attempted to double-free memory_block!");
		return HEAP_STATUS_DOUBLE_FREE;
	}

	if (block_number < 0 || block_number > num_blocks) {
		LOG("WARNING: Attempted to free block outside of valid heap region!");
		return HEAP_STATUS_INVALID_MEMORY_BLOCK;
	}

	free_space_bitmap[block_number] = BLOCK_FREE;
	return HEAP_STATUS_OK;
}

static int heap_find_free_block(void) {
	for (int i = 0; i < num_blocks; i++) {
		if (free_space_bitmap[i] == BLOCK_FREE) {
			return i;
		}
	}
	return -1;
}

HeapBlock* heap_alloc_block(void) {
	int first_free_block = heap_find_free_block();
	if (first_free_block < 0) {
		LOG("No free memory blocks.");
		return NULL;
	}
	free_space_bitmap[first_free_block] = BLOCK_USED;
	return &heap_start[first_free_block];
}
