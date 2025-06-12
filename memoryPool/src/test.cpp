#include <iostream>
#include "../include/memoryPool.h"
int main() {
  my_malloc_allocator::initializer();
  int *p = (int*)my_malloc_allocator::allocate(sizeof(int));
  *p = 1;
  std::cout<<*p;
}