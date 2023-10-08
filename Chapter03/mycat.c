#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

int catFile(char* fileName) {
  //以只读的方式打开文件
  int fd = open(fileName, O_RDONLY);
  //返回值小于0，说明文件打开失败
  if (fd < 0) {
    return -1;
  }
  char c;
  int ret = 0;
  while (1) {
    //从文件中读取一个字符
    ret = read(fd, &c, 1);
    //成功读取到一个字符
    if (1 == ret) {
      printf("%c", c);
      continue;
    }
    //返回0，表示文件读完，函数直接返回0
    if (0 == ret) {
      return 0;
    }
    //返回的既不是0也不是1，表示发生的错误直接退出while循环
    break;
  }
  //执行到这里说发生了错误，直接返回-1
  return -1;
}

int main(int argc, char** argv) {
  //只有一个参数时从标准输入读取数据
  if (argc <= 1) {
    char c;
    //只要读取到一个字符就输出到终端
    while (scanf("%c", &c) != EOF) {
      printf("%c", c);
    }
    return 0;
  }
  //传入main函数的参数列表中，第一个参数是程序名
  //故实际参数个数需要减1
  argc--;
  //参数多于5个时候报错并退出
  if (argc > 5) {
    fprintf(stderr, "Argc=%d,Too Many Arguments\n", argc);
    return -1;
  }
  int ret = 0;
  int i = 0;
  //按参数顺序读取文件内容并打印到终端标准输出
  for (i = 1; i <= argc; ++i) {
    ret = catFile(argv[i]);
    if (ret != 0) {
      //发生错误后系统会把错误码设置在errno这个全局变量中
      // strerror函数用于输出errno码对应的错误信息
      fprintf(stderr, "%s\n", strerror(errno));
      return -1;
    }
  }
  return 0;
}