// This file should contain all structure and function definitions
// that we want to share between the user and kernel.
#pragma once

#define RTX_OK   0
#define RTX_ERR -1

#define NULL 0

#define NUM_TEST_PROCS 10

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

struct msgbuf {
	// TODO: for all of our processes, set the type
	MessageType mtype; /* user defined message type */
	char mtext[1]; /* body of the message */
};

/* initialization table item */
typedef struct {
	int pid;
	int priority;
	int stack_size;
	void (*entry_point) ();
} PROC_INIT;

typedef unsigned char U8;
typedef unsigned int U32;
