#include "print.h"
#include "sort.h"

int main() {
  int data[8] = {10, 20, 25, 2, 234, 13, 3, 1};
  mySort(data, sizeof(data) / sizeof(int));
  myPrint(data, sizeof(data) / sizeof(int));
  return 0;
}