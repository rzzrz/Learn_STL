## 一些关于学习STL的笔记
### 由模版类中定义的结构的静态成员需要完整的作用域限定才能完成生命
```c++
template<int uniqueID>
class my_malloc_allocator {
public:
  union obj {  // 嵌套类型
    obj* free_list_link;
    char client_data[1];
  };
  
  static volatile obj* free_list[FREELIST_SIZE]; // 静态成员声明
};
// 错误示例:无法解析 obj 类型
template<int uniqueID>
volatile my_malloc_allocator::obj* my_malloc_allocator<uniqueID>::free_list[FREELIST_SIZE];

// 完整的声明
template<int uniqueID>
volatile typename my_malloc_allocator<uniqueID>::obj* my_malloc_allocator<uniqueID>::free_list[FREELIST_SZIE];

展开
```
### ```typename```关键词必须告诉编译器后面的这个my_malloc_allocator<uniqueID>::obj*是一个类型


## 内存复制函数要从后往前拷贝 能够解决拷贝区间重合的问题
