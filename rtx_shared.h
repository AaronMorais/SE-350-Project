// This file should contain all structure and function definitions
// that we want to share between the user and kernel.
#pragma once

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

typedef enum {
	MESSAGE_TYPE_KCD_KEYPRESS_EVENT       = 0,
	MESSAGE_TYPE_KCD_COMMAND_REGISTRATION = 1,
	MESSAGE_TYPE_CRT_DISPLAY_REQUEST      = 2,
	MESSAGE_TYPE_WALL_CLOCK               = 3,

	MESSAGE_TYPE_NUM                      = 4
} MessageType;

// The priority levels exposed to user space.
// KERNEL CODE SHOULD NOT USE THESE, USE ProcessPriority
// enum instead!
typedef enum {
	USER_PROCESS_PRIORITY_HIGH           = 0,
	USER_PROCESS_PRIORITY_MEDIUM         = 1,
	USER_PROCESS_PRIORITY_LOW            = 2,
	USER_PROCESS_PRIORITY_LOWEST         = 3,

	USER_PROCESS_PRIORITY_NUM            = 4
} UserProcessPriority;

struct msgbuf {
	// TODO: for all of our processes, set the type
	MessageType mtype; /* user defined message type */
	char mtext[1]; /* body of the message */
};

// Should only be used by user procs.
// Layout CANNOT change, required for ABI compatability with
// testing object code.
typedef struct {
	int pid;
	UserProcessPriority priority:32;
	int stack_size;
	void (*entry_point) ();
} PROC_INIT;

typedef unsigned char U8;
typedef unsigned int U32;
