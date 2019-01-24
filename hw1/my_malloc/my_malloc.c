#include "my_malloc.h"
#include <limits.h>

#include <stdlib.h>

block dummy = {0, 0, NULL, NULL, NULL, NULL};
block *head = &dummy;
block *tail = &dummy;
// size_t count = 0;
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
  block *curBlock = head;
  unsigned long freeSize = 0;
  while (curBlock != NULL) {
    freeSize += (unsigned long)(curBlock->size + sizeof(block));
    curBlock = curBlock->free_next;
  }
  return freeSize;
}

void blockDelete(block *currBlock) {

  currBlock->isFreed = 0;

  if (currBlock->free_next != NULL) {

    currBlock->free_next->free_prev = currBlock->free_prev;
  }

  if (currBlock->free_prev != NULL) {

    currBlock->free_prev->free_next = currBlock->free_next;
  }

  currBlock->free_next = NULL;
  currBlock->free_prev = NULL;
}

void blockAdd(block *newFreeBlock) {

  if (head->free_next == NULL) {
    head->free_next = newFreeBlock;
    newFreeBlock->free_prev = head;
  } else {
    head->free_next->free_prev = newFreeBlock;
    newFreeBlock->free_next = head->free_next;
    head->free_next = newFreeBlock;
    newFreeBlock->free_prev = head;
  }
}

void mergeBlock(block *currBlock, int index) {

  if (index == -1) {
    currBlock->prev->size += currBlock->size + sizeof(block);

    blockDelete(currBlock);
    // blockAdd(currBlock->prev);
    currBlock->prev->next = currBlock->next;
    if (currBlock->next != NULL) {
      currBlock->next->prev = currBlock->prev;
    }
    currBlock->prev = NULL;
    currBlock->prev = NULL;
  } else {
    currBlock->size += currBlock->next->size + sizeof(block);
    blockDelete(currBlock->next);
    blockAdd(currBlock);
    currBlock->next = currBlock->next->next;
    if (currBlock->next != NULL) {
      currBlock->next->prev = currBlock;
    }
    // currBlock->next->prev = NULL;
    // currBlock->next->next = NULL;
  }
}

void cutBlock(block *parentBlock, size_t requiredSize) {
  if (parentBlock->size < sizeof(block) + requiredSize) {
    blockDelete(parentBlock);
    return;
  }
  // printf("parent size %d required %d\n", parentBlock->size, requiredSize);
  block *newBlock = (void *)parentBlock + sizeof(block) + requiredSize;

  blockAdd(newBlock);

  blockDelete(parentBlock);

  newBlock->next = parentBlock->next;
  newBlock->prev = parentBlock;
  parentBlock->next = newBlock;
  newBlock->size = parentBlock->size - sizeof(block) - requiredSize;
  newBlock->isFreed = 1;
  parentBlock->size = requiredSize;
  // printf("child size %d  blocksize %d\n", newBlock->size, sizeof(block));
}

void *ff_malloc(size_t size) {
  if (size == 0) {
    return NULL;
  } // return  NULL if the request size is 0
  block *currBlock;
  currBlock = head;
  while (currBlock != NULL) {
    // printf("22\n");
    // printf("freeprev:%d freenext: %d\n", currBlock->free_prev,
    //       currBlock->free_next);
    if (currBlock->size == size) {
      currBlock->isFreed = 0;
      blockDelete(currBlock);
      return currBlock + 1;
    }
    if (currBlock->size > size) {
      currBlock->isFreed = 0;

      cutBlock(currBlock, size);

      return currBlock + 1;
    }

    currBlock = currBlock->free_next;
  }

  // traverse each block in freeBlock list to search freed block which can
  // satisfied requested size
  // return the first suitable block

  // if failed to find suitable block,use sbrk() to create new block for this
  // malloc request
  size_t alloc_size = size + sizeof(block);
  currBlock = sbrk(alloc_size);
  currBlock->isFreed = 0;
  currBlock->size = size;
  currBlock->next = NULL;

  tail->next = currBlock;
  // printf("dummy let it null %d\n", dummy.prev);
  currBlock->prev = tail;
  // printf("sbrk: currblock->prev = %d\n", currBlock->prev);
  tail = currBlock;
  // printf("after this tail is %d\n", tail);
  currBlock->free_next = NULL;
  currBlock->free_prev = NULL;
  return currBlock + 1;
}

void *bf_malloc(size_t size) {
  if (size == 0) {
    return NULL;
  } // return  NULL if the request size is 0
  block *currBlock, *diffBlock = &dummy;

  currBlock = head;
  size_t difference = INT_MAX;
  while (currBlock != NULL) {
    // printf("freeprev:%d freenext: %d\n", currBlock->free_prev,
    //     currBlock->free_next);
    if (currBlock->size == size) {
      currBlock->isFreed = 0;
      blockDelete(currBlock);
      return currBlock + 1;
    }
    if (currBlock->size > size) {
      size_t curDifference = currBlock->size - size;
      if (curDifference > 0 && curDifference < difference) {
        difference = curDifference;
        diffBlock = currBlock;
      }
    }

    currBlock = currBlock->free_next;

  } // traverse each block in freeBlock list to search freed block which can
    // satisfied requested size or has the min difference
  // printf("get differ\n");
  if (difference != INT_MAX) {
    diffBlock->isFreed = 0;

    cutBlock(diffBlock, size);
    return diffBlock + 1;

  } // choose the block of smallest difference

  // if failed to find freed block has space bigger than required size,use
  // sbrk() to create new block for this malloc request
  // printf("cant cut\n");
  size_t alloc_size = size + sizeof(block);
  currBlock = sbrk(alloc_size);
  currBlock->isFreed = 0;
  currBlock->size = size;
  currBlock->next = NULL;
  tail->next = currBlock;
  currBlock->prev = tail;
  tail = currBlock;
  currBlock->free_next = NULL;
  currBlock->free_prev = NULL;
  return currBlock + 1;
}

void general_free(void *ptr) {
  //  printf("begin to free!\n");
  block *currBlock = (void *)(ptr - sizeof(block));
  currBlock->isFreed = 1;

  currBlock->free_next = NULL;
  currBlock->free_prev = NULL;
  blockAdd(currBlock);

  if (currBlock->prev && currBlock->prev->isFreed) {
    mergeBlock(currBlock, -1);
  }
  if (currBlock->next && currBlock->next->isFreed) {
    mergeBlock(currBlock, 1);
  }
  return;
}
void ff_free(void *ptr) { general_free(ptr); }

void bf_free(void *ptr) { general_free(ptr); }
