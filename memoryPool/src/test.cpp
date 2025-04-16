#include "memoryPool.h"
#include <iostream>
#include <memory>
#include <pthread.h>
#include <sstream>

pthread_mutex_t cout_mtx = PTHREAD_MUTEX_INITIALIZER;
// pthread_mutex_t memPool_mtx = PTHREAD_MUTEX_INITIALIZER;

void *my_thread_handle(void *arg) {
  pthread_t threadID = pthread_self();
  my_malloc_allocator<1> *allocator = (my_malloc_allocator<1> *)arg;
  std::stringstream ss;

  // pthread_mutex_lock(&memPool_mtx);
  int *num = (int *)allocator->allocate(40);
  // pthread_mutex_unlock(&memPool_mtx);

  if (!num)
    std::cout << threadID << "线程：内存分配失败" << std::endl;
  else {
    for (int i = 0; i < 10; i++) {
      num[i] = i + 1;
      ss << "thread:" << threadID << "打印:" << num[i] << std::endl;
    }
  }
  // pthread_mutex_lock(&memPool_mtx);
  allocator->deallocate((void *)num, 40);
  // pthread_mutex_unlock(&memPool_mtx);

  pthread_mutex_lock(&cout_mtx);
  std::cout << ss.str();
  pthread_mutex_unlock(&cout_mtx);
  return nullptr;
}


int main() {

  //定义内存池
  my_malloc_allocator<1> allocator;

  // 定义线程数组
  // pthread_t thread_1;
  // pthread_t thread_2;
  pthread_t threads[10];

  void *reval;
  for (int i = 0; i < 10; i++) {
    if (pthread_create(threads + i, NULL, my_thread_handle, (void
    *)&allocator))
      std::cout<<"创建线程:"<<i+1<<"失败"<<std::endl;
  }

  for (int i = 0; i < 10; i++) {
    pthread_join(threads[i], &reval);
    std::cout<<"thread["<<i+1<<"]"<<"join"<<std::endl;
  }

  return 0;
}
