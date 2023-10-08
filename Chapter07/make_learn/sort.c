#include "sort.h"

#include <stdlib.h>

int myCmp(const void *lvalue, const void *rvalue) {
  int lIntValue = *(int *)lvalue;
  int rIntValue = *(int *)rvalue;
  return lIntValue - rIntValue;
}

void mySort(int *data, int len) { qsort((void *)data, len, sizeof(int), myCmp); }