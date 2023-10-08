#include <stdio.h>

#include "swap.h"
int main() {
  int a = 10;
  int b = 100;
  swap(&a, &b);
  printf("a=%d,b=%d\n", a, b);
  return 0;
}