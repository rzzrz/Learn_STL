// 使用的是linux Ubuntu 24 发行版对应的页的大小是4096B 
// 首先获取不同系统下的页大小

#include <cstddef>
#include <new>
#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#else
    #include <unistd.h>
#endif // defined(_WIN32) || defined(_WIN64)

size_t get_page_size() {
  static size_t page_size = 0;
  if (page_size != 0)
    return page_size; // 曾经获取过页大小
  
#if defined(_WIN32) || defined(_WIN64)
  SYSTEM_INFO info;
  GetSystemInfo(&info);
  page_size = info.dwPageSizel
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
//template <int uniqueID>
class my_malloc_allocator {
  
  using custom_alloc_false_func = void (*)(); // 内存分配失败处理函数别名
  
public:
  // 首先初始化函数负责首次初始化内存池的初始内存块
  my_malloc_allocator() {
#if DOUBLE_ALLOC_ON
    page_szie = get_page_size();
    memoryPoolPtr = static_cast<char *>(operator new(page_szie));
    start_free = memoryPoolPtr;
    end_free = start_free + page_szie;
    heap_size = page_szie;
#endif // DOUBLE_ALLOC_ON
    //alloc_false_func = defult_mem_malloc_false;
  }

  my_malloc_allocator(custom_alloc_false_func func) {
#if DOUBLE_ALLOC_ON
    page_szie = get_page_size();
    memoryPoolPtr = static_cast<char *>(operator new(page_szie));
    start_free = memoryPoolPtr;
    end_free = start_free + page_szie;
    heap_size = page_szie;
#endif // DOUBLE_ALLOC_ON
    alloc_false_func = func;
  }

  // 内存分配的接口
  static void *allocate(size_t n);

  // 析构函数关闭内存池
private:
  // 一级的内存分配直接封装new和free进行分配
  static void *big_mem_allocate(size_t n);
  
  // 二级分配
#if DOUBLE_ALLOC_ON // 关闭二级内存分配器
  class small_mem_allocator {
  public:
    static void* small_mem_allocate(size_t n) {
      // 找到对应的内存块
      volatile obj *temp = free_list[FREELIST_INDEX(n)];
      if (temp == NULL) // 没有可以使用的空间了
      {
        void *r = refill(ROUND_UP(n));
        return r;
      }
      volatile obj *result = temp;
      temp = result->free_list_link;
      return (void*)result;
    }
    
  };

  // 查看内存池的内存是否还有没有 没挂载道free_list上的
  static void *refill(size_t n);
  static char *chunk_alloc(size_t n, int &obj);

  // 默认的处理内存分配失败的函数
  //
  // 一个可变参数模板进行高度自定义
  static void defult_mem_malloc_false();

  // 向上取整的函数
  static size_t ROUND_UP(size_t n) {
    return (((n + ALIGN - 1) / ALIGN) * ALIGN);
  }

  static size_t FREELIST_INDEX(size_t bytes) { return ((bytes + ALIGN - 1) / ALIGN -1); }
#else

#endif // not defined(__DOUBLE_ALLOCATOR_OFF)

#if DOUBLE_ALLOC_ON
  enum { ALIGN = 8 };
  enum { MAX_BYTES = 4096 };
  enum { FREELIST_SIZE = MAX_BYTES / MAX_BYTES };

  union obj {
    union obj *free_list_link;
    char client_data[1];
  };

  // 这是一个数组
  static volatile obj *free_list[FREELIST_SIZE];

  static size_t heap_size;// 当前管理的堆内存总量

  size_t page_szie;
  char *memoryPoolPtr;

  static char *start_free;
  static char *end_free;
#endif // DOUBLE_ALLOC_ON

  custom_alloc_false_func alloc_false_func;
};

#if DOUBLE_ALLOC_ON
size_t my_malloc_allocator::heap_size;

char *my_malloc_allocator::start_free;
char *my_malloc_allocator::end_free;

volatile my_malloc_allocator::obj *my_malloc_allocator::free_list[FREELIST_SIZE];
#endif // DOUBLE_ALLOC_ON

void *my_malloc_allocator::allocate(size_t n) {
#if DOUBLE_ALLOC_ON
  if (n > MAX_BYTES)
    return big_mem_allocate(n);
  else
    return small_mem_allocator::small_mem_allocate(n);
#endif // DOUBLE_ALLOC_ON
  return big_mem_allocate(n);
}


void *my_malloc_allocator::big_mem_allocate(size_t n) {
  void *temp = operator new(n);
  if (!temp) // 申请失败
  {
    //alloc_false_func();
    return nullptr;
  }
  return temp;
}


void *my_malloc_allocator::refill(size_t n) {
  int nobj = 20;
  char *chunk = chunk_alloc(n, nobj); // 通过引用nobj返回能够返回的n大小的空间
  volatile obj **my_free_list = free_list + FREELIST_INDEX(n); // 挂载点

  if (1 == nobj) // 只足够一个n大小的空间
    return chunk;

  // 否则说明有多个空间我们需要把整块大小的空间拆开挂在到free_list上面

  // 不进行头插 为了保证以后一个指向null
  for (int i = 0; i <= nobj - 1; i++) {
    ((obj *)(chunk + i * n))->free_list_link = (obj *)(chunk + (i+1) * n);
    if (i == nobj - 1)
      ((obj *)(chunk + i * n))->free_list_link = NULL;
  }

  *my_free_list = (obj *)(chunk);

  volatile obj *reuslt = *my_free_list;
  *my_free_list = (*my_free_list)->free_list_link;

  return (void *)reuslt;
  // 之后从挂载的剩余空间中返回一个空间
}


char *my_malloc_allocator::chunk_alloc(size_t n, int &nobj) {
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
template<int uniqueID>
void defult_mem_malloc_false() {
  
};

#include<iostream>

int main() {

  my_malloc_allocator allocator;
  //int *num = (int *)allocator.allocate(4096);

  // for (int i = 0; i < 1024; i++) {
  //   num[i] = i + 1;
  //   std::cout << num[i]<<' ';
  // }
  // std::cout<<std::endl;
  // std::cout << " allocate agin ";

  //double * dnum= (double *)allocator.allocate(8);
  //if(dnum) = 0.001;
  // //std::cout << *dnum;
  // std::cout << std::endl;

  // for (int i = 0; i < 1024; i++) {
  //   num[i] = i + 1;
  //   std::cout << num[i] << ' ';
  // }
  // std::cout<<std::endl;
  return 0;
  }
