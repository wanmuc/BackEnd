#include <stdint.h>

#include <iostream>

bool IsSmall() {
  union Check {
    uint8_t bytes[4];
    uint32_t data;
  };
  Check check;
  check.data = 1;
  return check.bytes[0] == 1;
}

int main() {
  if (IsSmall()) {
    std::cout << "small" << std::endl;
  } else {
    std::cout << "big" << std::endl;
  }
  return 0;
}