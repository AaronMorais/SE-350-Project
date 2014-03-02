#include "heap_queue.h"
#include <stdio.h>
#include <stdlib.h>

static HeapBlock* test_queue;

static void insert(HeapBlock* queue, HeapQueueStatus should_be) {
  HeapQueueStatus status = heap_queue_push(&test_queue, queue);
  if(status != should_be) {
    printf("ERROR in insert, expected status ok, got %d\n", status);
    exit(1);
  }
}

static void pop(HeapBlock* should_be) {
  HeapBlock* result = heap_queue_pop(&test_queue);
  if( result != should_be ) {
    printf("ERROR in pop, expected HeapBlock at %p, got HeapBlock at %p.\n", should_be, result);
    exit(1);
  }
}

void test() {
  HeapBlock block1 = {0};
    block1.header.p_id = 1;
  HeapBlock block2 = {0};
    block2.header.p_id = 2;
  HeapBlock block3 = {0};
    block3.header.p_id = 3;

  printf("Starting basic tests\n");

  insert(&block1, QUEUE_STATUS_OK);
  insert(&block2, QUEUE_STATUS_OK);
  insert(&block3, QUEUE_STATUS_OK);
  insert(NULL, QUEUE_STATUS_INVALID_BLOCK);

  pop(&block1);
  pop(&block2);
  pop(&block3);
  pop(NULL);

  printf("End basic tests\n");
}

int main() {
  test();
  return 0;
}
