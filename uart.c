/**
 * @brief: uart_irq.c
 * @author: NXP Semiconductors
 * @author: Y. Huang
 * @date: 2014/02/08
 */

#include <LPC17xx.h>
#include "uart.h"
#include "uart_polling.h"
#ifdef DEBUG_0
#include "printf.h"
#endif
#include "rtx_shared.h"
#include "k_process.h"
#include "k_memory.h"
#include "heap.h"
#include "heap_queue.h"
#include "sys_proc.h"
#include "hot_key_helper.h"
#include "syscall.h"

#define DEBUG_HOTKEYS

/**
 * @brief: initialize the n_uart
 * NOTES: It only supports UART0. It can be easily extended to support UART1 IRQ.
 * The step number in the comments matches the item number in Section 14.1 on pg 298
 * of LPC17xx_UM
 */
int uart_irq_init(int n_uart) {

	LPC_UART_TypeDef *pUart;

	if ( n_uart ==0 ) {
		/*
		Steps 1 & 2: system control configuration.
		Under CMSIS, system_LPC17xx.c does these two steps

		-----------------------------------------------------
		Step 1: Power control configuration.
		        See table 46 pg63 in LPC17xx_UM
		-----------------------------------------------------
		Enable UART0 power, this is the default setting
		done in system_LPC17xx.c under CMSIS.
		Enclose the code for your refrence
		//LPC_SC->PCONP |= BIT(3);

		-----------------------------------------------------
		Step2: Select the clock source.
		       Default PCLK=CCLK/4 , where CCLK = 100MHZ.
		       See tables 40 & 42 on pg56-57 in LPC17xx_UM.
		-----------------------------------------------------
		Check the PLL0 configuration to see how XTAL=12.0MHZ
		gets to CCLK=100MHZin system_LPC17xx.c file.
		PCLK = CCLK/4, default setting after reset.
		Enclose the code for your reference
		//LPC_SC->PCLKSEL0 &= ~(BIT(7)|BIT(6));

		-----------------------------------------------------
		Step 5: Pin Ctrl Block configuration for TXD and RXD
		        See Table 79 on pg108 in LPC17xx_UM.
		-----------------------------------------------------
		Note this is done before Steps3-4 for coding purpose.
		*/

		/* Pin P0.2 used as TXD0 (Com0) */
		LPC_PINCON->PINSEL0 |= (1 << 4);

		/* Pin P0.3 used as RXD0 (Com0) */
		LPC_PINCON->PINSEL0 |= (1 << 6);

		pUart = (LPC_UART_TypeDef *) LPC_UART0;

	} else if ( n_uart == 1) {

		/* see Table 79 on pg108 in LPC17xx_UM */
		/* Pin P2.0 used as TXD1 (Com1) */
		LPC_PINCON->PINSEL4 |= (2 << 0);

		/* Pin P2.1 used as RXD1 (Com1) */
		LPC_PINCON->PINSEL4 |= (2 << 2);

		pUart = (LPC_UART_TypeDef *) LPC_UART1;

	} else {
		return 1; /* not supported yet */
	}

	/*
	-----------------------------------------------------
	Step 3: Transmission Configuration.
	        See section 14.4.12.1 pg313-315 in LPC17xx_UM
	        for baud rate calculation.
	-----------------------------------------------------
        */

	/* Step 3a: DLAB=1, 8N1 */
	pUart->LCR = UART_8N1; /* see uart.h file */

	/* Step 3b: 115200 baud rate @ 25.0 MHZ PCLK */
	pUart->DLM = 0; /* see table 274, pg302 in LPC17xx_UM */
	pUart->DLL = 9;	/* see table 273, pg302 in LPC17xx_UM */

	/* FR = 1.507 ~ 1/2, DivAddVal = 1, MulVal = 2
	   FR = 1.507 = 25MHZ/(16*9*115200)
	   see table 285 on pg312 in LPC_17xxUM
	*/
	pUart->FDR = 0x21;



	/*
	-----------------------------------------------------
	Step 4: FIFO setup.
	       see table 278 on pg305 in LPC17xx_UM
	-----------------------------------------------------
        enable Rx and Tx FIFOs, clear Rx and Tx FIFOs
	Trigger level 0 (1 char per interrupt)
	*/

	pUart->FCR = 0x07;

	/* Step 5 was done between step 2 and step 4 a few lines above */

	/*
	-----------------------------------------------------
	Step 6 Interrupt setting and enabling
	-----------------------------------------------------
	*/
	/* Step 6a:
	   Enable interrupt bit(s) wihtin the specific peripheral register.
           Interrupt Sources Setting: RBR, THRE or RX Line Stats
	   See Table 50 on pg73 in LPC17xx_UM for all possible UART0 interrupt sources
	   See Table 275 on pg 302 in LPC17xx_UM for IER setting
	*/
	/* disable the Divisior Latch Access Bit DLAB=0 */
	pUart->LCR &= ~(BIT(7));

// 	pUart->IER = IER_RBR | IER_THRE | IER_RLS;
	pUart->IER = IER_RBR | IER_RLS;

	/* Step 6b: enable the UART interrupt from the system level */

	if ( n_uart == 0 ) {
		NVIC_EnableIRQ(UART0_IRQn); /* CMSIS function */
	} else if ( n_uart == 1 ) {
		NVIC_EnableIRQ(UART1_IRQn); /* CMSIS function */
	} else {
		return 1; /* not supported yet */
	}
	LOG("Testing");
// 	pUart->THR = '\0';
// 	g_send_char = 1;
// 	gp_buffer = (uint8_t*)"hello";
	return 0;
}


/**
 * @brief: use CMSIS ISR for UART0 IRQ Handler
 * NOTE: This example shows how to save/restore all registers rather than just
 *       those backed up by the exception stack frame. We add extra
 *       push and pop instructions in the assembly routine.
 *       The actual c_UART0_IRQHandler does the rest of irq handling
 */
__asm void UART0_IRQHandler(void)
{
	PRESERVE8
	IMPORT c_UART0_IRQHandler
	CPSID I
	PUSH{r4-r11, lr}
	BL c_UART0_IRQHandler
	CPSIE I
	POP{r4-r11, pc}
}
/**
 * @brief: c UART0 IRQ Handler
 */

static HeapBlock* s_last_message_block = NULL; 
static char* s_message_buffer = NULL;
void c_UART0_IRQHandler(void)
{
	uint8_t IIR_IntId;	    // Interrupt ID from IIR
	LPC_UART_TypeDef *pUart = (LPC_UART_TypeDef *)LPC_UART0;

#ifdef DEBUG_0
	uart1_put_string("Entering c_UART0_IRQHandler\n\r");
#endif // DEBUG_0

	/* Reading IIR automatically acknowledges the interrupt */
	IIR_IntId = (pUart->IIR) >> 1 ; // skip pending bit in IIR
	if (IIR_IntId & IIR_RDA) { // Receive Data Avaialbe
		/* read UART. Read RBR will clear the interrupt */
		uint8_t g_char_in = pUart->RBR;

#ifdef DEBUG_HOTKEYS
		if( g_char_in == 'a' ) {
			print_ready_queue();
		} else if( g_char_in == 'b' ) {
			print_blocked_memory_queue();
		} else if( g_char_in == 'c') {
			print_blocked_receive_queue();
		}
#endif // DEBUG_HOTKEYS

		HeapBlock* block = heap_alloc_block();
		if (block == NULL) {
			uart1_put_string("Warning: Out of memory. Could not allocate block to send keyboard input to KCD.");
			return;
		}
		
		struct msgbuf* message = (struct msgbuf*)user_block_from_heap_block(block);
		LOG("UART: Alloc'd block %x for char %c", message, g_char_in);
		message->mtype = MESSAGE_TYPE_KCD_KEYPRESS_EVENT;
		message->mtext[0] = g_char_in;
		message->mtext[1] = '\0';
		
		k_send_message(PROCESS_ID_KCD, message);
		return;
	} else if (IIR_IntId & IIR_THRE) {
	/* THRE Interrupt, transmit holding register becomes empty */
		if (s_message_buffer == NULL || *s_message_buffer == '\0') {
			if (s_last_message_block) {
				heap_free_block(s_last_message_block);
				s_last_message_block = NULL;
			}
			PCB* uartPCB = process_find(PROCESS_ID_UART);
			s_last_message_block = heap_queue_pop(&uartPCB->message_queue);
			if (s_last_message_block == NULL) {
				uart1_put_string("Finish writing. Turning off IER_THRE\n\r");
				pUart->IER ^= IER_THRE; // toggle the IER_THRE bit
				pUart->THR = '\0';
				s_message_buffer = NULL;
				return;
			} else {
				s_message_buffer = ((struct msgbuf*)user_block_from_heap_block(s_last_message_block))->mtext;
			}
		}

		
		uint8_t g_char_out = *s_message_buffer;
		uart1_put_string("Writing a char = ");
		uart1_put_char(g_char_out);
		uart1_put_string("\n\r");

		pUart->THR = g_char_out;
		s_message_buffer++;
	} else {  /* not implemented yet */
#ifdef DEBUG_0
			printf("UART RLS Interrupt %x\n\r", pUart->LSR);
#endif // DEBUG_0
		return;
	}
}
