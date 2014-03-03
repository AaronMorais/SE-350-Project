#pragma once
#include "heap_queue.h"

extern HeapBlock* g_delayed_msg_list;
extern uint32_t timer_init ( uint8_t n_timer );  /* initialize timer n_timer */
extern volatile uint32_t g_timer_count;
