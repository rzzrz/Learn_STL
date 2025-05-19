#include "../include/my_vector.h"
#include <iostream>
class test_class {
public:
  test_class() { std::cout << "construct function called" << std::endl; }
  ~test_class() { std::cout << "deconstruct function called" << std::endl; }
  test_class(const test_class &other) {
    std::cout << "copy construct function called" << std::endl;
  }
  test_class &operator=(const test_class &other) {
    std::cout << "copy operator= function called" << std::endl;
    return *this;
  }
  test_class &operator=(const test_class &&other) {
    std::cout << "move operator= function called" << std::endl;
    return *this;
  }
};
int main() {
  // 测试平凡数据类型
  m_vector<int> vec(10, 1);
  for (int i = 0; i < 10; i++)
    std::cout << vec[i];
  std::cout<<std::endl;

  vec.insert(vec.begin(), 1);
  std::cout<<vec.size();
  std::cout << "--------------------------------" << std::endl;

  m_vector<test_class> vec_class(1, std::move(test_class()));
  std::cout<<"构造完成了"<<std::endl;
  vec_class.insert(vec_class.begin(), std::move(test_class()));

          std::cout
      << "--------------------------------" << std::endl;

  m_vector<int> is_pod_type;
  is_pod_type.push_back(1);
  std::cout<< is_pod_type[0]<<std::endl;

  return 0;
}