#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>

#include <iostream>

struct Test {
  uint8_t a;
  uint8_t b;
  uint32_t c;
  bool operator==(const Test &right) { return this->a == right.a && this->b == right.b && this->c == right.c; }
};
//序列化
void struct_to_array(Test *test, uint8_t *data) {
  *data = test->a;                     // 从data指向地址处写入一个字节到网络字节流中
  data++;                              // data指针向前移动一个字节
  *data = test->b;                     // 从data指向地址处写入一个字节到网络字节流中
  data++;                              // data指针向前移动一个字节
  *(uint32_t *)data = htonl(test->c);  // 从data指向地址处写入4个字节，即网络字节序的uint32_t
}
//反序列化
void array_to_struct(uint8_t *data, Test *test) {
  test->a = *data;  //从data指向地址处读取一个字节写入a中
  data++;           // data指针向前移动一个字节
  test->b = *data;  //从data指向地址处读取一个字节写入b中
  data++;           // data指针向前移动一个字节
  // 将data指向的网络字节流后的4个字节的网络字节序的uint32_t转化成本地字节序的uint32_t
  test->c = ntohl(*(uint32_t *)data);
}

int main() {
  Test test;
  Test testTwo;
  uint8_t data[6];
  test.a = 10;
  test.b = 11;
  test.c = 12;
  struct_to_array(&test, data);
  array_to_struct(data, &testTwo);
  if (test == testTwo) {
    std::cout << "equal" << std::endl;
  }
  return 0;
}