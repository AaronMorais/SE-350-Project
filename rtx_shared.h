// This file should contain all structure and function definitions
// that we want to share between the user and kernel.
#pragma once

#define RTX_OK   0
#define RTX_ERR -1

#define NULL 0

#define NUM_TEST_PROCS 2

#ifdef DEBUG_0
#include "printf.h"
#define LOG(format, ...) printf(format "\r\n", ##__VA_ARGS__)
#else
#define LOG(...)
#endif

typedef enum {
	PROCESS_PRIORITY_HIGH         = 0,
	PROCESS_PRIORITY_MEDIUM       = 1,
	PROCESS_PRIORITY_LOW          = 2,
	PROCESS_PRIORITY_LOWEST       = 3,
	PROCESS_PRIORITY_NULL_PROCESS = 4,

  PROCESS_PRIORITY_NUM          = 5
} ProcessPriority;

struct msgbuf {
  int mtype; /* user defined message type */
  char mtext[1]; /* body of the message */
};

typedef unsigned char U8;
typedef unsigned int U32;
