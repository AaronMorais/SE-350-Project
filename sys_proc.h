#pragma once

#define NUM_COMMANDS 26;
#define ASCII_START 65;
#define COMMAND_CHAR_NUM 20;
#define COMMAND_NULL '\0';

void sys_proc_init(void);

// Process IDs specified by the manual
typedef enum {
	PROCESS_ID_NULL = 0,
	// Test procs assigned numbers in-between
	PROCESS_ID_KCD = 12,
	PROCESS_ID_CRT = 13,
} SystemPID;

typedef struct {
  int process_id;
  char command;
} RegisteredCommand

