/*

The CRT Display Process
This process responds to only one message type: a CRT display request. The message body contains the character string to be displayed. The string may contain control characters (e.g. newline). The process causes the string to be output to the console CRT. In printing to the console display, the process must use the UART i-process. Any message received is freed using the release_memory_block primitive.

*/
#include "system_proc.h"
#include "k_process.h"
#include "k_memory.h"
#include "heap.h"
#include "uart.h"
#include <LPC17xx.h>


