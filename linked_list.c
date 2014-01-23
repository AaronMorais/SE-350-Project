#include "linked_list.h"

MemBlock* gpStartBlock = NULL;
MemBlock* gpEndBlock = NULL;

void PushMemBlock(MemBlock* pBlock) {
	if(gpEndBlock == gpStartBlock) {
		gpStartBlock = pBlock;
		pBlock->pNext = gpEndBlock;
	} else {
		MemBlock* temp = gpStartBlock;
		gpStartBlock = pBlock;
		pBlock->pNext = temp;
	}
	
	return;
}

MemBlock* PopMemBlock() {
	MemBlock* ret = gpStartBlock;
	
	if(gpEndBlock != gpStartBlock) {
		gpStartBlock = gpStartBlock->pNext;
		return ret;
	} else {
		return NULL;
	}

}
