#ifndef _MY_VECTOR_H_
#define _MY_VECTOR_H_

#include "./memoryPool.h"
#include "./uninitial.h"
#include <csignal>
#include <cstddef>

template <typename T, typename Default_alloctor = my_malloc_allocator<0>>
class m_vector {
public:
  // 类型定义
  using value_type = T;
  using pointer = T *;
  using const_pointer = const T *;
  using reference = T &;
  using const_reference = const T &;
  using iterator = value_type *;
  using const_iterator = const value_type *;
  using difference_type = ptrdiff_t;

  // 申请空间并填充空间
  iterator allocate_and_fill(std::size_t n, const value_type& value);

  // 统一初始化函数
  void init(std::size_t n, const value_type& value) {
    start = allocate_and_fill(n, value);
    finish = start + n;
    end_of_storage = finish;
  }

  // 重载构造函数
  m_vector() : allocator(), start(0), finish(0), end_of_storage(0) {};
  explicit m_vector(std::size_t n) : allocator() { init(n, T()); }
  m_vector(size_t n, const value_type &value) : allocator() { init(n, value); }

  // 重写拷贝运算符（深拷贝）
  m_vector<T>& operator=(const value_type &other) {
    if (this != &other) {
      this->allocator      = other.allocator;
      this->start          = other.start;
      this->finish         = other.finish;
      this->end_of_storage = other.end_of_storage;
    }
    return *this;
  }
  // 移动拷贝构造
  m_vector<T>& operator=(const value_type &&other) {
    if (this != &other) {
      this->allocator = other.allocator;
      this->start = other.start;
      this->finish = other.finish;
      this->end_of_storage = other.end_of_storage;
      other.~m_vector<T>();
    }
    return *this;
  }

  // 重载运算
  reference operator[](size_t n) { return start[n]; }

  // 基础功能函数
  difference_type size() { return difference_type(start - finish); }
  difference_type capacity() { return difference_type(end_of_storage - start); }

  iterator begin() { return start; }
  const_iterator begin() const { return start; }
  iterator end() { return end_of_storage; }
  const_iterator end() const { return end_of_storage; }


  void push_back(const value_type &value);
  iterator insert(iterator pos, const_reference value);

  
  // 析构函数
  ~m_vector() {
    if (start != nullptr) {
      iterator tmp = start;
      for (; tmp != end_of_storage; tmp++) {
        deconstruct<iterator, value_type>(tmp);
      }
      allocator.deallocate(start,
                           (end_of_storage - start) * sizeof(value_type));
    }
  }

protected:
  void extend_capacity(const value_type &value);
  void extend_capacity();

  void copy(iterator src_begin, iterator src_end, iterator des_begin,
            iterator des_end);
  
  

private:
  Default_alloctor allocator;
  iterator start;
  iterator finish;
  iterator end_of_storage;
};

template <typename T, typename Default_alloctor>
typename m_vector<T,Default_alloctor>::iterator
m_vector<T, Default_alloctor>::allocate_and_fill(std::size_t n,
                                                 const value_type &value) {
  iterator result =
      static_cast<iterator>(allocator.allocate(n * sizeof(value_type)));
  try {
    uninit<iterator, value_type>(result, n, value);
    return result;
  } catch (...) {
    allocator.deallocate(result, n * sizeof(value_type));
    throw;
  }
}

template <typename T, typename Default_alloctor>
void m_vector<T,Default_alloctor>::extend_capacity(const value_type &value) {
  // 申请新空间
  std::size_t new_size =
      (end_of_storage - start) != 0 ? (end_of_storage - start) * 2 : 10;
  iterator new_mem =
      static_cast<iterator>(allocator.allocate(new_size * sizeof(value_type)));
  // 复制内容
  iterator new_finish = new_mem;
  iterator old_start = start;
  if (start != nullptr) {
    for (; old_start != end_of_storage; old_start++, new_finish++) {
      new (new_finish) value_type(*old_start);
      // 将旧空间中的对象非平凡类型析构
      // 这里要使用函数重载 看是不是POD
      deconstruct<iterator, value_type>(old_start);
    }

    // 释放原内存
    allocator.deallocate(start, (end_of_storage - start) * sizeof(value_type));
  }
  start = new_mem;
  finish = new_finish;
  end_of_storage = start + new_size;
  // 插入新内容
  push_back(value);
}

template <typename T, typename Default_alloctor>
void m_vector<T, Default_alloctor>::extend_capacity() {
  // 申请新空间
  std::size_t new_size =
      (end_of_storage - start) != 0 ? (end_of_storage - start) * 2 : 10;
  iterator new_mem =
      static_cast<iterator>(allocator.allocate(new_size * sizeof(value_type)));
  // 复制内容
  iterator new_finish = new_mem;
  iterator old_start = start;
  if (start != nullptr) {
    for (; old_start != end_of_storage; old_start++, new_finish++) {
      new (new_finish) value_type(*old_start);
      // 将旧空间中的对象非平凡类型析构
      // 这里要使用函数重载 看是不是POD
      deconstruct<iterator, value_type>(old_start);
    }

    // 释放原内存
    allocator.deallocate(start, (end_of_storage - start) * sizeof(value_type));
  }
  start = new_mem;
  finish = new_finish;
  end_of_storage = start + new_size;
}

template <typename T, typename Default_alloctor>
void m_vector<T,Default_alloctor>::push_back(const value_type &value) {
  if (finish != end_of_storage) {
    uninit<iterator, value_type>(finish, 1, value);
  } else {
    extend_capacity(value);
  }
}
template <typename T, typename Default_alloctor>
typename m_vector<T, Default_alloctor>::iterator
    m_vector<T, Default_alloctor>::insert(iterator pos,const_reference value) {
  // 满了
  if (finish == end_of_storage) {
    difference_type n = pos - start;
    extend_capacity();
    pos = start + n;
  }
  copy(pos, finish,
       pos + 1,finish + 1);
  new(pos) value_type(value);
  return pos;
}

template <typename T, typename Default_alloctor>
void m_vector<T, Default_alloctor>::copy(iterator src_begin, iterator src_end,
                                         iterator des_begin, iterator des_end) {
  for (; src_end != src_begin; src_end--, des_end--) {
    *des_end = std::move(*src_end);
  }
  finish++;
}
#endif // _MY_VECTOR_H_