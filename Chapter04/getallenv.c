#include <stdio.h>

int main(int argc, char* argv[]) {
  int i = 0;
  // i从argc+1开始，跳过输入参数的结束标记串，直到环境变量的结束标记串
  for (i = argc + 1; argv[i] != '\0'; ++i) {
    printf("%s\n", argv[i]);
  }
  return 0;
}