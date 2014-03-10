#include "syscall.h"

#define SVC __svc_indirect(0)

void rtx_init(void)
{
	extern void k_rtx_init(void);
	extern void _rtx_init(U32 p_func) SVC;
	_rtx_init((U32)k_rtx_init);
}

int release_processor(void)
{
	extern int k_release_processor(void);
	extern int _release_processor(U32 p_func) SVC;
	return _release_processor((U32)k_release_processor);
}

void* request_memory_block(void)
{
	extern void* k_request_memory_block(void);
	extern void* _request_memory_block(U32 p_func) SVC;
	return _request_memory_block((U32)k_request_memory_block);
}

int release_memory_block(void* block)
{
	extern int k_release_memory_block(void*);
	extern int _release_memory_block(U32 p_func, void* p_mem_blk) SVC;
	return _release_memory_block((U32)k_release_memory_block, block);
}

int set_process_priority_no_preempt(int process_id, int priority)
{
	extern int k_set_process_priority_no_preempt(int, int);
	extern int _set_process_priority_no_preempt(U32 p_func, int id, int prior) SVC;
	return _set_process_priority_no_preempt((U32)k_set_process_priority_no_preempt, process_id, priority);
}

int set_process_priority(int process_id, int priority)
{
	extern int k_set_process_priority(int, int);
	extern int _set_process_priority(U32 p_func, int id, int prior) SVC;
	return _set_process_priority((U32)k_set_process_priority, process_id, priority);
}

int get_process_priority(int process_id)
{
	extern int k_get_process_priority(int);
	extern int _get_process_priority(U32 p_func, int id) SVC;
	return _get_process_priority((U32)k_get_process_priority, process_id);
}

int send_message(int dest_pid, void* message_envelope)
{
	extern int k_send_message(int, void*);
	extern int _send_message(U32 p_func, int dest_pid, void* message_envelope) SVC;
	return _send_message((U32)k_send_message, dest_pid, message_envelope);
}

void* receive_message(int* sender_pid)
{
	extern void* k_receive_message(int*);
	extern void* _receive_message(U32 p_func, int* sender_pid) SVC;
	return _receive_message((U32)k_receive_message, sender_pid);
}

int delayed_send(int process_id, void* message_envelope, int delay)
{
	extern int k_delayed_send(int, void*, int);
	extern int _delayed_send(U32 p_func, int process_id, void* message_envelope, int delay) SVC;
	return _delayed_send((U32)k_delayed_send, process_id, message_envelope, delay);
}
