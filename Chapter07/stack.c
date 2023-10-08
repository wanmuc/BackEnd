#include <malloc.h>
#include <stdio.h>

void fun2(int* pb) {
  int a;
  printf("stack alloc direction[%s]\n", &a > pb ? "Up" : "Down");
}

void fun1() {
  int b;
  fun2(&b);
}

int main() {
  fun1();
  return 0;
}
