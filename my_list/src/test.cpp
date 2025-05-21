#include "../include/my_list.h"
#include <iostream>
int main() {
  m_stl::list<int> list;
  list.emplace_back(1);
  std::cout << *list.begin();
  list.pop_back();
}