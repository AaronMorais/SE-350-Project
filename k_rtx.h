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

typedef unsigned char U8;
typedef unsigned int U32;

typedef enum {
	PROC_STATE_NEW = 0,
	PROC_STATE_READY = 1,
	PROC_STATE_RUN = 2,
} ProcState;

typedef struct {
	// Stack pointer
	U32 *mp_sp;
	// Process ID
	U32 m_pid;
	ProcState m_state;
	U32 m_priority;
} PCB;

/* initialization table item */
typedef struct proc_init {
	int m_pid;
	int m_priority;
	int m_stack_size;
	void (*mpf_start_pc) ();
} PROC_INIT;
