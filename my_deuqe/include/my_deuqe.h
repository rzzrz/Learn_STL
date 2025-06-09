#ifndef MY_DEQUE_H_
#define MY_DEQUE_H_

#include "memoryPool.h"
#include <cstddef>
#include <initializer_list>

namespace m_stl {

template <typename T, typename Default_allocator = my_malloc_allocator<0>,
          size_t buffsize = (512 > sizeof(T)) ? 1 : (512 / sizeof(T))>
class my_deque {
public:
  template <typename U, typename Ref, typename Ptr, size_t buff_size>
  class my_iterator {
  public:
    using value_type = U;
    using referenve = Ref;
    using pointer = Ptr;
    using size_type = size_t;
    using pointer_difference = std::ptrdiff_t;
    using map_pointer = U **;
    using self = my_iterator;

  public:
    my_iterator();
    my_iterator(pointer cur, map_pointer map);
    my_iterator(const my_iterator &other);

    referenve operator[](size_type n);
    referenve operator*();
    pointer operator->();

    self &operator++();
    self operator++(int); // 后置++

    self &operator--();
    self operator--(int); // 后置--

    self &operator=(const my_iterator other);
    self &operator=(my_iterator &&other);

    size_type operator-(const my_iterator &other);

    self &operator+=(pointer_difference n);
    self &operator-=(pointer_difference n);

    bool operator==(const my_iterator &other);
    bool operator!=(const my_iterator &other);

    bool operator<(const my_iterator &other);
    bool operator>(const my_iterator &other);

  private:
    pointer cur;

    pointer first;
    pointer last;

    map_pointer map;
  }; // class iterator
  template <typename U, typename Ref, typename Ptr, size_t buff_size>
  class my_reverse_iterator {}; // class iterator
  using value_type = T;
  using reference = T &;
  using const_reference = const T &;
  using pointer = T *;
  using const_pointer = const T *;
  using size_type = std::size_t;
  using pointer_diff = std::ptrdiff_t;
  using map_type = T **;

  using iterator = my_iterator<T, reference, pointer, buffsize>;
  using const_iterator =
      my_iterator<T, const_reference, const_pointer, buffsize>;
  using reverse_iterator = my_reverse_iterator<T, reference, pointer, buffsize>;
  using const_reverse_iterator =
      my_reverse_iterator<T, const_reference, const_pointer, buffsize>;

  my_deque() {
    Default_allocator();
    // 创建map和节点
    create_map_and_nodes(0);
  }
  my_deque(size_type n, const value_type &value);
  my_deque(size_t n);
  my_deque(const my_deque &other);
  my_deque(my_deque &&other);

  my_deque &operator=(const my_deque &other);
  my_deque &operator=(my_deque &&other);

  void push_back(const_reference value);
  void push_front(const_reference value);

  template <typename... Args> void emplace_back(Args &&...args);
  template <typename... Args> void emplace_front(Args &&...args);
  template <typename... Args> void emplace(iterator it, Args &&...args);

  void pop_back();
  void pop_front();

  size_type size();
  bool empty();

  reference at(size_t n);
  const_reference at(size_type n) const;

  reference back();
  const_reference back() const;
  reference front();
  const_reference front() const;

  iterator begin();
  const_iterator cbegin() const;
  iterator end();
  const_iterator cend() const;

  reverse_iterator rbegin();
  const_reverse_iterator crbegin() const;
  reverse_iterator rend();
  const_reverse_iterator rcend() const;

  iterator erase(const_iterator position);
  iterator erase(const_iterator position, const_iterator last);

  void assign(size_type n, const_reference value);
  void assign(std::initializer_list<value_type> value_list);
  void assign(iterator first, iterator last);

  iterator insert(const_iterator position, const_reference &value);
  iterator insert(const_iterator position, reference &&value);
  iterator insert(const_iterator position, size_type n, const_reference value);
  iterator insert(const_iterator position, const_reference first,
                  const_iterator last);

  void shrink_to_fit();
  void swap(my_deque &other) noexcept;
  friend void swap(my_deque &lhs, my_deque &rhs) noexcept;
  void clear();

protected:
  void create_map_and_nodes(size_type n) {
    // 模版中buffsize是一个内存快中应该含有的元素个数
    // 而映射器map的大小应该是8默认
    size_type buff_size = buffsize;
    map = Default_allocator::allocate(map_size * sizeof(char *));
    //调整指针的等
  }

protected:
  enum { map_szie = 8 };
  map_type map;
  size_type map_size;

  Default_allocator allocator;

  iterator start;
  iterator finish;
}; // class my_deuque

} // namespace m_stl

#endif // MY_DEQUE_H_