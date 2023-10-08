#include <stdio.h>

int getSum(int n) {
  int i = 0;
  int sum = 0;
  for (i = 1; i <= n; ++i) {
    sum += i;
  }
  return sum;
}

int main(int argc, char** argv) {
  int n = 10;
  if (argc >= 2) {
    n = atoi(argv[1]);
  }
  int sum = getSum(n);
  printf("sum = %d\n", sum);
  return 0;
}