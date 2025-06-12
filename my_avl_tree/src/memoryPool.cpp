#include "../include/memoryPool.h"
#define DOUBLE_ALLOC_ON true
#if DOUBLE_ALLOC_ON
size_t my_malloc_allocator::heap_size = 0;
size_t my_malloc_allocator::page_size = 0;

char *my_malloc_allocator::memoryPoolPtr = nullptr;

char *my_malloc_allocator::start_free =nullptr;
char *my_malloc_allocator::end_free = nullptr;

#if defined(THREAD_ON) && defined(_PTHREAD_H)

pthread_mutex_t my_malloc_allocator::mtx = PTHREAD_MUTEX_INITIALIZER;
#endif // defined(THREAD_ON) && defined(_PTHREAD_H)

volatile typename my_malloc_allocator::obj
    *my_malloc_allocator::free_list[FREELIST_SIZE] = {};
#endif // DOUBLE_ALLOC_ON

void my_malloc_allocator::initializer() {
  if (memoryPoolPtr)
    return;
#if DOUBLE_ALLOC_ON
  page_size = get_page_size();
  memoryPoolPtr = static_cast<char *>(operator new(page_size));
  start_free = memoryPoolPtr;
  end_free = start_free + page_size;
  heap_size = page_size;
#endif // DOUBLE_ALLOC_ON
}

void *my_malloc_allocator::allocate(size_t n) {
#if DOUBLE_ALLOC_ON
  if (n > MAX_BYTES)
    return big_mem_allocate(n);
  else
    return small_mem_allocator::small_mem_allocate(n);
#endif // DOUBLE_ALLOC_ON
  return big_mem_allocate(n);
}

void my_malloc_allocator::deallocate(void *p, size_t size) {
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

void *my_malloc_allocator::big_mem_allocate(size_t n) {
  void *temp = operator new(n);
  if (!temp) // 申请失败
  {
    // alloc_false_func();
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