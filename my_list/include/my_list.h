#ifndef _MY_LIST_H_
#define _MY_LIST_H_

#include "./memoryPool.h"
#include "./iterator_type.h"
#include <cstddef>
#include <type_traits>
#include <utility>

namespace m_stl {
template <typename T, typename Default_allocator = my_malloc_allocator<0>>
class list {

public:
  // 链表的节点结构体和迭代器
  struct list_node {
    using list_type = void *;
    using value_type = T;

    list_type next;
    list_type prev;
    value_type value;

    list_node() : next(0), prev(0) {}
    list_node(list_type next, list_type prev) {}
    list_node(list_type next, list_type prev, const value_type& value) {}
    list_node(list_type next, list_type prev, value_type &&value) {
      this->value = std::move(value);
    }
  };

  // 迭代器
  class iterator {
  public:
    // 双向迭代器
    using bidirectional_iterator_tag = bidirectional_iterator_tag;
    using value_type = T;
    using reference = value_type&;
    using const_reference = const value_type &;
    using self = iterator;

  public:
    // 普通构造函数
    iterator(list_node node) { now_node = node; }
    // 拷贝构造函数
    iterator(self &other) { now_node = other.now_node; }
    // 移动构造
    iterator(self &&other) { now_node = other.now_node; }

    // 重载运算符
    self &operator++() {
      now_node = *static_cast<list_node>(now_node.next);
      return *this;
    }
    self operator++(int) {
      self tmp = *this;
      now_node = *static_cast<list_node>(now_node.next);
      return tmp;
    }

    self &operator--() {
      now_node = *static_cast<list_node>(now_node.prev);
      return *this;
    }
    self operator--(int) {
      self tmp = *this;
      now_node = *static_cast<list_node>(now_node.prev);
      return tmp;
    }

    value_type &operator*() { return now_node.value; }
    value_type *operator->() { return &now_node.value; }
    

    self &operator=(const self& other) {
      now_node = other.now_node;
      return *this;
    }
    self &operator=(const self &&other) {
      now_node = other.now_node;
      return *this;
    }

  private:
    list_node now_node;
  };

  class reverse_iterator {
    
  };
public:
  // 别名定义
  using value_type = T;
  using iterator = iterator;
  using list_head = list_node;
public:
  // 构造函数：

  list() : head() ,allocator(),size(0){
    head.next = &head;
    head.prev = &head;
  }
  list(std::size_t n,const value_type& value) : head(),allocator(),size(0) {
    allocate_and_fill_value(head,n,value);
  }
  // 拷贝构造
  list(const list& other);
  // 移动拷贝构造
  list(list &&other);

  // 拷贝赋值
  list &operator=(const list &other);
  // 移动拷贝赋值
  list &operator=(list &&other);

  // 其他功能函数
  size_t size();
  void push_back();
  void push_front();
  void emplace_back();
  void emplace_front();

  iterator begin();
  const iterator cbegin();
  iterator end();
  const iterator cend();

  reverse_iterator rbegin();
  const reverse_iterator crbegin();
  reverse_iterator rend();
  const reverse_iterator crend();

  

protected:
  // 申请n个空间并且使用value初始化他们
  void allocate_and_fill_value(list_head head,size_t n,const value_type& value);
  list_node* construct(const value_type& value);
private:
  list_head head;
  Default_allocator allocator;
  int size;
};
template <typename T, typename Default_allocator>
void list<T, Default_allocator>::allocate_and_fill_value(
    list_head head, size_t n, const value_type &value) {
  if constexpr (std::is_trivially_constructible_v<value_type>()) {
    for (size_t i = 0; i < n; i++) {
      // 先构造node之后链接
      list_node *new_node = construct(value);
      // 开始插入
      (static_cast<list_node *>(head.prev))->next = new_node;
      new_node->prev = head.prev;

      new_node->next = &head;
      head.prev = new_node;
      //< A <-> B <-> C <-> head >
    }
  } else {
    // 先构造node之后链接
    list_node *new_node = static_cast<list_node *>(
        Default_allocator::allocate(sizeof(list_node)));
    new_node->value = value;
    // 开始插入
    (static_cast<list_node *>(head.prev))->next = new_node;
    new_node->prev = head.prev;

    new_node->next = &head;
    head.prev = new_node;
    //< A <-> B <-> C <-> head >
  }
  
}
template <typename T, typename Default_allocator>
typename list<T, Default_allocator>::list_node *
list<T, Default_allocator>::construct(const value_type &value) {
  list_node *new_node =
      static_cast<list_node *>(Default_allocator::allocate(sizeof(list_node)));
  new (&(new_node->value)) value_type(value);
  return new_node;
}
 
}; // namespace m_stl

#endif // _MY_LIST_H_