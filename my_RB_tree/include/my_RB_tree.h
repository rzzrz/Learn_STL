#ifndef MY_RB_TREE_H_
#define MY_RB_TREE_H_

#include <memory>
#include <utility>

// 红黑树的规则
// 每个节点要么是红色，要么是黑色
// 根节点是黑色的
// 每个叶子节点（NIL节点，空节点）是黑色的
// 如果一个节点是红色的，则它的两个子节点都是黑色的
// 对于每个节点，从该节点到其所有后代叶子节点的简单路径上，均包含相同数目的黑色节点

// 插入的旋转方法
// 默认插入的节点是红色的
// 如果插入的是根节点 那就违反了根节点应该为黑色的规则直接转换颜色即可
//
// 如果插入节点的叔叔是红色的,违反了不能连续出现红色的规则,这是变换父亲和叔叔节点为黑色,
// 并将当前节点移动至爷爷节点递归查看当前节点是否出现冲突
//
// 如果插入节点的叔叔是黑色,进行旋转然后变色即可
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
      : left(l), right(r), parent(p), data(val), color(c) {};
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

  void rightRotate(node_pointer x){
    /*
                   y
                  /
                 /
                a
               / \
              /   \
             x     ?
            /
           /
          b
    */
    auto y = x->parent->parent;
    auto a = x->parent;
    auto b = a->left;

    y->left->


  }
  void leftRotate(node_pointer x);
  void insertFixup(node_pointer x){
    // 因为我们开始插入的节点默认是红色的,
    // 可能出现红色红色连续出现的情况,所以
    // 判断是否违背不能出现两个红色的规则
    while(x != root && x->parent->color == Color::Red){
        // 首先判断叔叔节点是左还是右
        if(x->parent == x->parent->parent->left){
            // 叔叔在右边
            node_pointer u = x->parent->parent->right;

            // 1.叔叔节点是红色的
            if(u != nullptr && u->color == Color::Red){
                x->parent->color = Color::Black;
                u->color = Color::Black;
                x->parent->parent->color = Color::Red;
                x = x->parent->parent;
            }else{
                // 2叔叔节点是黑色,孩子是左边的
                if(x == x->parent->left){
                    // ll 形
                    rightRotate(x->parent);
                }else{ // 3.叔叔节点是黑色,孩子在右边
                    // lr 形状 左旋转左孩子,右旋
                    
                }
            }
        }else{
            // 叔叔在左边

        }
    }
  }

public:
  BRTree() {
    NIL = std::make_shared<RBTreeNode<T>>(T());
    NIL->color = Color::Black;
    root = NIL;
  }
  void insert(value_type val) {
    // 默认是红色的
    node_pointer z = std::make_shared<RBTreeNode<T>>(val);
    node_pointer y = nullptr;
    node_pointer x = root;

    while (x != NIL) {
      y = x;
      if (z->data > x->data) {
        x = x->right;
      } else {
        x = x->left;
      }
    }

    if (y == NIL) {
      // 是根节点
      root = z;
      root->color = Color::Black;
    } else {
      if (z->data > y->data) {
        y->right = z;
      } else {
        y->left = z;
      }
    }
  }

  insertFixup(z);
};
#endif // MY_RB_TREE_H_
