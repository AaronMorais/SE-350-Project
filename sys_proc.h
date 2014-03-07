#pragma once

void sys_proc_init(void);

// Process IDs specified by the manual
typedef enum {
	PROCESS_ID_NULL = 0,
	// Test procs assigned numbers in-between
	PROCESS_ID_KCD = 12,
	PROCESS_ID_CRT = 13,
} SystemPID;
