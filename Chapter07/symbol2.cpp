#include <iostream>
using namespace std;

class A {
 public:
  virtual void print() {}
};

class B : public A {
 public:
  void print() {}
};

int main() {
  B b;
  b.print();
  return 0;
}