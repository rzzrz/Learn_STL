#include "memoryPool.h"
#include <algorithm>

template<typename T>
class AVLNode {
public:
  AVLNode *left;
  AVLNode *right;
  int height;
  T data;

public:
  AVLNode() : left(nullptr), right(nullptr), height(0), data() {}
  AVLNode(AVLNode *left, AVLNode *right, int height, const T &data)
      : left(left), right(right), height(height), data(data) {}
  AVLNode(AVLNode *left, AVLNode *right, int height, const T &&data)
      : left(left), right(right), height(height), data(std::move(data)) {}
};

template <typename T,typename DefualtAllocator =  my_malloc_allocator<2>>
class AVL_Tree {
public:
  using PNode = AVLNode<T> *;
  using Node = AVLNode<T>;

  AVL_Tree() {
    DefualtAllocator();
  }
protected:
  int getHeight(PNode node) { return node ? node->height : -1; }
  void updateHeight(PNode node) {
    if (node)
      node->height = 1 + max(getHeight(node->left), getHeight(node->right));
  }
  int getBalanceFactor() {
    
  }

private:
  PNode root;
};