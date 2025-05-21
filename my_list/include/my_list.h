#ifndef _MY_LIST_H_
#define _MY_LIST_H_

#include "./iterator_type.h"
#include "./memoryPool.h"
#include <algorithm>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace m_stl {
template <typename T, typename Default_allocator = my_malloc_allocator<0>>
class list {

public:
  // 链表的节点结构体和迭代器
  struct list_node {
    using value_type = T;
    using list_type = list_node*;
    

    list_type next;
    list_type prev;
    value_type value;

    list_node() : next(this), prev(this) {}
    list_node(list_type next, list_type prev) {}
    list_node(list_type next, list_type prev, const value_type &value) {}
    list_node(list_type next, list_type prev, value_type &&value) {
      this->value = std::move(value);
    }
  };

  // 迭代器
  class iterator {
  public:
    // 双向迭代器
    using iterator_tag = bidirectional_iterator_tag;
    using value_type = T;
    using reference = value_type &;
    using const_reference = const value_type &;
    using self = iterator;

  public:
    // 普通构造函数
    iterator(list_node* node) { now_node = node; }
    // 拷贝构造函数
    iterator(self &other) { now_node = other.now_node; }
    // 移动构造
    iterator(self &&other) { now_node = other.now_node; }

    // 重载运算符
    self &operator++() {
      now_node = *static_cast<list_node>(now_node->next);
      return *this;
    }
    self operator++(int) {
      self tmp = *this;
      now_node = *static_cast<list_node>(now_node->next);
      return tmp;
    }

    self &operator--() {
      now_node = *static_cast<list_node>(now_node->prev);
      return *this;
    }
    self operator--(int) {
      self tmp = *this;
      now_node = *static_cast<list_node>(now_node->prev);
      return tmp;
    }

    value_type &operator*() { return now_node->value; }
    value_type *operator->() { return &now_node->value; }

    self &operator=(const self &other) {
      now_node = other.now_node;
      return *this;
    }
    self &operator=(const self &&other) {
      now_node = other.now_node;
      return *this;
    }

  private:
    list_node* now_node;
  };

  class reverse_iterator {};

public:
  // 别名定义
  using value_type = T;
  using iterator = list::iterator;
  using list_head = list_node;

public:
  // 构造函数：

  list() : head(), allocator(), _size(0) {
    head.next = &head;
    head.prev = &head;
  }
  list(std::size_t n, const value_type &value) : head(), allocator(), _size(0) {
    allocate_and_fill_value(head, n, value);
  }
  // 拷贝构造
  list(const list &other);
  // 移动拷贝构造
  list(list &&other);

  // 拷贝赋值
  list &operator=(const list &other);
  // 移动拷贝赋值
  list &operator=(list &&other);

  // 其他功能函数
  size_t size() { return _size; }
  void push_back(const value_type &);
  void push_front(const value_type &);

  template <typename... Args> void emplace_back(Args... args);
  template <typename... Args> void emplace_front(Args... args);

  void pop_back();
  void pop_front();
  iterator erase(iterator pos);
  iterator erase(iterator first, iterator last);

  void clear();
  void resize();

  iterator begin(){return iterator(static_cast<list_node*>(head.next));}
  const iterator cbegin();
  iterator end();
  const iterator cend();

  reverse_iterator rbegin();
  const reverse_iterator crbegin();
  reverse_iterator rend();
  const reverse_iterator crend();

  value_type &front();
  const value_type front() const;
  value_type &back();
  const value_type back() const;

  bool empty() const;

  // 析构函数
  ~list() =default;

protected:
  // 申请n个空间并且使用value初始化他们
  void allocate_and_fill_value(list_head head, size_t n,
                               const value_type &value);
  list_node *construct(const value_type &value);
  template <typename... Args>
  list_node *construct_in_palce(Args... args);
  void deconstruct(list_node*);
private:
  list_head head;
  Default_allocator allocator;
  int _size;
};

template <typename T, typename Default_allocator>
void list<T, Default_allocator>::push_back(const value_type &tmp) {
  list_node *new_node = construct(tmp);
  // 注意哨兵node在最后那么我们要push_back的话就要使用头插
  (*static_cast<list_node *>(head.prev)).next = new_node;
  new_node->prev = head.prev;

  new_node->next = &head;
  head.prev = new_node;
}

template <typename T, typename Default_allocator>
void list<T, Default_allocator>::push_front(const value_type &tmp) {
  list_node *new_node = construct(tmp);

  // 插入到哨兵node后面
  (*static_cast<list_node *>(head.next)).prev = new_node;
  new_node->next = head.next;

  new_node->prev = &head;
  head.next = new_node;
}

template <typename T, typename Default_allocator>
template <typename... Args>
void list<T, Default_allocator>::emplace_back(Args... args) {

  list_node *new_node = construct_in_palce(std::forward<Args>(args)...);
  (*static_cast<list_node *>(head.prev)).next = new_node;
  new_node->prev = head.prev;

  new_node->next = &head;
  head.prev = new_node;
}

template <typename T, typename Default_allocator>
template <typename... Args>
void list<T, Default_allocator>::emplace_front(Args... args) {
  list_node *new_node = construct_in_palce(std::forward<Args>(args)...);
  (*static_cast<list_node *>(head.next)).prev = new_node;
  new_node->next = head.next;

  new_node->prev = &head;
  head.next = new_node;
}
template <typename T, typename Default_allocator>
void list<T, Default_allocator>::pop_back() {
  // 先析构对应的数值
  list_node *tmp = head.prev;
  tmp->prev->next = &head;
  head.prev = tmp->prev;
  deconstruct(tmp);
}

    // 申请空间并且使用value填满
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
  _size += n;
}

// 构造一个新节点
template <typename T, typename Default_allocator>
typename list<T, Default_allocator>::list_node *
list<T, Default_allocator>::construct(const value_type &value) {
  list_node *new_node =
      static_cast<list_node *>(Default_allocator::allocate(sizeof(list_node)));
  new (&(new_node->value)) value_type(value);
  return new_node;
}

template <typename T, typename Default_allocator>
template <typename... Args>
typename list<T, Default_allocator>::list_node *
list<T, Default_allocator>::construct_in_palce(Args... args) {
  list_node *new_node =
      static_cast<list_node *>(allocator.allocate(sizeof(value_type)));
  new (&new_node->value) value_type(std::forward<Args>(args)...);
  return new_node;
}
template <typename T, typename Default_allocator>
void
list<T, Default_allocator>::deconstruct(list_node *node) {
  if constexpr (std::is_trivially_destructible<value_type>()) {
    node->value.~value_type();
  }
  allocator.deallocate(static_cast<void *>(node), sizeof(value_type));
  return;
}

}; // namespace m_stl

#endif // _MY_LIST_H_