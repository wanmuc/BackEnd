#include <stdio.h>

int main() {
#ifdef TEST
  printf("TEST is set\n");
#else
  printf("TEST not set\n");
#endif
  return 0;
}
