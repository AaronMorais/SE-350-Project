#include <assert.h>
#include <malloc.h>

#undef NULL
#include "heap.h"

void test_basic() {
	int arena_size = 128*10;
	void* arena = malloc(arena_size);
	heap_init(arena, arena + arena_size);

	HeapStatus status = HEAP_STATUS_OK;

	HeapBlock* mem = heap_alloc_block();
	assert(mem != NULL);

	status = heap_free_block(mem);
	assert(status == HEAP_STATUS_OK);

	status = heap_free_block(NULL);
	assert(status != HEAP_STATUS_OK);

	for (int i = 0; i < 10; i++) {
		mem = heap_alloc_block();
		if (i < 9) {
			assert(mem != NULL);
		} else {
			assert(mem == NULL);
		}
	}
}

int main() {
	test_basic();
	printf("OK [heap]\n");
	return 0;
}
