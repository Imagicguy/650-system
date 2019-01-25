#include "my_malloc.h"
#include <limits.h>
#include <unistd.h>
block dummy = {0, 0, NULL, NULL, NULL, NULL};
block *head = &dummy;
block *tail = &dummy;
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
void freeAdd(block *block) {
  block->isFreed = 1;
  block->free_prev = head;
  block->free_next = head->free_next;
  if (head->free_next != NULL) {
    head->free_next->free_prev = block;
  }
  head->free_next = block;
}

void freeDelete(block *block) {
  block->free_prev->free_next = block->free_next;
  if (block->free_next != NULL) {
    block->free_next->free_prev = block->free_prev;
  }
  block->free_next = NULL;
  block->free_prev = NULL;
  block->isFreed = 0;
}
void split(block *parent, size_t size) {
  if (parent == NULL) {
    printf("parent is NULL\n");
  }
  block *child = (void *)parent + size + sizeof(block);

  child->prev = parent;
  child->next = parent->next;
  if (parent->next != NULL) {
    parent->next->prev = child;
  }
  parent->next = child;
  child->size = parent->size - sizeof(block) - size;
  parent->size = size;
  child->free_next = NULL;
  child->free_prev = NULL;

  child->isFreed = 1;
  freeAdd(child);
  parent->isFreed = 0;
  freeDelete(parent);

  if (child->next == NULL) {
    tail = child;
  }
}
void merge(block *fronter) {

  block *tailer = fronter->next;

  fronter->size += sizeof(block) + tailer->size;
  freeDelete(tailer);
  if (tailer->next != NULL) {
    tailer->next->prev = fronter;
  } else {
    tail = fronter;
  }
  fronter->next = tailer->next;

  tailer->next = NULL;
  tailer->prev = NULL;
}
void general_free(void *ptr) {
  block *b = ptr - sizeof(block);
  if (ptr == NULL) {
    return;
  }
  if (!b->prev || !b->size || b->isFreed) {
    printf("free failed to get it\n");
  }

  if (b->isFreed) {
    return;
  }
  b->isFreed = 1;
  freeAdd(b);
  if (b->next && b->next->isFreed) {
    merge(b);
  }
  if (b->prev && b->prev->isFreed) {
    merge(b->prev);
  }
}

void ff_free(void *ptr) { general_free(ptr); }
void bf_free(void *ptr) { general_free(ptr); }
block *ff_traverse(size_t size) {
  block *currBlock = head;
  while (currBlock != NULL) {
    if (currBlock != head && currBlock->size >= size &&
        currBlock->size <= size + sizeof(block) + 2) {
      freeDelete(currBlock);
      return currBlock + 1;
    }
    if (currBlock->size > size + sizeof(block) + 2) {
      split(currBlock, size);
      return currBlock + 1;
    }
    currBlock = currBlock->free_next;
  }
  return currBlock;
}

block *bf_traverse(size_t size) {
  block *currBlock = head, *bestBlock = &dummy;
  unsigned long difference = INT_MAX;
  while (currBlock != NULL) {
    if (currBlock->size == size) {
      bestBlock = currBlock;
      difference = 0;
      break;
    }
    if (currBlock->size > size) {
      unsigned long currDiffer = currBlock->size - size;
      if (currDiffer < difference) {
        difference = currDiffer;
        bestBlock = currBlock;
      }
    }
    currBlock = currBlock->free_next;
  }
  if (difference != INT_MAX) {
    if (bestBlock->size > size + sizeof(block) + 2) {
      split(bestBlock, size);
      return bestBlock + 1;
    }
    freeDelete(bestBlock);
    return bestBlock + 1;
  }
  return currBlock;
}
block *allocate(size_t size) {
  block *newBlock = sbrk(sizeof(block) + size);

  newBlock->isFreed = 0;
  newBlock->size = size;
  newBlock->free_next = NULL;
  newBlock->free_prev = NULL;
  newBlock->prev = tail;
  tail->next = newBlock;

  newBlock->next = NULL;

  tail = newBlock;

  return newBlock + 1;
}
void *ff_malloc(size_t size) {
  block *allocBlock = ff_traverse(size);
  if (allocBlock == NULL) {
    return allocate(size);
  }
  return allocBlock;
}
void *bf_malloc(size_t size) {
  block *allocBlock = bf_traverse(size);
  if (allocBlock == NULL) {
    return allocate(size);
  }
  return allocBlock;
}
