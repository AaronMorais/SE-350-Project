#pragma once

int uart_irq_init(int n_uart); /* initialize uart irq */
extern volatile uint8_t g_send_char;
