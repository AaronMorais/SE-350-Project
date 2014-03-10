#include "heap.h"

static U8* s_free_space_bitmap = NULL;
static HeapBlock* s_heap_start = NULL;
static int s_num_blocks = 0;

#define BLOCK_FREE 0x00
#define BLOCK_USED 0xFF

// The heap memory is laid out like this:
//
// +---------------------+ start_address
// | s_free_space_bitmap |
// +---------------------+  + s_num_blocks
// |    heap block #1    |
// + - - - - - - - - - - +  + BLOCK_SIZE
// |    heap block #2    |
// + - - - - - - - - - - +  + BLOCK_SIZE
// |         ...         |
// + - - - - - - - - - - +  + ...
// |     heap block n    |
// +---------------------+ end_address
//
// The free space bitmap keeps track of which blocks have been
// given out, and which blocks are free. The advantage of having
// this information at the start, as opposed to within the blocks,
// is that we can scan the entire table very quickly (since it's
// all contiguous in memory), and there is a reduced possibility
// of corrupting the heap bookkeeping by memory errors. As compared
// to a simple free_list implementation, this also allows us to
// simply and easily gaurd against double frees, frees at invalid
// addresses, and frees for invalid blocks.
void heap_init(byte* start_address, byte* end_address) {
	s_free_space_bitmap = (U8*)start_address;
	// +1 for the s_free_space_bitmap.
	s_num_blocks = (end_address - start_address) / (sizeof(HeapBlock) + 1);

	for (int i = 0; i < s_num_blocks; i++) {
		s_free_space_bitmap[i] = BLOCK_FREE;
	}

	s_heap_start = (HeapBlock*)(start_address + s_num_blocks);
}

HeapStatus heap_free_block(HeapBlock* memory_block) {
	U8* byte_aligned_block = (U8*)memory_block;
	U8* byte_aligned_heap_start = (U8*)s_heap_start;
	int offset = byte_aligned_block - byte_aligned_heap_start;
	int alignment = offset % sizeof(HeapBlock);
	if (alignment != 0) {
		LOG("WARNING: memory_block passed to heap_free_block has invalid alignment %d!", alignment);
		return HEAP_STATUS_INVALID_MEMORY_BLOCK;
	}

	int block_number = offset / sizeof(HeapBlock);
	if (s_free_space_bitmap[block_number] != BLOCK_USED) {
		LOG("WARNING: Attempted to double-free memory_block!");
		return HEAP_STATUS_DOUBLE_FREE;
	}

	if (block_number < 0 || block_number > s_num_blocks) {
		LOG("WARNING: Attempted to free block outside of valid heap region!");
		return HEAP_STATUS_INVALID_MEMORY_BLOCK;
	}

	s_free_space_bitmap[block_number] = BLOCK_FREE;
	return HEAP_STATUS_OK;
}

static int heap_find_free_block(void) {
	for (int i = 0; i < s_num_blocks; i++) {
		if (s_free_space_bitmap[i] == BLOCK_FREE) {
			return i;
		}
	}
	return -1;
}

void mem_clear(char* ptr, char n) {
	for (int i = 0; i < n; i++) {
		ptr[i] = 0x00;
	}
}

HeapBlock* heap_alloc_block(void) {
	int first_free_block = heap_find_free_block();
	if (first_free_block < 0) {
		LOG("No free memory blocks.");
		return NULL;
	}
	s_free_space_bitmap[first_free_block] = BLOCK_USED;
	HeapBlock* block = &s_heap_start[first_free_block];
	mem_clear((char*)block, sizeof(*block));
	return block;
}

HeapBlock* heap_block_from_user_block(void* user_block) {
	return (HeapBlock*)((U8*)user_block - sizeof(HeapBlockHeader));
}

void* user_block_from_heap_block(HeapBlock* heap_block) {
	return (void*) heap_block->data;
}
