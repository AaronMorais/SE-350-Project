#include <LPC17xx.h>
#include "../syscall.h"
#include "../uart_polling.h"
#include "process_id.h"

void crt_process() {
	while (1) {
		struct msgbuf* message = receive_message(NULL);
		if (message == NULL || message->mtype != MESSAGE_TYPE_CRT_DISPLAY_REQUEST) {
			LOG("ERROR: CRT_Proc received a message that was not of type CRT_DISPLAY_REQUEST");
			continue;
		}
		LOG("=======Crt process running...");
		send_message(PROCESS_ID_UART, message);
		LPC_UART_TypeDef *pUart = (LPC_UART_TypeDef*) LPC_UART0;
		pUart->IER = IER_RBR | IER_THRE | IER_RLS;
		pUart->THR = '\0';
	}
}
