#ifndef _MY_MALLOC_H_
#define _MY_MALLOC_H_
#include <stdio.h>

#include <unistd.h>

void *ff_malloc(size_t size);
void ff_free(void *ptr);
void *bf_malloc(size_t size);
void bf_free(void *ptr);
void printlist();
void printfreelist();
unsigned long get_data_segment_size();
unsigned long get_data_segment_free_space_size();
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
