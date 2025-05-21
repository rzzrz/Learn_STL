#include "my_list.h"
#include <cassert>
#include <iostream>

// 打印当前测试用例进度
#define TEST_HEADER()                                                          \
  (std::cout << "=== Running Test Case: " << __func__ << " ===" << std::endl)

// 打印失败信息
#define TEST_FAILED()                                                          \
  (std::cout << "❌ Assertion Failed in " << __func__ << " at line "           \
             << __LINE__ << std::endl)

// 打印成功信息
#define TEST_PASSED()                                                          \
  (std::cout << "✅ Test Passed: " << __func__ << std::endl << std::endl)

// 测试基础构造函数和 push_back/push_front
void test_constructor_and_push() {
  TEST_HEADER();
  {
    m_stl::list<int> lst;
    assert(lst.size() == 0);
    assert(lst.empty());

    lst.push_back(1);
    assert(lst.size() == 1);
    assert(lst.front() == 1 && lst.back() == 1);

    lst.push_back(2);
    assert(lst.size() == 2);
    assert(lst.front() == 1 && lst.back() == 2);

    lst.push_front(0);
    assert(lst.size() == 3);
    assert(lst.front() == 0 && lst.back() == 2);
  }

  {
    m_stl::list<int> lst(3, 5);
    assert(lst.size() == 3);
    assert(lst.front() == 5 && lst.back() == 5);

    auto it = lst.begin();
    assert(*it++ == 5 && *it++ == 5 && *it++ == 5);
  }

  TEST_PASSED();
}

// 测试拷贝构造和拷贝赋值
void test_copy_operations() {
  TEST_HEADER();
  {
    m_stl::list<int> lst1(3, 10);
    m_stl::list<int> lst2 = lst1; // 拷贝构造

    assert(lst2.size() == 3);
    for (auto it = lst2.begin(); it != lst2.end(); ++it) {
      assert(*it == 10);
    }

    lst1.push_back(20); // 修改原始列表
    assert(lst1.back() == 20);
    assert(lst2.back() == 10); // 确保拷贝独立
  }

  {
    m_stl::list<std::string> lst1;
    lst1.push_back("hello");
    lst1.push_back("world");

    m_stl::list<std::string> lst2;
    lst2 = lst1; // 拷贝赋值

    assert(lst2.size() == 2);
    assert(*lst2.begin() == "hello" && *(++lst2.begin()) == "world");

    lst1.push_back("!");
    assert(lst2.size() == 2); // 确保赋值后独立
  }

  TEST_PASSED();
}

// 测试移动构造和移动赋值
void test_move_operations() {
  TEST_HEADER();
  {
    m_stl::list<int> lst1;
    lst1.push_back(100);
    lst1.push_back(200);

    m_stl::list<int> lst2 = std::move(lst1); // 移动构造

    assert(lst2.size() == 2);
    assert(lst2.front() == 100 && lst2.back() == 200);
    assert(lst1.empty()); // 原列表应为空
  }

  {
    m_stl::list<std::string> lst1;
    lst1.push_back("move");

    m_stl::list<std::string> lst2;
    lst2 = std::move(lst1); // 移动赋值

    assert(lst2.size() == 1);
    assert(lst2.front() == "move");
    assert(lst1.empty());
  }

  TEST_PASSED();
}

// 测试 emplace 操作
void test_emplace() {
  TEST_HEADER();
  {
    m_stl::list<std::pair<int, std::string>> lst;
    lst.emplace_back(42, "life");
    lst.emplace_front(7, "secrets");

    assert(lst.size() == 2);
    assert(lst.front().first == 7);
    assert(lst.back().first == 42 && lst.back().second == "life");
  }

  TEST_PASSED();
}

// 测试 pop_back/pop_front/clear
void test_erase_clear() {
  TEST_HEADER();
  {
    m_stl::list<int> lst;
    lst.push_back(1);
    lst.push_back(2);
    lst.push_back(3);

    lst.pop_back();
    assert(lst.size() == 2);
    assert(lst.back() == 2);

    lst.pop_front();
    assert(lst.size() == 1);
    assert(lst.front() == 2);

    lst.push_back(3);
    lst.push_back(4);
    lst.clear();
    assert(lst.empty());
  }

  TEST_PASSED();
}

// 测试 erase 和 resize
void test_resize_erase() {
  TEST_HEADER();
  {
    m_stl::list<int> lst;
    lst.push_back(1);
    lst.push_back(2);
    lst.push_back(3);
    lst.push_back(4);

    // 测试 erase 单个元素
    auto it = lst.erase(lst.begin());
    assert(*it == 2 && lst.size() == 3 && lst.front() == 2);

    // 测试 erase 范围
    lst.erase(lst.begin(), lst.end());
    assert(lst.empty());
  }

  {
    m_stl::list<int> lst(5, 1); // 5个1
    lst.resize(3);
    assert(lst.size() == 3);
    for (auto x : lst)
      assert(x == 1);

    lst.resize(5, 0);
    assert(lst.size() == 5);
    assert(lst.front() == 1 && lst.back() == 0);
  }

  TEST_PASSED();
}

// 测试迭代器遍历（包括 reverse）
void test_iterators() {
  TEST_HEADER();
  {
    m_stl::list<int> lst;
    lst.push_back(10);
    lst.push_back(20);
    lst.push_back(30);

    // 正向迭代器
    int arr[] = {10, 20, 30};
    int idx = 0;
    for (auto it = lst.begin(); it != lst.end(); ++it) {
      assert(*it == arr[idx++]);
    }

    // 反向迭代器
    idx = 2;
    for (auto it = lst.rbegin(); it != lst.rend(); ++it) {
      assert(*it == arr[idx--]);
    }
  }

  TEST_PASSED();
}

// 主测试程序
int main() {
  test_constructor_and_push();
  test_copy_operations();
  test_move_operations();
  test_emplace();
  test_erase_clear();
  test_resize_erase();
  test_iterators();

  std::cout << "All Tests Passed! 🎉" << std::endl;
  return 0;
}
