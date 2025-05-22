#include "../include/my_list.h" // 你的头文件路径
#include <iostream>

using namespace m_stl;

void test_basic_operations() {
  std::cout << "===== Testing Basic Operations =====\n";
  list<int> lst;

  // 测试空链表
  std::cout << "Empty list: " << (lst.empty() ? "Passed" : "Failed") << "\n";
  std::cout << "Size: " << lst.size() << " (Expected 0)\n";

  // 添加元素
  lst.push_back(10);
  lst.push_back(20);
  lst.push_front(5);
  std::cout << "Size after push: " << lst.size() << " (Expected 3)\n";
  std::cout << "Front: " << lst.front() << " (Expected 5)\n";
  std::cout << "Back: " << lst.back() << " (Expected 20)\n";

  // 遍历测试
  std::cout << "Elements: ";
  for (auto it = lst.begin(); it != lst.end(); ++it) {
    std::cout << *it << " ";
  }
  std::cout << "(Expected: 5 10 20)\n";

  // 删除元素
  lst.pop_back();
  lst.pop_front();
  std::cout << "Size after pop: " << lst.size() << " (Expected 1)\n";
  std::cout << "Front after pop: " << lst.front() << " (Expected 10)\n";

  // 清空链表
  lst.clear();
  std::cout << "After clear: " << (lst.empty() ? "Passed" : "Failed") << "\n\n";
}

void test_copy_and_move() {
  std::cout << "===== Testing Copy/Move Semantics =====\n";
  list<int> orig;
  orig.push_back(1);
  orig.push_back(2);
  orig.push_back(3);

  // 拷贝构造
  list<int> copy(orig);
  std::cout << "Copy size: " << copy.size() << " (Expected 3)\n";
  std::cout << "Copy elements: ";
  for (auto &x : copy)
    std::cout << x << " ";
  std::cout << "(Expected: 1 2 3)\n";

  // 移动构造
  list<int> moved(std::move(orig));
  std::cout << "Moved size: " << moved.size() << " (Expected 3)\n";
  std::cout << "Original size after move: " << orig.size()
            << " (Expected 0)\n\n";
}

void test_iterator_erase() {
  std::cout << "===== Testing Iterator and Erase =====\n";
  list<int> lst{1, 2, 3, 4, 5};

  // 删除中间元素
  auto it = lst.begin();
  ++it; // 指向 2
  ++it; // 指向 3
  lst.erase(it);
  std::cout << "After erase: ";
  for (auto &x : lst)
    std::cout << x << " ";
  std::cout << "(Expected: 1 2 4 5)\n";

  // 删除范围
  lst.erase(lst.begin(), lst.begin() + 2);
  std::cout << "After range erase: ";
  for (auto &x : lst)
    std::cout << x << " ";
  std::cout << "(Expected: 4 5)\n\n";
}

void test_resize() {
  std::cout << "===== Testing Resize =====\n";
  list<int> lst;
  lst.resize(3, 100);
  std::cout << "After resize up: ";
  for (auto &x : lst)
    std::cout << x << " ";
  std::cout << "(Expected: 100 100 100)\n";

  lst.resize(1);
  std::cout << "After resize down: ";
  for (auto &x : lst)
    std::cout << x << " ";
  std::cout << "(Expected: 100)\n\n";
}

int main() {
  test_basic_operations();
  test_copy_and_move();
  test_iterator_erase();
  test_resize();
  return 0;
}
