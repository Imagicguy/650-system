#include "my_malloc.h"
#include <stdlib.h>

int main() { // 13k
  char *string1 = (char *)ff_malloc(8);
  ff_free(string1);
  char *string4 = (char *)ff_malloc(5);

  ff_free(string4);
  char *string2 = (char *)ff_malloc(3);
  ff_free(string2);
  char *string3 = (char *)ff_malloc(10);
  ff_free(string3);

  char *string33 = (char *)ff_malloc(2);
  ff_free(string33);
  printf("start bf now\n");

  char *string11 = (char *)bf_malloc(3);
  printf("1\n");
  char *string21 = (char *)bf_malloc(5);
  printf("2\n");
  bf_free(string21);
  printf("3\n");
  char *string31 = (char *)bf_malloc(5);

  bf_free(string31);

  char *string41 = (char *)bf_malloc(5);
  bf_free(string11);
  bf_free(string41);
  return 0;
}
