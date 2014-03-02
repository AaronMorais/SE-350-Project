#include <stdint.h>

/* The fault handler implementation calls a function called
prvGetRegistersFromStack(). */
__asm void HardFault_Handler(void)
{
	import prvGetRegistersFromStack
	tst lr, #4 
  ite eq                                                    
  mrseq r0, msp                                             
  mrsne r0, psp                                             
  ldr r1, [r0, #24]                                         
  BL prvGetRegistersFromStack
}

/* These are volatile to try and prevent the compiler/linker optimising them
away as the variables never actually get used.  If the debugger won't show the
values of the variables, make them global my moving their declaration outside
of this function. */
volatile uint32_t r0;
volatile uint32_t r1;
volatile uint32_t r2;
volatile uint32_t r3;
volatile uint32_t r12;
volatile uint32_t lr; /* Link register. */
volatile uint32_t pc; /* Program counter. */
volatile uint32_t psr;/* Program status register. */

void prvGetRegistersFromStack(uint32_t* pulFaultStackAddress)
{
    r0 = pulFaultStackAddress[0];
    r1 = pulFaultStackAddress[1];
    r2 = pulFaultStackAddress[2];
    r3 = pulFaultStackAddress[3];

    r12 = pulFaultStackAddress[4];
    lr  = pulFaultStackAddress[5];
    pc  = pulFaultStackAddress[6];
    psr = pulFaultStackAddress[7];

    /* When the following line is hit, the variables contain the register values. */


	while(1) {}
}



