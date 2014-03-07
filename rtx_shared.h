// This file should contain all structure and function definitions
// that we want to share between the user and kernel.
#pragma once

#define RTX_OK   0
#define RTX_ERR -1

#define NULL 0

#define NUM_TEST_PROCS 10

typedef enum {
  TIMER_I_PROCESS_ID = NUM_TEST_PROCS + 1
  UART_PROCESS_ID    = NUM_TEST_PROCS + 2,
  CRT_PROCESS_ID     = NUM_TEST_PROCS + 3,
  KCD_PROCESS_ID     = NUM_TEST_PROCS + 4
} SystemProcPid;

#ifdef DEBUG_0
#include "printf.h"
#define LOG(format, ...) printf(format "\r\n", ##__VA_ARGS__)
#else
#define LOG(...)
#endif

typedef enum {
  PROCESS_PRIORITY_SYSTEM_PROCESS = 0,
  PROCESS_PRIORITY_HIGH           = 1,
  PROCESS_PRIORITY_MEDIUM         = 2,
  PROCESS_PRIORITY_LOW            = 3,
  PROCESS_PRIORITY_LOWEST         = 4,
  PROCESS_PRIORITY_NULL_PROCESS   = 5,

  PROCESS_PRIORITY_NUM            = 6
} ProcessPriority;

typedef enum {
  MESSAGE_TYPE_CRT_DISPLAY_REQUEST   = 0,

  MESSAGE_TYPE_NUM = 1
} MessageType;

struct msgbuf {
  MessageType mtype; /* user defined message type */
  char mtext[1]; /* body of the message */
};

typedef unsigned char U8;
typedef unsigned int U32;
