#pragma once
#include "heap_queue.h"

HeapBlock* g_delayed_msg_list = NULL;

extern uint32_t timer_init ( uint8_t n_timer );  /* initialize timer n_timer */
int g_timer_count;
