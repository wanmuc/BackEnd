#include <stdint.h>

#include <iostream>

bool IsSmall() {
  union Check {
    uint8_t bytes[4];
    uint32_t data;
  };
  Check check;
  for (int i = 0; i < 4; i++) {
    check.bytes[i] = i;
  }
  std::cout << "data = " << check.data << std::endl;
  uint8_t* data = (uint8_t*)&check.data;
  return data[0] == 0 && data[1] == 1 && data[2] == 2 && data[3] == 3;
}

int main() {
  if (IsSmall()) {
    std::cout << "small" << std::endl;
  } else {
    std::cout << "big" << std::endl;
  }
  return 0;
}