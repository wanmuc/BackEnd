#include "print.h"

#include <stdio.h>

void myPrint(int* data, int len) {
  int i = 0;
  for (i = 0; i < len; ++i) {
    printf("%d ", data[i]);
  }
  printf("\n");
}