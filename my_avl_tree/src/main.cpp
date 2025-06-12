#include "../include/avl_tree.h"


int main() {
  m_stl::AVL_Tree<int> t;
  for (int i = 1; i <= 5; i++) {
    t.insert(i);
  }
  t.print();
}