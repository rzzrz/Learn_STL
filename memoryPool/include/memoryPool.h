#ifndef _MEMORYPOOL_H_
#define _MEMORYPOOL_H_
// 使用的是linux Ubuntu 24 发行版对应的页的大小是4096B
// 首先获取不同系统下的页大小

#include <cstddef>
#include <memory>
#include <new>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <unistd.h>
#endif // defined(_WIN32) || defined(_WIN64)

#define THREAD_ON
// THREADS_ON 多线程启动 和 包含 pthread.h的时候启动线程安全

#if defined(THREAD_ON) && defined(_PTHREAD_H)
#include <pthread.h>
#define LOCK(mtx) pthread_mutex_lock(mtx)
#define UNLOCK(mtx) pthread_mutex_unlock(mtx)

#else
#define LOCK(mtx)
#define UNLOCK(mtx)
#endif // defined(THREAD_ON)

inline size_t get_page_size() {
  static size_t page_size = 0;
  if (page_size != 0)
    return page_size; // 曾经获取过页大小

#if defined(_WIN32) || defined(_WIN64)
  SYSTEM_INFO info;
  GetSystemInfo(&info);
  page_size = info.dwPageSize;
#else
  page_size = sysconf(_SC_PAGESIZE);
#endif // defined(_WIN32) || defined(_WIN64)

  return page_size;
}

#if defined(DOUBLE_ALLOCATOR_OFF) // 关闭二级内存分配器
#define DOUBLE_ALLOC_ON flase
#else
#define DOUBLE_ALLOC_ON true
#endif // defined(DOUBLE_ALLOCATOR_OFF)

// 实现编译时多个内存池
class my_malloc_allocator {

  using custom_alloc_false_func = void (*)(); // 内存分配失败处理函数别名

public:
  // 首先初始化函数负责首次初始化内存池的初始内存块
  my_malloc_allocator() {}

  // 静态初始化构造函数
  static void initializer();

  // 析构函数
  ~my_malloc_allocator() { operator delete(memoryPoolPtr); }

  // 内存分配的接口
  static void *allocate(size_t n);

  // 内存释放接口
  static void deallocate(void *p, size_t size);

  // 使用内存池创建智能指针
  template <typename T> static std::shared_ptr<T> make_shared_with_pool();
  template <typename T, typename... Args>
  static std::shared_ptr<T> make_shared_with_pool(Args... args);

  template <typename T, size_t N>
  static std::shared_ptr<T> make_shared_with_pool();
  // 析构函数关闭内存池

private:
  // 一级的内存分配直接封装new和free进行分配
  static void *big_mem_allocate(size_t n);

  // 二级分配
#if DOUBLE_ALLOC_ON // 关闭二级内存分配器
  class small_mem_allocator {
  public:
    static void *small_mem_allocate(size_t n) {
      // 找到对应的内存块
      LOCK(&my_malloc_allocator::mtx);
      volatile obj *temp = free_list[FREELIST_INDEX(n)];
      if (temp == NULL) // 没有可以使用的空间了
      {
        void *r = refill(ROUND_UP(n));
        UNLOCK(&my_malloc_allocator::mtx);
        return r;
      }
      volatile obj *result = temp;
      temp = result->free_list_link;
      UNLOCK(&my_malloc_allocator::mtx);
      return (void *)result;
    }
  };

  // 查看内存池的内存是否还有没有 没挂载道free_list上的
  static void *refill(size_t n);
  static char *chunk_alloc(size_t n, int &obj);

  // 向上取整的函数
  static size_t ROUND_UP(size_t n) {
    return (((n + ALIGN - 1) / ALIGN) * ALIGN);
  }

  static size_t FREELIST_INDEX(size_t bytes) {
    return ((bytes + ALIGN - 1) / ALIGN - 1);
  }

#endif // not defined(__DOUBLE_ALLOCATOR_OFF)

#if DOUBLE_ALLOC_ON
  enum { ALIGN = 8 };
  enum { MAX_BYTES = 4096 };
  enum { FREELIST_SIZE = MAX_BYTES / ALIGN };

  union obj {
    union obj *free_list_link;
    char client_data[1];
  };

  // 这是一个数组
  static volatile obj *free_list[FREELIST_SIZE];

  static size_t heap_size; // 当前管理的堆内存总量

  static size_t page_size;
  static char *memoryPoolPtr;

  static char *start_free;
  static char *end_free;

  // 对于 多线程模式可能修改的变量是free_list,heap_szie,start_free,end_free
  // 所以在修改变量的时候要加上互斥锁
#if defined(THREAD_ON) && defined(_PTHREAD_H)
  static pthread_mutex_t mtx;
#endif // defined(THREAD_ON) && defined(_PTHREAD_H)
#endif // DOUBLE_ALLOC_ON
};

// 无参构造智能指针
template <typename T>
std::shared_ptr<T> my_malloc_allocator::make_shared_with_pool() {
  T *ptmp = (T *)my_malloc_allocator::allocate(sizeof(T));
  // new(ptmp) T();
  return std::shared_ptr<T>(ptmp, [](T *ptr) {
    // ptr->~T();
    my_malloc_allocator::deallocate((void *)ptr, sizeof(T));
  });
}


template <typename T, typename... Args>
std::shared_ptr<T>
my_malloc_allocator::make_shared_with_pool(Args... args) {
  T *ptmp = (T *)my_malloc_allocator::allocate(sizeof(T));
  new (ptmp) T(std::forward<Args>(args)...);
  return std::shared_ptr<T>(ptmp, [](T *ptr) {
    ptr->~T();
    my_malloc_allocator::deallocate((void *)ptr, sizeof(T));
  });
}




template <typename T, size_t N>
std::shared_ptr<T> my_malloc_allocator::make_shared_with_pool() {
  size_t size = N;
  T *ptmp = (T *)my_malloc_allocator::allocate(sizeof(T) * size);
  return std::shared_ptr<T>(ptmp, [size](T *ptr) {
    my_malloc_allocator::deallocate((void *)ptr, sizeof(T) * size);
  });
}

 

#endif // _MEMORYPOOL_H_