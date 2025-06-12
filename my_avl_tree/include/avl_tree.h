#include "memoryPool.h"
#include <algorithm>
#include <iostream>
#include <utility>
#include <new>
namespace m_stl {

template <typename T> class AVLNode {
public:
  AVLNode *left;
  AVLNode *right;
  int height;
  T data;

public:
  AVLNode() : left(nullptr), right(nullptr), height(0), data() {}
  AVLNode(AVLNode *left_, AVLNode *right_, int height_, const T &data_)
      : left(left_), right(right_), height(height_), data(data_) {}
  AVLNode(AVLNode *left_, AVLNode *right_, int height_, const T &&data_)
      : left(left_), right(right_), height(height_), data(std::move(data_)) {}
};

template <typename T, typename DefualtAllocator = my_malloc_allocator>
class AVL_Tree {
public:
  using PNode = AVLNode<T> *;
  using Node = AVLNode<T>;

  AVL_Tree() : root(nullptr) {
    DefualtAllocator::initializer();
    }

public:
  template<typename U>
  void insert(U &&value) {
    root = insertHelper(root, std::forward<U>(value));
    std::cout << "插入 " << value << " 成功" << std::endl;
  }

  void print() { printHelper(root); }

protected:
  void  printHelper(PNode node) {
    if (!node)
      return;
    std::cout << node->data << ' ';
    printHelper(node->left);
    printHelper(node->right);
    return;
  }
  template<typename U>
  PNode insertHelper(PNode node, U &&value) {
    if (!node)
      return construct_node(std::forward<U>(value));
    if (value < node->data) {
      std::cout << value << " < " << node->data << " → 向左子树插入"
                << std::endl;
      node->left = insertHelper(node->left, std::forward<U>(value));
    } else if (value > node->data) {
      std::cout << value << " > " << node->data << " → 向右子树插入"
                << std::endl;
      node->right = insertHelper(node->right, std::forward<U>(value));
    } else {
      std::cout << "错误 重复值！" << std::endl;
      
    }

    return balanceNode(node);
  }
  int getHeight(PNode node) { return node ? node->height : -1; }
  void updateHeight(PNode node) {
    if (node)
      node->height = 1 + std::max(getHeight(node->left), getHeight(node->right));
  }
  int getBalanceFactor(PNode node) {
    return node ? getHeight(node->left) - getHeight(node->right) : 0;
  }

  PNode rightRotate(PNode node) {
    /*
              A
            /   \
           B     C
          / \
         D   E
        /
       F <- 导致失衡
    */
    PNode B = node->left;
    PNode E = B->right;

    B->right = node;
    node->left = E;

    updateHeight(B);
    updateHeight(node);

    return B;
  }

  PNode leftRotate(PNode node) {
    /*
        A
       / \
      B   C
         / \
        D   E
             \
              F <- 导致失衡
    */
    PNode C = node->right;
    PNode D = C->left;

    C->left = node;
    node->right = D;

    updateHeight(node);
    updateHeight(C);

    return C;
  }

  PNode balanceNode(PNode node) {
    if (!node)
      return nullptr;

    int balance = getBalanceFactor(node);
    int leftBalance = getBalanceFactor(node->left);
    int rightBalance = getBalanceFactor(node->right);

    if (balance > 1 && leftBalance >= 0) {
      std::cout << "LL型 右旋" << std::endl;
      rightRotate(node);
    }

    if (balance < -1 && rightBalance <= 0) {
      std::cout << "RR型 左旋" << std::endl;
      leftRotate(node);
    }

    if (balance > 1 && leftBalance < 0) {
      std::cout << "LR型 左旋和右旋" << std::endl;
      node->left = leftRotate(node->left);
      return rightRotate(node);
    }

    if (balance < -1 && rightBalance > 0) {
      std::cout << "RL型 右旋和左旋" << std::endl;
      node->right = rightRotate(node->right);
      return leftRotate(node);
    }
    return node;
  }
  template<typename U>
  PNode construct_node(U &&value) {
    PNode node = static_cast<PNode>
        (DefualtAllocator::allocate(sizeof(Node)));
    new (node) Node(nullptr, nullptr, 0, std::forward<U>(value));
    //PNode node = new Node(nullptr, nullptr, 0, std::forward<U>(value));
    return node;
  }

private:
  PNode root;
}; // class AVL_Tree
} // namespace m_stl