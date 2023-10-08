#include <malloc.h>
#include <stdint.h>
#include <string.h>

#include <iostream>

int main() {
  void *p = malloc(5);                //申请5个字节大小的堆内存
  memset(p, 66, 5);                   //把每个字节内容设置成66
  char *pC = (char *)p;               //声明一个char指针pC指向分配的内存的起始地址
  uint32_t *pUint32 = (uint32_t *)p;  //声明一个int32_t指针pUint32指向分配的内存的起始地址
  if (*pC == 66) {                    //从起始地址开始取1个字节的内存去解析成char变量
    std::cout << "char yes" << std::endl;
  }
  if (*pUint32 == 66 + (66 << 8) + (66 << 16) + (66 << 24)) {  //从起始地址开始取4个字节的内存去解析成uint32_t变量
    std::cout << "int32 yes" << std::endl;
  }
  return 0;
}