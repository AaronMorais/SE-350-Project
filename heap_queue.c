#include "heap_queue.h"

HeapQueueStatus heap_queue_push(HeapBlock** pp_head, HeapBlock* p_block) {
  if(p_block == NULL) {
    return QUEUE_STATUS_INVALID_BLOCK;
  }

  if(*pp_head == NULL) {
    *pp_head = p_block;
  } else {
    HeapBlock* p_temp_block = *pp_head;
    while(p_temp_block->header.p_next != NULL) {
      p_temp_block = p_temp_block->header.p_next;
    }
    p_block->header.p_next = NULL;
    p_temp_block->header.p_next = p_block;
  }

  return QUEUE_STATUS_OK;
}

HeapBlock* heap_queue_pop(HeapBlock** pp_head) {
  if( *pp_head == NULL ) {
    return *pp_head;
  }

  HeapBlock* top = *pp_head;
  *pp_head = (*pp_head)->header.p_next;

  return top;
}

