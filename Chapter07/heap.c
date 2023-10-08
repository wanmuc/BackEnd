#include <malloc.h>
#include <stdio.h>
#include <unistd.h>

int main() {
  void* a = sbrk(10);  //调整堆顶指针brk
  void* b = sbrk(20);
  printf("heap alloc direction[%s]\n", b > a ? "Up" : "Down");
  return 0;
}
