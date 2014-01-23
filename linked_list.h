#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

#define NULL 0

typedef struct MemBlock {
	struct MemBlock* pNext;
} MemBlock;

extern MemBlock* gpStartBlock;
extern MemBlock* gpEndBlock;

void PushMemBlock(MemBlock* pBlock);

// Returns a null if there is nothing to pop
MemBlock* PopMemBlock(void);

#endif
