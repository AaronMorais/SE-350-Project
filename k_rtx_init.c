#include "k_rtx_init.h"
#include "uart_polling.h"
#include "k_memory.h"
#include "k_process.h"
#include "procs/init.h"

// I-procs
#include "procs/uart.h"
#include "procs/timer.h"

void k_rtx_init(void)
{
	__disable_irq();
	
	uart1_init();
	memory_init();
	procs_create_all();
	memory_init_heap();
	
	uart_irq_init(0);
	timer_init();
	
	__enable_irq();
	
	// Start the first process
	k_release_processor();
}
