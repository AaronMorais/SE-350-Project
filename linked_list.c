#include "linked_list.h"

MemBlock* gpStartBlock = NULL;
MemBlock* gpEndBlock = NULL;

void PushMemBlock(MemBlock* pBlock) {
	if(gpEndBlock == gpStartBlock) {
		gpEndBlock = pBlock;
		pBlock->pNext = gpStartBlock;
	} else {
		MemBlock* temp = gpEndBlock;
		gpEndBlock = pBlock;
		pBlock->pNext = temp;
	}
	
	return;
}

MemBlock* PopMemBlock() {
	MemBlock* ret = gpEndBlock;
	
	if(gpEndBlock != gpStartBlock) {
		gpEndBlock = gpEndBlock->pNext;
		return ret;
	} else {
		return NULL;
	}

}
