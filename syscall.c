#include "syscall.h"

void rtx_init(void)
{
	extern void k_rtx_init(void);
	extern void _rtx_init(U32 p_func) __svc_indirect(0);
	_rtx_init((U32)k_rtx_init);
}

int release_processor(void)
{
	extern int k_release_processor(void);
	extern int _release_processor(U32 p_func) __svc_indirect(0);
	return _release_processor((U32)k_release_processor);
}

void* request_memory_block(void)
{
	extern void* k_request_memory_block(void);
	extern void* _request_memory_block(U32 p_func) __svc_indirect(0);
	return _request_memory_block((U32)k_request_memory_block);
}

int release_memory_block(void* block)
{
	extern int k_release_memory_block(void*);
	extern int _release_memory_block(U32 p_func, void* p_mem_blk) __svc_indirect(0);
	return _release_memory_block((U32)k_release_memory_block, block);
}
