

#include "my_malloc.h"
#include <limits.h>

#include <stdlib.h>

unsigned long get_data_segment_size() {
  block *curBlock = head;
  unsigned long totalSize = 0;
  while (curBlock != NULL) {
    totalSize += (unsigned long)(curBlock->size + sizeof(block));
    curBlock = curBlock->next;
  }
  return totalSize;
}
unsigned long get_data_segment_free_space_size() {
  block *curBlock = head->next;
  unsigned long freeSize = 0;
  while (curBlock != NULL) {
    if (curBlock->isFreed == 1) {
      freeSize += (unsigned long)(curBlock->size + sizeof(block));
    }
    curBlock = curBlock->next;
  }
  return freeSize;
}

void blockDelete(block *currBlock) {
  currBlock->isFreed = 0;
  currBlock->free_prev->free_next = currBlock->free_next;
  if (currBlock->free_next != NULL) {
    currBlock->free_next->free_prev = currBlock->prev;
  } // remove currblock from free list
  // currBlock->free_next = NULL;
  // currBlock->free_prev = NULL;
}

void blockAdd(block *newFreeBlock) {
  newFreeBlock->isFreed = 1;
  newFreeBlock->free_next = head->free_next;
  if (head->free_next != NULL) {
    head->free_next->free_prev = newFreeBlock;
  }
  head->free_next = newFreeBlock;
  newFreeBlock->free_prev = head;
}

block *cutBlock(block *parentBlock, size_t requiredSize) {
  block *newBlock = (void *)parentBlock + sizeof(block) + requiredSize;
  newBlock->size = parentBlock->size - requiredSize - sizeof(block);
  blockDelete(parentBlock); // delete parentblock
  blockAdd(newBlock);

  parentBlock->size = requiredSize;
  newBlock->isFreed = 1;
  newBlock->prev = parentBlock;
  newBlock->next = parentBlock->next;

  if (parentBlock->next != NULL) {
    parentBlock->next->prev = newBlock;
  }
  parentBlock->next = newBlock;
  return parentBlock + 1;
}

void mergeBlock(block *currBlock, int index) {
  if (index == -1) {
    currBlock->prev->size += currBlock->size + sizeof(block);
    blockDelete(currBlock);

    currBlock->prev->next = currBlock->next;
    if (currBlock->next != NULL) {
      currBlock->next->prev = currBlock->prev;
    }

  } else {
    currBlock->size += currBlock->next->size + sizeof(block);
    blockDelete(currBlock->next);

    currBlock->next = currBlock->next->next;
    if (currBlock->next != NULL) {
      currBlock->next->prev = currBlock;
    }
  }
}
void *ff_malloc(size_t size) {

  if (size == 0) {
    return NULL;
  } // return  NULL if the request size is 0
  block *currBlock;
  currBlock = head;
  while (currBlock != NULL) {
    if (currBlock->isFreed && currBlock->size == size) {
      currBlock->isFreed = 0;
      blockDelete(currBlock);
      return currBlock + 1;
    } else if (currBlock->isFreed && currBlock->size > size) {
      currBlock->isFreed = 0;

      return cutBlock(currBlock, size);
    } else {

      currBlock = currBlock->free_next;
    }
  } // traverse each block in freeBlock list to search freed block which can
    // satisfied requested size
  // return the first suitable block

  // if failed to find suitable block,use sbrk() to create new block for this
  // malloc request
  size_t alloc_size = size + sizeof(block);
  // if (size < 10000) {
  //  alloc_size = 10000;
  //}
  currBlock = sbrk(alloc_size);
  currBlock->isFreed = 0;
  currBlock->size = size;
  // printf("currBlock is %d its size is %d, sizeof(block) is %d\n", currBlock,
  //     size, sizeof(block));
  currBlock->next = NULL;
  prevBlock->next = currBlock;
  currBlock->prev = prevBlock;
  // if (alloc_size > size + sizeof(block)) {
  //  return cut(currBlock, size);
  //}
  return currBlock + 1;
}

void general_free(void *ptr) {
  //  printf("begin to free!\n");
  block *currBlock = ptr - sizeof(block);
  currBlock->isFreed = 1;
  if (currBlock->prev && currBlock->prev->isFreed) {
    mergeBlock(currBlock, -1);
    return;
  }
  if (currBlock->next && currBlock->next->isFreed) {
    mergeBlock(currBlock, 1);
    return;
  }
}
void ff_free(void *ptr) { general_free(ptr); }

void *bf_malloc(size_t size) {
  if (size == 0) {
    return NULL;
  } // return  NULL if the request size is 0
  block *currBlock, *prevBlock, *diffBlock = NULL;

  currBlock = head;
  size_t difference = INT_MAX;
  while (currBlock != NULL) {
    if (currBlock->isFreed && currBlock->size == size) {
      currBlock->isFreed = 0;
      return currBlock + 1;
    }
    if (currBlock->isFreed && currBlock->size > size) {
      size_t curDifference = currBlock->size - size;
      if (curDifference > 0 && curDifference < difference) {
        difference = curDifference;
        diffBlock = currBlock;
      }
    }
    prevBlock = currBlock;
    currBlock = currBlock->next;

  } // traverse each block in freeBlock list to search freed block which can
    // satisfied requested size or has the min difference
  if (difference != INT_MAX) {
    diffBlock->isFreed = 0;
    return cut(diffBlock, size);
  } // choose the block of smallest difference

  // if failed to find freed block has space bigger than required size,use
  // sbrk() to create new block for this malloc request

  size_t alloc_size = size + sizeof(block);
  // if (size < 10000) {
  //  alloc_size = 10000;
  //}
  currBlock = sbrk(alloc_size);
  currBlock->isFreed = 0;
  currBlock->size = size;
  // printf("currBlock is %d its size is %d, sizeof(block) is %d\n", currBlock,
  //     size, sizeof(block));
  currBlock->next = NULL;
  prevBlock->next = currBlock;
  currBlock->prev = prevBlock;
  // if (alloc_size > size + sizeof(block)) {
  //  return cut(currBlock, size);
  //}
  return currBlock + 1;
}

void bf_free(void *ptr) { general_free(ptr); }
