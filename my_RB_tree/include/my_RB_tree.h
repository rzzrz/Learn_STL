#ifndef MY_RB_TREE_H_
#define MY_RB_TREE_H_

#include <memory>
#include <utility>

enum Color { Red, Black };

template <typename T> struct RBTreeNode {
  using pointer = std::shared_ptr<RBTreeNode<T>>;
  using valueType = T;
  using leftValue = T &&;
  pointer left;
  pointer right;
  pointer parent;

  Color color;

  valueType data;

  RBTreeNode(valueType val, Color c = Red, pointer l = nullptr,
             pointer r = nullptr, pointer p = nullptr)
      : left(l), right(r), parent(p), data(val), color(c) {}
  RBTreeNode(leftValue val, Color c = Red, pointer l = nullptr,
             pointer r = nullptr, pointer p = nullptr)
      : left(l), right(r), parent(p), data(std::move(val)), color(c) {}
};

template <typename T> class BRTree {
  using node_pointer = std::shared_ptr<RBTreeNode<T>>;
  using value_type = T;

private:
  node_pointer root;
  node_pointer NIL;

  // 右旋转操作
  // 以x为轴进行右旋转：
  //      x               y
  //     / \             / \
  //    y   C    ->     A   x
  //   / \                 / \
  //  A   B               B   C
  void rightRotate(node_pointer x) {
    auto y = x->left;         // 保存x的左孩子y
    x->left = y->right;       // 将y的右子树变为x的左子树
    
    if (y->right != NIL) {
      y->right->parent = x;   // 更新B的父节点
    }
    
    y->parent = x->parent;    // 更新y的父节点
    
    // 更新祖父节点的孩子指针
    if (x->parent == NIL) {
      root = y;               // x是根节点的情况
    } else if (x == x->parent->right) {
      x->parent->right = y;
    } else {
      x->parent->left = y;
    }
    
    y->right = x;             // 将x变为y的右孩子
    x->parent = y;            // 更新x的父节点
  }

  // 左旋转操作
  // 以x为轴进行左旋转：
  //      x               y
  //     / \             / \
  //    A   y    ->     x   C
  //       / \         / \
  //      B   C       A   B
  void leftRotate(node_pointer x) {
    auto y = x->right;        // 保存x的右孩子y
    x->right = y->left;       // 将y的左子树变为x的右子树
    
    if (y->left != NIL) {
      y->left->parent = x;    // 更新B的父节点
    }
    
    y->parent = x->parent;    // 更新y的父节点
    
    // 更新祖父节点的孩子指针
    if (x->parent == NIL) {
      root = y;               // x是根节点的情况
    } else if (x == x->parent->left) {
      x->parent->left = y;
    } else {
      x->parent->right = y;
    }
    
    y->left = x;              // 将x变为y的左孩子
    x->parent = y;            // 更新x的父节点
  }

  // 插入后修复红黑树性质
  // 主要处理连续红色节点冲突
  void insertFixup(node_pointer z) {
    // 只有当父节点是红色时才需要修复
    while (z->parent->color == Color::Red) {
      // 父节点是祖父的左孩子
      if (z->parent == z->parent->parent->left) {
        auto uncle = z->parent->parent->right;  // 获取叔叔节点
        
        // Case 1: 叔叔是红色
        // 解决方案: 父叔变黑，祖父变红，z上移到祖父
        if (uncle->color == Color::Red) {
          z->parent->color = Color::Black;
          uncle->color = Color::Black;
          z->parent->parent->color = Color::Red;
          z = z->parent->parent;
        } 
        // Case 2/3: 叔叔是黑色
        else {
          // Case 2: LR形 (z是右孩子)
          // 解决方案: 左旋父节点转为LL形
          if (z == z->parent->right) {
            z = z->parent;
            leftRotate(z);
          }
          // Case 3: LL形 (z是左孩子)
          // 解决方案: 父变黑，祖父变红，右旋祖父
          z->parent->color = Color::Black;
          z->parent->parent->color = Color::Red;
          rightRotate(z->parent->parent);
        }
      } 
      // 父节点是祖父的右孩子 (对称情况)
      else {
        auto uncle = z->parent->parent->left;  // 获取叔叔节点
        
        // Case 1: 叔叔是红色
        if (uncle->color == Color::Red) {
          z->parent->color = Color::Black;
          uncle->color = Color::Black;
          z->parent->parent->color = Color::Red;
          z = z->parent->parent;
        } 
        // Case 2/3: 叔叔是黑色
        else {
          // Case 2: RL形 (z是左孩子)
          if (z == z->parent->left) {
            z = z->parent;
            rightRotate(z);
          }
          // Case 3: RR形 (z是右孩子)
          z->parent->color = Color::Black;
          z->parent->parent->color = Color::Red;
          leftRotate(z->parent->parent);
        }
      }
    }
    // 确保根节点始终为黑色
    root->color = Color::Black;
  }

public:
  BRTree() {
    // 初始化NIL节点
    NIL = std::make_shared<RBTreeNode<T>>(T());
    NIL->color = Color::Black;
    NIL->left = NIL;
    NIL->right = NIL;
    NIL->parent = NIL;
    root = NIL;
  }

  // 插入新节点
  void insert(value_type val) {
    // 创建新节点（默认为红色）
    node_pointer z = std::make_shared<RBTreeNode<T>>(val);
    z->left = NIL;
    z->right = NIL;
    z->parent = NIL;
    
    node_pointer y = NIL;   // 跟踪插入位置的父节点
    node_pointer x = root;  // 遍历指针

    // 查找插入位置
    while (x != NIL) {
      y = x;
      if (z->data < x->data) {
        x = x->left;
      } else {
        x = x->right;
      }
    }

    // 设置新节点的父节点
    z->parent = y;
    
    // 处理空树情况
    if (y == NIL) {
      root = z;
    } 
    // 插入到左子树
    else if (z->data < y->data) {
      y->left = z;
    } 
    // 插入到右子树
    else {
      y->right = z;
    }
    
    // 修复红黑树性质
    insertFixup(z);
  }
};
#endif // MY_RB_TREE_H_

