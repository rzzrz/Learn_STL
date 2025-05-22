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
template <int uniqueID> class my_malloc_allocator {

  using custom_alloc_false_func = void (*)(); // 内存分配失败处理函数别名

public:
  // 首先初始化函数负责首次初始化内存池的初始内存块
  my_malloc_allocator() {
#if DOUBLE_ALLOC_ON
    page_size = get_page_size();
    memoryPoolPtr = static_cast<char *>(operator new(page_size));
    start_free = memoryPoolPtr;
    end_free = start_free + page_size;
    heap_size = page_size;
#endif // DOUBLE_ALLOC_ON
  }

  my_malloc_allocator(custom_alloc_false_func func) {
#if DOUBLE_ALLOC_ON
    page_size = get_page_size();
    memoryPoolPtr = static_cast<char *>(operator new(page_size));
    start_free = memoryPoolPtr;
    end_free = start_free + page_size;
    heap_size = page_size;
#endif // DOUBLE_ALLOC_ON
  }

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
  char *memoryPoolPtr;

  static char *start_free;
  static char *end_free;

  // 对于 多线程模式可能修改的变量是free_list,heap_szie,start_free,end_free
  // 所以在修改变量的时候要加上互斥锁
#if defined(THREAD_ON) && defined(_PTHREAD_H)
  static pthread_mutex_t mtx;
#endif // defined(THREAD_ON) && defined(_PTHREAD_H)
#endif // DOUBLE_ALLOC_ON
};

#if DOUBLE_ALLOC_ON
template <int uniqueID> size_t my_malloc_allocator<uniqueID>::heap_size = 0;
template <int uniqueID> size_t my_malloc_allocator<uniqueID>::page_size = 0;

template <int uniqueID> char *my_malloc_allocator<uniqueID>::start_free = nullptr;
template <int uniqueID> char *my_malloc_allocator<uniqueID>::end_free = nullptr;

#if defined(THREAD_ON) && defined(_PTHREAD_H)

template <int uniqueID>
pthread_mutex_t my_malloc_allocator<uniqueID>::mtx = PTHREAD_MUTEX_INITIALIZER;
#endif // defined(THREAD_ON) && defined(_PTHREAD_H)

template <int uniqueID>
volatile typename my_malloc_allocator<uniqueID>::obj
    *my_malloc_allocator<uniqueID>::free_list[FREELIST_SIZE] = {nullptr};
#endif // DOUBLE_ALLOC_ON

template <int uniqueID>
void *my_malloc_allocator<uniqueID>::allocate(size_t n) {
#if DOUBLE_ALLOC_ON
  if (n > MAX_BYTES)
    return big_mem_allocate(n);
  else
    return small_mem_allocator::small_mem_allocate(n);
#endif // DOUBLE_ALLOC_ON
  return big_mem_allocate(n);
}

template <int uniqueID>
void my_malloc_allocator<uniqueID>::deallocate(void *p, size_t size) {
  if (p == nullptr)
    return;
#if DOUBLE_ALLOC_ON
  size = ROUND_UP(size);
  if (size <= MAX_BYTES) {
    LOCK(&my_malloc_allocator::mtx);
    volatile obj *my_free_list = free_list[FREELIST_INDEX(size)];
    ((obj *)p)->free_list_link = (obj *)my_free_list;
    my_free_list = (obj *)p;
    UNLOCK(&my_malloc_allocator::mtx);
    p = nullptr;
    return;
  }
#endif // DOUBLE_ALLOC_ON
  operator delete(p);
  p = nullptr;
  return;
}

// 无参构造智能指针
template <int uniqueID>
template <typename T>
std::shared_ptr<T> my_malloc_allocator<uniqueID>::make_shared_with_pool() {
  T *ptmp = (T *)my_malloc_allocator<uniqueID>::allocate(sizeof(T));
  // new(ptmp) T();
  return std::shared_ptr<T>(ptmp, [](T *ptr) {
    // ptr->~T();
    my_malloc_allocator<uniqueID>::deallocate((void *)ptr, sizeof(T));
  });
}

template <int uniqueID>
template <typename T, typename... Args>
std::shared_ptr<T>
my_malloc_allocator<uniqueID>::make_shared_with_pool(Args... args) {
  T *ptmp = (T *)my_malloc_allocator<uniqueID>::allocate(sizeof(T));
  new (ptmp) T(std::forward<Args>(args)...);
  return std::shared_ptr<T>(ptmp, [](T *ptr) {
    ptr->~T();
    my_malloc_allocator<uniqueID>::deallocate((void *)ptr, sizeof(T));
  });
}

template <int uniqueID>
void *my_malloc_allocator<uniqueID>::big_mem_allocate(size_t n) {
  void *temp = operator new(n);
  if (!temp) // 申请失败
  {
    // alloc_false_func();
    return nullptr;
  }
  return temp;
}
template <int uniqueID>
template <typename T, size_t N>
std::shared_ptr<T> my_malloc_allocator<uniqueID>::make_shared_with_pool() {
  size_t size = N;
  T *ptmp = (T *)my_malloc_allocator<uniqueID>::allocate(sizeof(T) * size);
  return std::shared_ptr<T>(ptmp, [size](T *ptr) {
    my_malloc_allocator<uniqueID>::deallocate((void *)ptr, sizeof(T) * size);
  });
}

template <int uniqueID> void *my_malloc_allocator<uniqueID>::refill(size_t n) {
  int nobj = 20;
  char *chunk = chunk_alloc(n, nobj); // 通过引用nobj返回能够返回的n大小的空间
  volatile obj **my_free_list = free_list + FREELIST_INDEX(n); // 挂载点

  if (1 == nobj) // 只足够一个n大小的空间
    return chunk;

  // 否则说明有多个空间我们需要把整块大小的空间拆开挂在到free_list上面

  // 不进行头插 为了保证以后一个指向null
  for (int i = 0; i <= nobj - 1; i++) {
    ((obj *)(chunk + i * n))->free_list_link = (obj *)(chunk + (i + 1) * n);
    if (i == nobj - 1)
      ((obj *)(chunk + i * n))->free_list_link = NULL;
  }

  *my_free_list = (obj *)(chunk);

  volatile obj *reuslt = *my_free_list;
  *my_free_list = (*my_free_list)->free_list_link;

  return (void *)reuslt;
  // 之后从挂载的剩余空间中返回一个空间
}

template <int uniqueID>
char *my_malloc_allocator<uniqueID>::chunk_alloc(size_t n, int &nobj) {
  // 这个函数是专门用来返回内存池chunk的

  char *result;
  size_t total_bytes = n * nobj;             // 要求的总大小
  size_t bytes_left = end_free - start_free; // 剩余内存

  if (bytes_left >= total_bytes) {
    result = start_free;
    start_free += total_bytes;
    return result;
  } else if (bytes_left >= n) {
    result = start_free;
    nobj = bytes_left / n;
    start_free += n * nobj;
    return result;
  } else {
    // 到这里就是chunk中已经不满足要分配的大小了
    // 为了充分利用free_list中挂在的内存我们要对其重新挂在后面的

    if (bytes_left > 0) {
      volatile obj **my_free_list = free_list + FREELIST_INDEX(bytes_left);
      ((obj *)(start_free))->free_list_link = (obj *)*my_free_list;
      (*my_free_list)->free_list_link = (obj *)(start_free);
    }

    size_t bytes_to_get = total_bytes * 2 + (heap_size >> 4);
    start_free = static_cast<char *>(operator new(bytes_to_get));
    if (0 == start_free) { // 最新分配内存失败了

      for (int i = n; i <= MAX_BYTES; i += ALIGN) {
        volatile obj **my_free_list = free_list + FREELIST_INDEX(n);
        if (my_free_list != 0) // 有空余的内存这样我们就把它从旧挂载点卸载
        {
          start_free = (char *)*my_free_list;
          end_free = start_free + i;
          (*my_free_list) = (*my_free_list)->free_list_link;
          return chunk_alloc(n, nobj);
        }
      }
    }
    end_free = start_free + bytes_to_get;
    heap_size += bytes_to_get;
    return chunk_alloc(n, nobj);
  }
}

#endif // _MEMORYPOOL_H_