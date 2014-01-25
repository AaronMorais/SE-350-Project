#pragma once

#include "k_rtx.h"

void memory_init(void);
U32* memory_alloc_stack(U32 size_b);
void* k_request_memory_block(void);
int k_release_memory_block(void*);
