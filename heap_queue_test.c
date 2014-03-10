#include "heap_queue.h"
#include <stdio.h>
#include <stdlib.h>

static HeapBlock* test_queue;
static HeapBlock* test_sort_queue;

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

static void test() {
  HeapBlock block1 = {0};
    block1.header.source_pid = 1;
  HeapBlock block2 = {0};
    block2.header.source_pid = 2;
  HeapBlock block3 = {0};
    block3.header.source_pid = 3;

  printf("Running basic tests\n");

  insert(&block1, QUEUE_STATUS_OK);
  insert(&block2, QUEUE_STATUS_OK);
  insert(&block3, QUEUE_STATUS_OK);
  insert(NULL, QUEUE_STATUS_INVALID_BLOCK);

  pop(&block1);
  pop(&block2);
  pop(&block3);
  pop(NULL);

  printf("Running insert popped block\n");

  insert(&block2, QUEUE_STATUS_OK);
  pop(&block2);

  insert(&block3, QUEUE_STATUS_OK);
  pop(&block3);

  insert(&block1, QUEUE_STATUS_OK);
  pop(&block1);
}


static void sort_insert(HeapBlock* queue, HeapQueueStatus should_be) {
  HeapQueueStatus status = sorted_heap_queue_push(&test_sort_queue, queue);
  if(status != should_be) {
    printf("ERROR in sort_insert, expected status ok, got %d\n", status);
    exit(1);
  }
}

static void sort_pop(HeapBlock* should_be) {
  HeapBlock* result = sorted_heap_queue_pop(&test_sort_queue);
  if( result != should_be ) {
    printf("ERROR in sort_pop, expected HeapBlock at %p, got HeapBlock at %p.\n", should_be, result);
    exit(1);
  }
}


static void sort_top(HeapBlock* should_be) {
  HeapBlock* result = sorted_heap_queue_top(&test_sort_queue);
  if( result != should_be ) {
    printf("ERROR in sort_top, expected HeapBlock at %p, got HeapBlock at %p.\n", should_be, result);
    exit(1);
  }
}

void test_sort() {
  HeapBlock block1 = {0};
    block1.header.source_pid = 1;
    block1.header.send_time = 10;
  HeapBlock block2 = {0};
    block2.header.source_pid = 2;
    block2.header.send_time = 15;
  HeapBlock block3 = {0};
    block3.header.source_pid = 3;
    block3.header.send_time = 5;

  printf("Starting sort tests\n");

  sort_insert(&block1, QUEUE_STATUS_OK);
  sort_insert(&block2, QUEUE_STATUS_OK);
  sort_insert(&block3, QUEUE_STATUS_OK);
  sort_insert(NULL, QUEUE_STATUS_INVALID_BLOCK);

  HeapBlock* test = test_sort_queue;
  while( test != NULL ) {
    printf( "test send time: %d\n", test->header.send_time );
    test = test->header.p_next;
  }

  sort_top(&block3);

  sort_pop(&block3);
  sort_pop(&block1);
  sort_pop(&block2);
  sort_pop(NULL);

  sort_insert(&block2, QUEUE_STATUS_OK);
  sort_insert(&block3, QUEUE_STATUS_OK);
  sort_pop(&block3);
  sort_insert(&block1, QUEUE_STATUS_OK);
  sort_pop(&block1);

  printf("End sort tests\n");
}

int main() {
  printf("Testing basic heap\n");
  test();
  printf("End test basic heap\n");

  printf("Testing sorted heap\n");
  test_sort();
  printf("End test sorted heap\n");
  return 0;
}
