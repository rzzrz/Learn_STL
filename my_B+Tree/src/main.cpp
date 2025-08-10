#include "../include/my_B+Tree.h"
#include <cstdint>

int main(){
    BPlusTree<int,100> tree;

    for(int i = 0;i<10000;i++){
        tree.insert(i);
    }
    
    tree.print();
}