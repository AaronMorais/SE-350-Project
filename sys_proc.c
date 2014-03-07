#include <LPC17xx.h>
#include "syscall.h"
#include "sys_proc.h"
#include "k_rtx.h"
#include "k_process.h"
#include "uart.h"

static void null_process(void);
static void crt_process(void);
static void kcd_process(void);

void sys_proc_init() {
	// Set up NULL process
	PROC_INIT null_state;
	null_state.pid = (U32)PROCESS_ID_NULL;
	null_state.priority = PROCESS_PRIORITY_NULL_PROCESS;
	null_state.stack_size = 0x200;
	null_state.entry_point = &null_process;
	process_create(&null_state);

	// Set up CRT process
	PROC_INIT crt_state;
	crt_state.pid = (U32)PROCESS_ID_CRT;
	crt_state.priority = PROCESS_PRIORITY_SYSTEM_PROCESS;
	crt_state.stack_size = 0x200;
	crt_state.entry_point = &crt_process;
	process_create(&crt_state);

	// Set up KCD process
	PROC_INIT kcd_state;
	kcd_state.pid = (U32)PROCESS_ID_KCD;
	kcd_state.priority = PROCESS_PRIORITY_SYSTEM_PROCESS;
	kcd_state.stack_size = 0x200;
	kcd_state.entry_point = &kcd_process;
	process_create(&kcd_state);
}

static void null_process(){
	while (1) {
		release_processor();
		LOG("Running null process");
	}
}

static void kcd_process() {
	while (1) {
		struct msgbuf* message = receive_message(NULL);
		if (message == NULL || message->mtype != MESSAGE_TYPE_KCD_KEYPRESS_EVENT) {
			LOG("ERROR: KCD_Proc received a message that was not of type KCD_KEYPRESS_EVENT");
			continue;
		}
		message->mtype = MESSAGE_TYPE_CRT_DISPLAY_REQUEST;
		send_message(PROCESS_ID_CRT, message);
	}
}

static void uart_schedule_write(char* str) {
	if (g_send_char == 1) {
		LOG("Warning: UART write scheduled when UART busy! Ignoring...");
		return;
	}
	g_send_char = 1;
	extern uint8_t *gp_buffer;
	// TODO: Buffer size
	gp_buffer = (uint8_t*)str;
	
	LPC_UART_TypeDef *pUart = (LPC_UART_TypeDef*) LPC_UART0;
	pUart->IER = IER_RBR | IER_THRE | IER_RLS;
	LOG("-----Setting interrupt");
}

static void crt_process() {
	while (1) {
		struct msgbuf* message = receive_message(NULL);
		if (message == NULL || message->mtype != MESSAGE_TYPE_CRT_DISPLAY_REQUEST) {
			LOG("ERROR: CRT_Proc received a message that was not of type CRT_DISPLAY_REQUEST");
			continue;
		}
		LOG("=======Crt process running...");
		uart_schedule_write(message->mtext);
		set_process_priority(PROCESS_ID_CRT, PROCESS_PRIORITY_LOWEST);
		release_memory_block(message);
	}
}
