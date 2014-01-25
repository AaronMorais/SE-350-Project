// This file contains the "user" facing API (i.e. the API provided to
// userland processes that wish to interact with the kernel).
#pragma once

#include "rtx_shared.h"

void rtx_init(void);
int release_processor(void);
void* request_memory_block(void);
int release_memory_block(void*);
