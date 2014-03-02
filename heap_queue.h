#pragma once

#include "heap.h"

typedef enum {
  QUEUE_STATUS_OK = 0,
  QUEUE_STATUS_INVALID_BLOCK = 1
} HeapQueueStatus;

HeapQueueStatus heap_queue_push(HeapBlock** pp_head, HeapBlock* p_block);
HeapBlock* heap_queue_pop(HeapBlock** pp_head);
void heap_queue_print(HeapBlock* pp_head);
