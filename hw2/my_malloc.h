#ifndef _MY_MALLOC_H_
#define _MY_MALLOC_H_
#include <stdio.h>

#include <unistd.h>

void *ts_malloc_lock(size_t size);
void ts_free_lock(void *ptr);
void *ts_malloc_nolock(size_t size);
void ts_free_nolock(void *ptr);
struct block_t {
  int isFreed;
  size_t size;
  struct block_t *next;
  struct block_t *prev;
  struct block_t *free_next;
  struct block_t *free_prev;
};
typedef struct block_t block;

#endif
