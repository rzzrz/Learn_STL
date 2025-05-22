#ifndef _MY_LIST_H_
#define _MY_LIST_H_

#include <glog/logging.h>

#include "./iterator_type.h"
#include "./memoryPool.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <type_traits>
#include <utility>

namespace m_stl {
template <typename T, typename Default_allocator = my_malloc_allocator<0>>
class list {

public:
  // 链表的节点结构体和迭代器
  struct list_node {
    using value_type = T;
    using list_type = list_node *;

    list_type next;
    list_type prev;
    value_type value;

    list_node() : next(this), prev(this), value() {}
    list_node(list_type next, list_type prev)
        : next(next), prev(prev), value() {}
    list_node(list_type next, list_type prev, const value_type &value)
        : next(next), prev(prev), value(value) {}
    list_node(list_type next, list_type prev, value_type &&value)
        : next(next), prev(prev), value(std::move(value)) {
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

    friend class list;

  public:
    // 普通构造函数
    iterator(list_node *node) { now_node = node; }
    // 拷贝构造函数
    iterator(const self &other) { now_node = other.now_node; }
    // 移动构造
    iterator(self &&other) {
      now_node = other.now_node;
      other.now_node = nullptr;
    }

    // 重载运算符
    self &operator++() {
      now_node = now_node->next;
      return *this;
    }
    self operator++(int) {
      self tmp = *this;
      now_node = now_node->next;
      return tmp;
    }

    self &operator--() {
      now_node = now_node->prev;
      return *this; 
    }
    self operator--(int) {
      self tmp = *this;
      now_node = now_node->prev;
      return tmp;
    }

    value_type &operator*() { return now_node->value; }
    value_type *operator->() { return &now_node->value; }

    bool operator==(const iterator &other) const {
      return this->now_node == other.now_node;
    }
    bool operator!=(const iterator &other) const {
      return this->now_node != other.now_node;
    }

    self operator+(size_t n) {
      self tmp = *this;
      for (size_t i = 0; i < n; ++i)
        ++tmp;
      return tmp;
    }

    self &operator=(const self &other) {
      now_node = other.now_node;
      return *this;
    }
    self &operator=(const self &&other) {
      now_node = other.now_node;
      return *this;
    }

  private:
    list_node *now_node;
  };

  class reverse_iterator {
  public:
    // 双向迭代器
    using iterator_tag = bidirectional_iterator_tag;
    using value_type = T;
    using reference = value_type &;
    using const_reference = const value_type &;
    using self = reverse_iterator;

    friend class list;

  public:
    // 普通构造函数
    reverse_iterator(list_node *node) { now_node = node; }
    // 拷贝构造函数
    reverse_iterator(const self &other) { now_node = other.now_node; }
    // 移动构造
    reverse_iterator(self &&other) { now_node = other.now_node; }

    // 重载运算符
    self &operator++() {
      now_node = now_node->prev;
      return *this;
    }
    self operator++(int) {
      self tmp = *this;
      now_node = now_node->prev;
      return tmp;
    }

    self &operator--() {
      now_node = now_node->next;
      return *this;
    }
    self operator--(int) {
      self tmp = *this;
      now_node = now_node->next;
      return tmp;
    }

    value_type &operator*() { return now_node->value; }
    value_type *operator->() { return &now_node->value; }

    bool operator==(const self &other) const {
      return this->now_node == other.now_node;
    }
    bool operator!=(const self &other) const {
      return this->now_node != other.now_node;
    }

    self &operator+(size_t n) {
      self tmp = *this;
      for (size_t i = 0; i < n; ++i)
        ++tmp;
      return tmp;
    }

    self &operator=(const self &other) {
      now_node = other.now_node;
      return *this;
    }
    self &operator=(const self &&other) {
      now_node = other.now_node;
      return *this;
    }

  private:
    list_node *now_node;
  };

public:
  // 别名定义
  using value_type = T;
  using iterator = list::iterator;
  using reverse_iterator = list::reverse_iterator;
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
  list(list &other) : head(), allocator(), _size(0) {
    iterator other_it = other.begin();
    for (int i = 0; i < other._size; i++) {
      list_node *tmp = construct(*other_it);
      insert(tmp);
      other_it++;
      _size++;
    }
  }
  // 初始化列表构造函数
  list(std::initializer_list<T> init) : head(), allocator(), _size(0) {
    // 初始化哨兵节点
    head.next = &head;
    head.prev = &head;
    // 遍历初始化列表，逐个构造节点并插入
    for (const T &value : init) {
      list_node *new_node = construct(value);
      insert(new_node);
      _size++;
    }
  }
  // 移动拷贝构造
  list(list &&other) : head(), allocator(), _size(other._size) {
    if (other.head.next != &other.head) {
      head.next = other.head.next;
      head.prev = other.head.prev;
      head.next->prev = &head;
      head.prev->next = &head;
    }
    other.head.next = &other.head;
    other.head.prev = &other.head;
    other._size = 0;
  }

  // 拷贝赋值
  list &operator=(list &other) {
    if (this != &other) {
      clear();
      for (iterator it = other.begin(); it != other.end(); ++it) {
        list_node *new_node = construct(*it); // 通过值构造新节点
        insert(new_node);
        _size++;
      }
    }
    return *this;
  }
  // 移动拷贝赋值
  list &operator=(list &&other) {
    if (this != &other) {
      clear();
      head.next = other.head.next;
      head.prev = other.head.prev;

      if (other.head.next != &other.head) {
        head.next->prev = &head; // 更新节点指向
        head.prev->next = &head;
      }

      other.head.next = other.head.prev = &other.head; // 重置对方
      other._size = 0;
    }
    return *this;
  }

  // 其他功能函数
  size_t size() { return _size; }
  void push_back(const value_type &);
  void push_front(const value_type &);

  template <typename... Args> void emplace_back(Args &&...args);
  template <typename... Args> void emplace_front(Args &&...args);

  void pop_back();
  void pop_front();
  iterator erase(iterator pos);
  iterator erase(iterator first, iterator last);

  void clear();
  void resize(size_t n);
  void resize(size_t n, const value_type &);

  iterator begin() { return iterator(head.next); }
  const iterator cbegin() const { return iterator(head.next); }
  iterator end() { return iterator(&head); }
  const iterator cend() const { return iterator(&head); }

  reverse_iterator rbegin() { return reverse_iterator(head.prev); }
  const reverse_iterator crbegin() const { return reverse_iterator(head.prev); }
  reverse_iterator rend() { return reverse_iterator(&head); }
  const reverse_iterator crend() const { return reverse_iterator(&head); }

  value_type &front() {
    assert(!empty());
    return *begin();
  }
  const value_type &front() const {
    assert(!empty());
    return *begin();
  }
  value_type &back() {
    assert(!empty());
    return *(--end());
  }
  const value_type &back() const {
    assert(!empty());
    return *(--end());
  }

  bool empty() const { return head.next == &head; }

  // 析构函数
  ~list();

protected:
  // 申请n个空间并且使用value初始化他们
  void allocate_and_fill_value(list_head head, size_t n,
                               const value_type &value);
  list_node *construct(const value_type &value);
  template <typename... Args> list_node *construct_in_place(Args... args);
  void deconstruct(list_node *);
  void insert(list_node *);

private:
  list_head head;
  Default_allocator allocator;
  size_t _size;
};

template <typename T, typename Default_allocator>
void list<T, Default_allocator>::push_back(const value_type &tmp) {

  list_node *new_node = construct(tmp);
  try {
    insert(new_node);
    _size++;
  } catch (...) {
    deconstruct(new_node);
    throw;
  }
}

template <typename T, typename Default_allocator>
void list<T, Default_allocator>::push_front(const value_type &tmp) {
  list_node *new_node = construct(tmp);

  new_node->next = head.next;
  new_node->prev = &head;

  // 插入到哨兵node后面
  head.next->prev = new_node;
  head.next = new_node;

  this->_size++;
}

template <typename T, typename Default_allocator>
template <typename... Args>
void list<T, Default_allocator>::emplace_back(Args &&...args) {

  list_node *new_node = construct_in_place(std::forward<Args>(args)...);
  (*static_cast<list_node *>(head.prev)).next = new_node;
  new_node->prev = head.prev;

  new_node->next = &head;
  head.prev = new_node;
  this->_size++;
}

template <typename T, typename Default_allocator>
template <typename... Args>
void list<T, Default_allocator>::emplace_front(Args &&...args) {
  list_node *new_node = construct_in_place(std::forward<Args>(args)...);
  (*static_cast<list_node *>(head.next)).prev = new_node;
  new_node->next = head.next;

  new_node->prev = &head;
  head.next = new_node;
  this->_size++;
}
template <typename T, typename Default_allocator>
void list<T, Default_allocator>::pop_back() {
  // 先析构对应的数值
  list_node *tmp = head.prev;
  tmp->prev->next = &head;
  head.prev = tmp->prev;
  this->_size--;
  deconstruct(tmp);
}

template <typename T, typename Default_allocator>
void list<T, Default_allocator>::pop_front() {
  // 先析构对应的数值
  list_node *tmp = head.next;
  tmp->next->prev = &head;
  head.next = tmp->next;
  this->_size--;
  deconstruct(tmp);
}
template <typename T, typename Default_allocator>
typename list<T, Default_allocator>::iterator
list<T, Default_allocator>::erase(iterator pos) {
  list_node *tmp = pos.now_node;
  tmp->prev->next = tmp->next;
  tmp->next->prev = tmp->prev;
  iterator ret(tmp->next);
  deconstruct(tmp);
  this->_size--;
  return ret;
}
template <typename T, typename Default_allocator>
typename list<T, Default_allocator>::iterator
list<T, Default_allocator>::erase(iterator first, iterator last) {
  while (first != last) {
    first = erase(first);
  }
  return last;
}
template <typename T, typename Default_allocator>
void list<T, Default_allocator>::clear() {
  while (!empty()) {
    erase(begin());
  }
  this->_size = 0;
}
template <typename T, typename Default_allocator>
void list<T, Default_allocator>::resize(size_t n) {
  if (n < _size) {
    iterator it = begin();
    for (size_t i = 0; i < n; ++i)
      ++it;
    erase(it, end()); // 删除从第n个节点到末尾
  }
}

template <typename T, typename Default_allocator>
void list<T, Default_allocator>::resize(size_t n, const value_type &value) {
  if (n <= _size) {
    iterator last = begin();
    for (int i = 0; i < n; i++)
      last++;
    erase(begin(), last);
  } else {
    for (int i = 0; i < n - _size; i++) {
      list_node *tmp = construct(value);
      insert(tmp);
    }
  }
}

// 申请空间并且使用value填满
template <typename T, typename Default_allocator>
void list<T, Default_allocator>::allocate_and_fill_value(
    list_head head, size_t n, const value_type &value) {
  for (size_t i = 0; i < n; i++) {
    // 先构造node之后链接
    list_node *new_node = construct(value);
    // 开始插入
    insert(new_node);
    //< A <-> B <-> C <-> head >
  }
  _size += n;
}

// 构造一个新节点
template <typename T, typename Default_allocator>
typename list<T, Default_allocator>::list_node *
list<T, Default_allocator>::construct(const value_type &value) {
  list_node *new_node =
      static_cast<list_node *>(allocator.allocate(sizeof(list_node)));
  new (new_node) list_node(new_node,new_node,value);
  return new_node;
}

template <typename T, typename Default_allocator>
template <typename... Args>
typename list<T, Default_allocator>::list_node *
list<T, Default_allocator>::construct_in_place(Args... args) {
  list_node *new_node =
      static_cast<list_node *>(allocator.allocate(sizeof(list_node)));
  new (&new_node->value) value_type(std::forward<Args>(args)...);
  return new_node;
}
template <typename T, typename Default_allocator>
void list<T, Default_allocator>::deconstruct(list_node *node) {
  if constexpr (!std::is_trivially_destructible<value_type>()) {
    node->value.~value_type();
  }
  allocator.deallocate(static_cast<void *>(node), sizeof(list_node));
  return;
}
template <typename T, typename Default_allocator>
list<T, Default_allocator>::~list() {
  clear();
}

template <typename T, typename Default_allocator>
void list<T, Default_allocator>::insert(list_node *node) {
  head.prev->next = node;
  node->prev = head.prev;

  node->next = &head;
  head.prev = node;
}

}; // namespace m_stl

#endif // _MY_LIST_H_