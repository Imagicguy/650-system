#include "my_malloc.h"
#include <stdlib.h>

int main() { // 13k
  char *string1 = (char *)ff_malloc(8);

  ff_free(string1);

  char *string2 = (char *)ff_malloc(3);
  char *string3 = (char *)ff_malloc(10);

  // char *string11 = (char *)bf_malloc(3);
  // char *string21 = (char *)bf_malloc(5);
  // char *string31 = (char *)bf_malloc(5);

  // bf_free(string11);
  // bf_free(string21);
  // bf_free(string31);

  // char *string41 = (char *)bf_malloc(5);
  // bf_free(string41);
  return 0;
}
