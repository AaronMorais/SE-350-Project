// This file should contain all structure and function definitions
// that we want to share between the user and kernel.
#pragma once

#include <stdbool.h>
#include <stdint.h>

#define RTX_OK   0
#define RTX_ERR -1

#define NULL 0

#define NUM_TEST_PROCS 6

#ifdef DEBUG_0
#include "printf.h"
#define LOG(format, ...) printf(format "\r\n", ##__VA_ARGS__)
#else
#define LOG(...)
#endif

void strcpyn(char* dst, const char* src, int n);
void strcpy(char* dst, const char* src);
bool strequal(const char* a, const char* b);

typedef enum {
	MESSAGE_TYPE_KCD_KEYPRESS_EVENT       = 0,
	MESSAGE_TYPE_KCD_COMMAND_REGISTRATION = 1,
	MESSAGE_TYPE_CRT_DISPLAY_REQUEST      = 2,
	MESSAGE_TYPE_WALL_CLOCK               = 3,
	MESSAGE_TYPE_COUNT_REPORT             = 4,
	MESSAGE_TYPE_WAKEUP_10                = 5,

	MESSAGE_TYPE_NUM                      = 6,
} MessageType;

// Process IDs specified by the manual
typedef enum {
	PROCESS_ID_NULL           = 0,

	// User test procs
	PROCESS_ID_1              = 1,
	PROCESS_ID_2              = 2,
	PROCESS_ID_3              = 3,
	PROCESS_ID_4              = 4,
	PROCESS_ID_5              = 5,
	PROCESS_ID_6              = 6,

	// Stress testing procs
	PROCESS_ID_A              = 7,
	PROCESS_ID_B              = 8,
	PROCESS_ID_C              = 9,

	// User utility procs
	PROCESS_ID_SET_PRIORITY   = 10,
	PROCESS_ID_WALL_CLOCK     = 11,

	// System procs
	PROCESS_ID_KCD            = 12,
	PROCESS_ID_CRT            = 13,

	// i-procs
	PROCESS_ID_TIMER          = 14,
	PROCESS_ID_UART           = 15,
} ProcessID;

// The priority levels exposed to user space.
// KERNEL CODE SHOULD NOT USE THESE, USE ProcessPriority
// enum instead!
typedef enum {
	USER_PROCESS_PRIORITY_HIGH           = 0,
	USER_PROCESS_PRIORITY_MEDIUM         = 1,
	USER_PROCESS_PRIORITY_LOW            = 2,
	USER_PROCESS_PRIORITY_LOWEST         = 3,

	USER_PROCESS_PRIORITY_NUM            = 4,
} UserProcessPriority;

struct msgbuf {
	// TODO: for all of our processes, set the type
	MessageType mtype:32; /* user defined message type */
	// Hard code the size to make debuuging easier
	char mtext[128 - 32/8]; /* body of the message */
};

// Should only be used by user procs.
// Layout CANNOT change, required for ABI compatability with
// testing object code.
typedef struct {
	int pid;
	// The :32 is neccessary for ABI compatibility
	UserProcessPriority priority:32;
	int stack_size;
	void (*entry_point) ();
} PROC_INIT;

typedef unsigned char U8;
typedef unsigned int U32;
