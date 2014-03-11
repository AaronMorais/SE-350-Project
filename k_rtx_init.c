#include "k_rtx_init.h"
#include "uart_polling.h"
#include "k_memory.h"
#include "k_process.h"
#include "timer.h"

#include "uart.h"
// #include <LPC17xx.h>

void k_rtx_init(void)
{
	__disable_irq();
	uart1_init();
	uart_irq_init(0);
	memory_init();
	process_init();
	memory_init_heap();
	timer_init();
	__enable_irq();
	
	// Start the first process
	k_release_processor();
}
