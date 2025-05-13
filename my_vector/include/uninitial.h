#ifndef _UNINITIAL_H_
#define _UNINITIAL_H_

#include "./m_iterator_traits.h"
#include <cstddef>

template <typename Forward_iterator, typename value_type>
void __uninit(Forward_iterator begin, Forward_iterator end,
              const value_type &value,
              __false_type) // 这个是非平凡类型的重载函数
{
  for (;begin != end;begin++) {
    new (begin) value_type(value);
  }
}

template <typename Forward_iterator, typename value_type>
void __uninit(Forward_iterator begin, Forward_iterator end,
              const value_type &value,
              __true_type) // 这个是平凡类型的重载函数
{
  for (; begin != end; begin++) {
    *begin = value;    
  }
}

template <typename Forward_iterator, typename value_type>
void uninit(Forward_iterator iterator, std::size_t n, const value_type &value) {
  typedef typename m_type_traits<value_type>::is_POD_type is_POD;
  return __uninit(iterator, iterator + n, value, is_POD());
}

template <typename Forward_iterator, typename value_type>
void __deconstruct(Forward_iterator iterator,
                   __true_type) // 这个是平凡类型的重载函数
{
  return;
}

template <typename Forward_iterator, typename value_type>
void __deconstruct(Forward_iterator iterator,
                   __false_type) // 这个是平凡类型的重载函数
{
  (*iterator).~value_type();
}

template <typename Forward_iterator, typename value_type>
void deconstruct(Forward_iterator iterator) {
  typedef typename m_type_traits<value_type>::is_POD_type is_POD;
  return __deconstruct<Forward_iterator,value_type>(iterator, is_POD());
}

#endif // _UNINITIAL_H_