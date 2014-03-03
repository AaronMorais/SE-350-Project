#include "heap_queue.h"

HeapQueueStatus heap_queue_push(HeapBlock** pp_head, HeapBlock* p_block) {
  if (p_block == NULL) {
    return QUEUE_STATUS_INVALID_BLOCK;
  }

  if (*pp_head == NULL) {
    *pp_head = p_block;
  } else {
    HeapBlock* p_temp_block = *pp_head;
    while (p_temp_block->header.p_next != NULL) {
      p_temp_block = p_temp_block->header.p_next;
    }
    p_block->header.p_next = NULL;
    p_temp_block->header.p_next = p_block;
  }

  return QUEUE_STATUS_OK;
}

HeapBlock* heap_queue_pop(HeapBlock** pp_head) {
  if (*pp_head == NULL) {
    return NULL;
  }

  HeapBlock* top = heap_queue_top(pp_head);
  *pp_head = (*pp_head)->header.p_next;

  return top;
}

HeapBlock* heap_queue_top(HeapBlock** pp_head) {
  return *pp_head;
}

HeapQueueStatus sorted_heap_queue_push(HeapBlock** pp_head, HeapBlock* p_block) {
  if( p_block == NULL ) {
    return QUEUE_STATUS_INVALID_BLOCK;
  }

  if( *pp_head == NULL ) {
    *pp_head = p_block;
  } else {
    HeapBlock* p_sort_block = *pp_head;

    if( p_sort_block->header.send_time > p_block->header.send_time) {
      p_block->header.p_next = p_sort_block;
      *pp_head = p_block;
    } else {
      while( p_sort_block->header.p_next != NULL &&
             p_sort_block->header.p_next->header.send_time <= p_block->header.send_time ) {
        p_sort_block = p_sort_block->header.p_next;
      }
      p_block->header.p_next = p_sort_block->header.p_next;
      p_sort_block->header.p_next = p_block;
    }
  }

  return QUEUE_STATUS_OK;
}

HeapBlock* sorted_heap_queue_top(HeapBlock** pp_head) {
  return heap_queue_top(pp_head);
}

HeapBlock* sorted_heap_queue_pop(HeapBlock** pp_head) {
  return heap_queue_pop( pp_head );
}
