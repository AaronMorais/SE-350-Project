#include "../k_process.h"

static void user_test_procs_create(void) {
	// Warning: Be careful when modifying this code! It has been written
	// a bit strangely so that it can continue to link correctly with the
	// TAs code/object files.
	extern void set_test_procs(void);
	set_test_procs();

	extern PROC_INIT g_test_procs[NUM_TEST_PROCS];
	for (int i = 0; i < NUM_TEST_PROCS; i++) {
		PROC_INIT* usr_proc = &g_test_procs[i];
		ProcInit proc = {0};
		int pid = usr_proc->pid;
		if (pid < PROCESS_ID_1 || pid > PROCESS_ID_6) {
			LOG("WARING: Invalid user test process id %d! We may have strange behaviour...", pid);
		}
		proc.pid = (ProcessID)pid;
		proc.priority = user_priority_to_system_priority(usr_proc->priority);
		proc.stack_size = usr_proc->stack_size;
		proc.entry_point = usr_proc->entry_point;
		process_create(proc);
	}
}

void procs_create_all() {
	extern void null_process(void);
	process_create((ProcInit) {
		.pid         = PROCESS_ID_NULL,
		.priority    = PROCESS_PRIORITY_NULL_PROCESS,
		.stack_size  = 0x200,
		.entry_point = &null_process,
	});

//	user_test_procs_create();

	// Stress-testing procs
	extern void stress_procs_create(void);
	stress_procs_create();

	// User util procs
	extern void set_priority_process(void);
	process_create((ProcInit) {
		.pid         = PROCESS_ID_SET_PRIORITY,
		.priority    = PROCESS_PRIORITY_HIGH,
		.stack_size  = 0x200,
		.entry_point = &set_priority_process,
	});
	extern void wall_clock_process(void);
	process_create((ProcInit) {
		.pid         = PROCESS_ID_WALL_CLOCK,
		.priority    = PROCESS_PRIORITY_HIGH,
		.stack_size  = 0x200,
		.entry_point = &wall_clock_process,
	});

	// Console input/output
	extern void kcd_process(void);
	process_create((ProcInit) {
		.pid         = PROCESS_ID_KCD,
		.priority    = PROCESS_PRIORITY_SYSTEM_PROCESS,
		.stack_size  = 0x200,
		.entry_point = &kcd_process,
	});
	extern void crt_process(void);
	process_create((ProcInit) {
		.pid         = PROCESS_ID_CRT,
		.priority    = PROCESS_PRIORITY_SYSTEM_PROCESS,
		.stack_size  = 0x200,
		.entry_point = &crt_process,
	});

	// I-procs
	process_create((ProcInit) {
		.pid         = PROCESS_ID_TIMER,
		.priority    = PROCESS_PRIORITY_UNSCHEDULABLE,
		.stack_size  = 0x0,
		.entry_point = NULL,
	});
	process_create((ProcInit) {
		.pid         = PROCESS_ID_UART,
		.priority    = PROCESS_PRIORITY_UNSCHEDULABLE,
		.stack_size  = 0x0,
		.entry_point = NULL,
	});
}
