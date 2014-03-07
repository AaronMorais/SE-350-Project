#pragma once

#include "rtx_shared.h"
#include "uart_polling.h"

extern uint8_t g_buffer[];

int uart_irq_init(int n_uart); /* initialize uart irq */
extern volatile uint8_t g_send_char;
