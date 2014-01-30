#include <assert.h>
#include <malloc.h>

#undef NULL
#include "heap.h"

void test_basic() {
	int arena_size = 128*10;
	void* arena = malloc(arena_size);
	heap_init(arena, arena + arena_size);

	HeapStatus status = HEAP_STATUS_OK;

	printf("Running alloc and free two blocks...\n");
	HeapBlock* mem = heap_alloc_block();
	assert(mem != NULL);
	HeapBlock* mem2 = heap_alloc_block();
	assert(mem2 != NULL);

	status = heap_free_block(mem);
	assert(status == HEAP_STATUS_OK);
	status = heap_free_block(mem2);
	assert(status == HEAP_STATUS_OK);


	printf("Running free at invalid alignment...\n");
	mem = heap_alloc_block();
	assert(mem != NULL);
	status = heap_free_block((HeapBlock*)((void*)mem + 1));
	assert(status != HEAP_STATUS_OK);
	status = heap_free_block(mem);
	assert(status == HEAP_STATUS_OK);


	printf("Running alloc/free many times...\n");
	for (int i = 0; i < 1000; i++) {
		mem = heap_alloc_block();
		assert(mem != NULL);
		status = heap_free_block(mem);
		assert(status == HEAP_STATUS_OK);
	}


	printf("Running alloc and then free all blocks...\n");
	HeapBlock* all_blocks[10] = { NULL };
	for (int i = 0; i < 10; i++) {
		all_blocks[i] = heap_alloc_block();
		if (i < 9) {
			assert(all_blocks[i] != NULL);
		} else {
			assert(all_blocks[i] == NULL);
		}
	}
	for (int i = 0; i < 10; i++) {
		status = heap_free_block(all_blocks[i]);
		if (i < 9) {
			assert(status == HEAP_STATUS_OK);
		} else {
			assert(status != HEAP_STATUS_OK);
		}
	}


	printf("Running free invalid block...\n");
	status = heap_free_block(NULL);
	assert(status != HEAP_STATUS_OK);


	printf("Running double-free...\n");
	mem = heap_alloc_block();
	status = heap_free_block(mem);
	assert(status == HEAP_STATUS_OK);
	status = heap_free_block(mem);
	assert(status != HEAP_STATUS_OK);
}

int main() {
	test_basic();
	printf("OK [heap]\n");
	return 0;
}
