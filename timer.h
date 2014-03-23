#pragma once

#include <stdint.h>
#include "heap_queue.h"

extern void timer_init(void);
extern HeapQueueStatus timer_schedule_delayed_send(HeapBlock* block, int delay_ms);
extern uint32_t timer_elapsed_ms(void);
extern void timer_print_delayed_message_queue(void);
