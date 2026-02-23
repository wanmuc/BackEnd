#include <arpa/inet.h>
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
  uint32_t hostValue = 666888;
  bool hostValueIsSmall = false;
  hostValueIsSmall = IsSmall();
  if (hostValueIsSmall) {
    std::cout << "host byte order is small" << std::endl;
  } else {
    std::cout << "host byte order is big" << std::endl;
  }
  uint32_t netValue = htonl(hostValue);
  if (netValue != hostValue) {
    if (hostValueIsSmall) {
      std::cout << "net byte order is big" << std::endl;
    } else {
      std::cout << "net byte order is small" << std::endl;
    }
  } else {
    if (hostValueIsSmall) {
      std::cout << "net byte order is small" << std::endl;
    } else {
      std::cout << "net byte order is big" << std::endl;
    }
  }
  return 0;
}