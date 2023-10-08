#include <unistd.h>

int main() {
  //第一个参数0表示工作目录切换到"/"
  //第二个参数0表示重定向标准输入、标准输出、标准错误到"/dev/null"
  if (0 == daemon(0, 0)) {
    while (1) {
      sleep(1);
    }
  }
  return 0;
}