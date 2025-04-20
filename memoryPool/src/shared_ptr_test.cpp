#include "memoryPool.h"
#include <memory>
#include <iostream>
#include <string>
#include <vector>

class MyClass {
public:
  MyClass(const std::string s):str(s) { std::cout << "调用构造函数" << std::endl; }
  ~MyClass() { std::cout << "调用析构函数" << std::endl; }

  std::string str;
};

int main() {
  // 初始化内存池
  my_malloc_allocator<1> allocator;


  std::shared_ptr<int> pnum = allocator.make_shared_with_pool<int>();
  *pnum = 2025;
  std::cout << *pnum<<std::endl;

  // 无参调用就不会调用构造函数，且也不会调用析构函数
  std::shared_ptr<MyClass> pclass1 = allocator.make_shared_with_pool<MyClass>();
  // 有参构造调用构造和析构
  std::shared_ptr<MyClass> pclass2 = allocator.make_shared_with_pool<MyClass>("hello world");
  std::cout << pclass2->str << std::endl;

  std::shared_ptr<std::vector<int>> vec =
      allocator.make_shared_with_pool<std::vector<int>>(10, 1);
  for (int i = 0; i < 10; i++)
    std::cout << (*vec)[i] << std::endl;

  // 数组类型的申请
  std::shared_ptr<int> parr = allocator.make_shared_with_pool<int,10>();
  for (int i = 0; i < 10; i++) {
    parr.get()[i] = i + 1;
    std::cout<<"parr["<<i<<"] = "<<parr.get()[i]<<std::endl;
  } 

}