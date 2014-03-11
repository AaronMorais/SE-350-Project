#pragma once

#include "heap.h"

typedef enum {
  HEAP_QUEUE_STATUS_OK = 0,
  HEAP_QUEUE_STATUS_INVALID_BLOCK = 1
} HeapQueueStatus;

HeapQueueStatus heap_queue_push(HeapBlock** pp_head, HeapBlock* p_block);
HeapBlock* heap_queue_pop(HeapBlock** pp_head);
HeapBlock* heap_queue_top(HeapBlock** pp_head);

HeapQueueStatus sorted_heap_queue_push(HeapBlock** pp_head, HeapBlock* p_block);
HeapBlock* sorted_heap_queue_top(HeapBlock** pp_head);
HeapBlock* sorted_heap_queue_pop(HeapBlock** pp_head);
