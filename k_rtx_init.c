/** 
 * @file:   k_rtx_init.c
 * @brief:  Kernel initialization C file
 * @auther: Yiqing Huang
 * @date:   2014/01/17
 */

#include "k_rtx_init.h"
#include "uart_polling.h"
#include "k_memory.h"
#include "k_process.h"

void k_rtx_init(void)
{
	printf("INSIDE K RTX INIT");
	__disable_irq();
	uart0_init();
	memory_init();
	process_init();
	memory_init_heap();
	__enable_irq();
	
	// Start the first process
	k_release_processor();
}
