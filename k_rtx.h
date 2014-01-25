#pragma once

#define RTX_ERR -1
#define RTX_OK  0

#define NULL 0
#define NUM_TEST_PROCS 3

#ifdef DEBUG_0
#define USR_SZ_STACK 0x200         /* user proc stack size 512B   */
#else
#define USR_SZ_STACK 0x100         /* user proc stack size 218B  */
#endif

#define LOG(str) printf(str "\r\n")

typedef unsigned char U8;
typedef unsigned int U32;

typedef enum {
	PROC_STATE_NEW = 0,
	PROC_STATE_READY = 1,
	PROC_STATE_RUN = 2,
} ProcState;

typedef struct {
	// Stack pointer
	U32 *sp;
	// Process ID
	U32 pid;
	ProcState state;
	U32 priority;
} PCB;

/* initialization table item */
typedef struct {
	int pid;
	int priority;
	int stack_size;
	void (*entry_point) ();
} PROC_INIT;
