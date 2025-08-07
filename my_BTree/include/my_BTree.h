#ifndef MY_BTREE_H_
#define MY_BTREE_H_

#include <vector>
#include <iostream>
#include <algorithm> // 用于std::move_backward

/**
 * @class B树节点的模板类
 * @tparam T 存储的数据类型得支持<操作
 * @tparam order B树的阶数
 */
template <typename T, int order> class BTreeNode {
public:
    using Node = BTreeNode<T, order>;
    using NodePointer = Node*;
    using valueType = T;
    using reference =  T &;
    using const_reference  = const T &;

    bool isLeaf;                         // 是不是叶子节点
    int keyCount;                        // 当前节点存储值的个数
    std::vector<valueType> keys;         // 节点存储的值
    std::vector<NodePointer> children;   // 节点的孩子

    /**
     * @brief 构造函数
     *
     * @param isLeaf 是否为叶子节点
     */
    BTreeNode(bool isLeaf) : isLeaf(isLeaf), keyCount(0) {
        // 存储order-1个数的值
        keys.resize(order - 1);
        if (!isLeaf) {
            // 非叶子节点初始化order个子节点指针
            children.resize(order, nullptr);
        }
    }

    ~BTreeNode() {
        // 递归删除所有子节点
        for (NodePointer child : children) {
            if (child) delete child;
        }
    }

    /**
     * @brief 向未满节点插入数据 (递归的)最终都要插入到叶子节点处
     * 
     * @param val 被插入数据
     */
    void insertNoFill(const_reference val) {
        int i = keyCount - 1;  // 从最后一个键开始
        
        if (isLeaf) {
            // 叶子节点：找到插入位置并移动元素
            while (i >= 0 && val < keys[i]) {
                keys[i + 1] = keys[i];  // 向后移动大于val的键
                i--;
            }
            keys[i + 1] = val;  // 插入新键
            keyCount++;         // 更新键数量
        } else {
            // 内部节点：找到合适的子节点
            while (i >= 0 && val < keys[i]) i--;
            i++;  // 此时i是应该插入的子节点索引
            
            // 检查子节点是否需要分裂
            if (children[i]->keyCount == order - 1) {
                splitChild(i, children[i]);  // 分裂子节点
                
                // 分裂后可能需要调整插入位置
                if (val > keys[i]) i++;
            }
            
            // 递归插入到子节点
            children[i]->insertNoFill(val);
        }
    }

    /**
     * @brief 分裂当前节点的子节点
     * 
     * @param i 子节点在children数组中的索引
     * @param fullChild 需要分裂的子节点指针
     */
    void splitChild(int i, NodePointer fullChild) {
        // 创建新节点作为右兄弟
        NodePointer newChild = new Node(fullChild->isLeaf);
        
        // 计算中间键位置（提升到父节点）
        const int midIndex = (order - 1) / 2;
        const int newChildKeyCount = (order - 1) - midIndex - 1;
        
        // 新节点获取后半部分键
        newChild->keyCount = newChildKeyCount;
        for (int j = 0; j < newChildKeyCount; j++) {
            newChild->keys[j] = fullChild->keys[j + midIndex + 1];
        }
        
        // 如果不是叶子节点，复制对应的孩子指针
        if (!fullChild->isLeaf) {
            for (int j = 0; j <= newChildKeyCount; j++) {
                newChild->children[j] = fullChild->children[j + midIndex + 1];
                fullChild->children[j + midIndex + 1] = nullptr; // 原节点不再拥有这些孩子
            }
        }
        
        // 原节点保留前半部分键
        fullChild->keyCount = midIndex;
        
        // 在父节点中为新节点腾出位置
        // 1. 移动子节点指针
        for (int j = keyCount; j > i; j--) {
            children[j + 1] = children[j];
        }
        children[i + 1] = newChild;
        
        // 2. 移动键值
        for (int j = keyCount - 1; j >= i; j--) {
            keys[j + 1] = keys[j];
        }
        
        // 3. 插入中间键
        keys[i] = fullChild->keys[midIndex];
        keyCount++;
    }
};

/**
 * @brief B树类
 *
 * @tparam T 存储元素类型
 * @tparam order B树阶数
 */
template <typename T, int order> class BTree {
    using Node = BTreeNode<T, order>;
    using NodePointer = Node*;
    using valueType = T;
    using reference =  T &;
    using const_reference  = const T &;
    
public:
    BTree() : root(nullptr) {}
    
    ~BTree() {
        if (root) delete root;
    }

    /**
     * @brief 插入元素
     * 
     * @param val 被插入的元素
     */
    void insert(const_reference val) {
        if (root == nullptr) {
            // 树为空：创建根节点
            root = new Node(true);
            root->keys[0] = val;
            root->keyCount = 1;
        } else {
            // 检查根节点是否已满
            if (root->keyCount == order - 1) {
                // 根节点已满：需要分裂
                NodePointer newRoot = new Node(false); // 创建新根
                newRoot->children[0] = root;          // 原根成为新根的子节点
                root = newRoot;                       // 更新根指针
                
                // 分裂原根节点
                newRoot->splitChild(0, newRoot->children[0]);
                
                // 根据val大小选择插入到哪个子树
                int i = (val > newRoot->keys[0]) ? 1 : 0;
                newRoot->children[i]->insertNoFill(val);
            } else {
                // 根节点未满：直接插入
                root->insertNoFill(val);
            }
        }
    }

    /**
     * @brief 打印B树结构（用于调试）
     */
    void print() const {
        if (root) {
            printTree(root, 0);
        } else {
            std::cout << "Empty B-Tree\n";
        }
    }

private:
    /**
     * @brief 递归打印树结构
     * 
     * @param node 当前节点
     * @param depth 当前深度
     */
    void printTree(NodePointer node, int depth) const {
        // 打印缩进
        for (int i = 0; i < depth; i++) {
            std::cout << "  ";
        }
        
        // 打印当前节点键值
        std::cout << "[" << depth << "]: ";
        for (int i = 0; i < node->keyCount; i++) {
            std::cout << node->keys[i];
            if (i < node->keyCount - 1) std::cout << ", ";
        }
        std::cout << "\n";
        
        // 递归打印子节点
        if (!node->isLeaf) {
            for (int i = 0; i <= node->keyCount; i++) {
                if (node->children[i]) {
                    printTree(node->children[i], depth + 1);
                }
            }
        }
    }

    NodePointer root; // B树的根节点
};

#endif // MY_BTREE_H_
