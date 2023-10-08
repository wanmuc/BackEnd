#include <arpa/inet.h>
#include <stdint.h>

#include <iostream>

bool IsSmall(uint32_t value) {
  union Check {
    uint8_t bytes[4];
    uint32_t data;
  };
  Check check;
  check.data = value;
  std::cout << "data = " << check.data << std::endl;
  uint8_t* data = (uint8_t*)&check.data;
  return data[0] == check.bytes[0] && data[1] == check.bytes[1] && data[2] == check.bytes[2] &&
         data[3] == check.bytes[3];
}

int main() {
  uint32_t hostValue = 666888;
  bool hostValueIsSmall = false;
  hostValueIsSmall = IsSmall(hostValue);
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