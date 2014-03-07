/*

The CRT Display Process
This process responds to only one message type: a CRT display request. The message body contains the character string to be displayed. The string may contain control characters (e.g. newline). The process causes the string to be output to the console CRT. In printing to the console display, the process must use the UART i-process. Any message received is freed using the release_memory_block primitive.

*/
#include "system_proc.h"

void crt_process() {

  while(1) {
      struct msgbuf* message_envelope = k_receive_message(NULL);
      if(message_envelope != NULL &&
         message_envelope->mtype == MESSAGE_TYPE_CRT_DISPLAY_REQUEST) {
          uart_process(message_envelope);
      } else {
        printf("ERROR: CRT_Proc received a message that was not of type CRT_DISPLAY_REQUEST");
      }
  }
}

void uart_process(struct msgbuf* message_envelope) {
  while(1) {
      LPC_UART_TypeDef *pUart = (LPC_UART_TypeDef*) LPC_UART0;
      // write to buffer
      while(g_send_char == 1) {
        pUart->IER = IER_THRE | IER_RLS | IER_RBR;
      }
  }
}