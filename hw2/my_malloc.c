#include "my_malloc.h"
#include <limits.h>
#include <pthread.h>

#include <unistd.h>
static __thread block dummy_local = {0, 0, NULL, NULL, NULL, NULL};
static __thread block *head_local = NULL;
static __thread block *tail_local = NULL;
block dummy = {0, 0, NULL, NULL, NULL, NULL};
block *head = &dummy;
block *tail = &dummy;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

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
void split_lock(block *parent, size_t size) {
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

void split_nolock(block *parent, size_t size) {
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
    tail_local = child;
  }
}

void merge_nolock(block *fronter) {
  block *tailer = fronter->next;
  fronter->size += sizeof(block) + tailer->size;
  freeDelete(tailer);
  if (tailer->next != NULL) {
    tailer->next->prev = fronter;
  } else {
    tail_local = fronter;
  }
  fronter->next = tailer->next;

  tailer->next = NULL;
  tailer->prev = NULL;
}
void merge_lock(block *fronter) {

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
void ts_free_lock(void *ptr) {
  pthread_mutex_lock(&lock);
  block *b = ptr - sizeof(block);
  if (ptr == NULL) {
    pthread_mutex_unlock(&lock);
    return;
  }
  if (!b->prev || !b->size || b->isFreed) {
    printf("free failed to get it\n");
  }

  if (b->isFreed) {
    pthread_mutex_unlock(&lock);
    return;
  }
  b->isFreed = 1;
  freeAdd(b);
  if (b->next && b->next->isFreed) {
    merge_lock(b);
  }
  if (b->prev && b->prev->isFreed) {
    merge_lock(b->prev);
  }
  pthread_mutex_unlock(&lock);
}

void ts_free_nolock(void *ptr) {
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
    if (b->next == (void *)b + sizeof(block) + b->size) {
      merge_nolock(b);
    }
  }
  if (b->prev && b->prev->isFreed) {
    if (b == (void *)b->prev + sizeof(block) + b->prev->size) {
      merge_nolock(b->prev);
    }
  }
}

block *traverse_nolock(size_t size) {
  block *currBlock = head_local, *bestBlock = &dummy_local;
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
      split_nolock(bestBlock, size);
      return bestBlock + 1;
    }
    freeDelete(bestBlock);
    return bestBlock + 1;
  }
  return currBlock;
}

block *traverse_lock(size_t size) {
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
      split_lock(bestBlock, size);
      return bestBlock + 1;
    }
    freeDelete(bestBlock);
    return bestBlock + 1;
  }
  return currBlock;
}

block *allocate_lock(size_t size) {
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

block *allocate_nolock(size_t size) {
  pthread_mutex_lock(&lock);
  block *newBlock = sbrk(sizeof(block) + size);
  pthread_mutex_unlock(&lock);
  if (tail_local == NULL) {
    tail_local = &dummy_local;
  }
  newBlock->isFreed = 0;
  newBlock->size = size;
  newBlock->free_next = NULL;
  newBlock->free_prev = NULL;
  newBlock->prev = tail_local;
  tail_local->next = newBlock;

  newBlock->next = NULL;

  tail_local = newBlock;

  return newBlock + 1;
}

void *ts_malloc_nolock(size_t size) {
  head_local = &dummy_local;
  block *allocBlock = traverse_nolock(size);
  if (allocBlock == NULL) {
    allocBlock = allocate_nolock(size);
  }

  return allocBlock;
}

void *ts_malloc_lock(size_t size) {
  pthread_mutex_lock(&lock);
  block *allocBlock = traverse_lock(size);
  if (allocBlock == NULL) {
    allocBlock = allocate_lock(size);
  }
  pthread_mutex_unlock(&lock);
  return allocBlock;
}
