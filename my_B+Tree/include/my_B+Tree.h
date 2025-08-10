#ifndef MY_BPLUS_TREE_H_
#define MY_BPLUS_TREE_H_

#include <vector>
#include <iostream>
#include <algorithm> // 用于std::lower_bound

/**
 * @class B+树节点的模板类
 * @tparam T 存储的数据类型，需支持<操作
 * @tparam order B+树的阶数
 */
template <typename T, int order> class BPlusTreeNode {
public:
    using Node = BPlusTreeNode<T, order>;
    using NodePointer = Node*;
    using valueType = T;
    using const_reference = const T &;

    bool is_leaf;                   // 是否为叶子节点
    int key_count;                  // 当前节点存储键的数量
    std::vector<valueType> keys;    // 存储的键
    std::vector<NodePointer> children; // 子节点指针(仅内部节点使用)
    NodePointer next;               // 指向下一个叶子节点(仅叶子节点使用)
    NodePointer parent;             // 父节点指针

    /**
     * @brief 构造函数
     * @param is_leaf 是否为叶子节点
     */
    BPlusTreeNode(bool is_leaf) 
        : is_leaf(is_leaf), key_count(0), next(nullptr), parent(nullptr) {
        keys.resize(order - 1);      // 最多存储order-1个键
        if (!is_leaf) {
            children.resize(order, nullptr); // 内部节点有order个子节点
        }
    }

    ~BPlusTreeNode() {
        for (NodePointer child : children) {
            if (child) delete child;
        }
    }

    /**
     * @brief 在叶子节点中查找插入位置
     * @param key 要查找的键
     * @return 插入位置索引
     */
    int find_insert_position(const_reference key) const {
        // 使用二分查找找到第一个不小于key的位置
        auto it = std::lower_bound(keys.begin(), keys.begin() + key_count, key);
        return it - keys.begin();
    }

    /**
     * @brief 在叶子节点中插入键
     * @param key 要插入的键
     */
    void insert_into_leaf(const_reference key) {
        int pos = find_insert_position(key);
        
        // 移动后面的元素腾出位置
        for (int i = key_count; i > pos; i--) {
            keys[i] = keys[i - 1];
        }
        
        // 插入新键
        keys[pos] = key;
        key_count++;
    }
};

/**
 * @brief B+树类
 * @tparam T 存储元素类型
 * @tparam order B+树阶数
 */
template <typename T, int order> class BPlusTree {
    using Node = BPlusTreeNode<T, order>;
    using NodePointer = Node*;
    using valueType = T;
    using const_reference = const T &;
    
    static_assert(order >= 3, "B+ tree order must be at least 3");

public:
    BPlusTree() : root(nullptr), first_leaf(nullptr) {}
    
    ~BPlusTree() {
        if (root) delete root;
    }

    /**
     * @brief 插入元素
     * @param key 要插入的键
     */
    void insert(const_reference key) {
        // 情况1: 树为空
        if (root == nullptr) {
            root = new Node(true);  // 创建叶子节点
            first_leaf = root;      // 第一个叶子节点
            root->insert_into_leaf(key);
            return;
        }
        
        // 查找插入的叶子节点
        NodePointer leaf = find_leaf(key);
        
        // 情况2: 叶子节点有空间
        if (leaf->key_count < order - 1) {
            leaf->insert_into_leaf(key);
        } 
        // 情况3: 叶子节点已满，需要分裂
        else {
            split_leaf(leaf, key);
        }
    }

    /**
     * @brief 打印B+树结构(用于调试)
     */
    void print() const {
        if (root) {
            print_tree(root, 0);
            print_leaves();
        } else {
            std::cout << "Empty B+ Tree\n";
        }
    }

private:
    NodePointer root;       // 根节点
    NodePointer first_leaf; // 第一个叶子节点(用于遍历叶子节点)

    /**
     * @brief 查找键应插入的叶子节点
     * @param key 要查找的键
     * @return 叶子节点指针
     */
    NodePointer find_leaf(const_reference key) const {
        NodePointer current = root;
        
        // 从根节点向下查找，直到叶子节点
        while (!current->is_leaf) {
            int pos = current->find_insert_position(key);
            current = current->children[pos];
        }
        
        return current;
    }

    /**
     * @brief 分裂叶子节点
     * @param leaf 要分裂的叶子节点
     * @param key 要插入的键
     */
    void split_leaf(NodePointer leaf, const_reference key) {
        // 创建新叶子节点
        NodePointer new_leaf = new Node(true);
        
        // 计算分裂位置(中间键索引)
        const int split_index = order / 2;
        const int new_leaf_key_count = order - 1 - split_index;
        
        // 新叶子节点获取后半部分键
        for (int i = 0; i < new_leaf_key_count; i++) {
            new_leaf->keys[i] = leaf->keys[i + split_index];
        }
        new_leaf->key_count = new_leaf_key_count;
        
        // 原叶子节点保留前半部分键
        leaf->key_count = split_index;
        
        // 更新叶子节点链表
        new_leaf->next = leaf->next;
        leaf->next = new_leaf;
        new_leaf->parent = leaf->parent;
        
        // 确定新键插入位置(原节点或新节点)
        if (key < new_leaf->keys[0]) {
            leaf->insert_into_leaf(key);
        } else {
            new_leaf->insert_into_leaf(key);
        }
        
        // 中间键(新叶子节点的第一个键)需要提升到父节点
        const_reference promote_key = new_leaf->keys[0];
        
        // 如果没有父节点(即分裂根节点)
        if (leaf->parent == nullptr) {
            create_new_root(leaf, new_leaf, promote_key);
        } else {
            // 将新节点插入父节点
            insert_into_parent(leaf->parent, new_leaf, promote_key);
        }
    }

    /**
     * @brief 分裂内部节点
     * @param node 要分裂的内部节点
     * @param new_child 新子节点
     * @param key 提升的键
     */
    void split_internal(NodePointer node, NodePointer new_child, const_reference key) {
        // 创建新内部节点
        NodePointer new_node = new Node(false);
        
        // 计算分裂位置
        const int split_index = (order - 1) / 2;
        const int new_node_key_count = order - 1 - split_index - 1;
        
        // 新节点获取后半部分键和子节点
        for (int i = 0; i < new_node_key_count; i++) {
            new_node->keys[i] = node->keys[i + split_index + 1];
        }
        for (int i = 0; i <= new_node_key_count; i++) {
            new_node->children[i] = node->children[i + split_index + 1];
            if (new_node->children[i]) {
                new_node->children[i]->parent = new_node;
            }
            node->children[i + split_index + 1] = nullptr;
        }
        new_node->key_count = new_node_key_count;
        
        // 原节点保留前半部分
        node->key_count = split_index;
        
        // 提升键
        const_reference promote_key = node->keys[split_index];
        
        // 设置父节点
        new_node->parent = node->parent;
        
        // 如果没有父节点(分裂根节点)
        if (node->parent == nullptr) {
            create_new_root(node, new_node, promote_key);
        } else {
            // 将新节点插入父节点
            insert_into_parent(node->parent, new_node, promote_key);
        }
    }

    /**
     * @brief 创建新的根节点
     * @param left_child 左子节点
     * @param right_child 右子节点
     * @param key 第一个键
     */
    void create_new_root(NodePointer left_child, NodePointer right_child, const_reference key) {
        NodePointer new_root = new Node(false);
        new_root->keys[0] = key;
        new_root->key_count = 1;
        new_root->children[0] = left_child;
        new_root->children[1] = right_child;
        
        left_child->parent = new_root;
        right_child->parent = new_root;
        
        root = new_root;
    }

    /**
     * @brief 将新节点插入父节点
     * @param parent 父节点
     * @param new_child 新子节点
     * @param key 关联的键
     */
    void insert_into_parent(NodePointer parent, NodePointer new_child, const_reference key) {
        // 找到新节点在父节点中的插入位置
        int pos = parent->find_insert_position(key);
        
        // 移动键和子节点指针腾出位置
        for (int i = parent->key_count; i > pos; i--) {
            parent->keys[i] = parent->keys[i - 1];
        }
        for (int i = parent->key_count + 1; i > pos + 1; i--) {
            parent->children[i] = parent->children[i - 1];
        }
        
        // 插入键和子节点指针
        parent->keys[pos] = key;
        parent->children[pos + 1] = new_child;
        parent->key_count++;
        
        // 设置父节点
        new_child->parent = parent;
        
        // 如果父节点已满，分裂父节点
        if (parent->key_count == order - 1) {
            split_internal(parent, new_child, key);
        }
    }

    /**
     * @brief 递归打印树结构
     * @param node 当前节点
     * @param depth 当前深度
     */
    void print_tree(NodePointer node, int depth) const {
        // 打印缩进
        for (int i = 0; i < depth; i++) {
            std::cout << "  ";
        }
        
        // 打印节点类型和键
        if (node->is_leaf) {
            std::cout << "L" << depth << " (leaf): [";
        } else {
            std::cout << "L" << depth << " (internal): [";
        }
        
        for (int i = 0; i < node->key_count; i++) {
            std::cout << node->keys[i];
            if (i < node->key_count - 1) std::cout << ", ";
        }
        std::cout << "]\n";
        
        // 递归打印子节点
        if (!node->is_leaf) {
            for (int i = 0; i <= node->key_count; i++) {
                if (node->children[i]) {
                    print_tree(node->children[i], depth + 1);
                }
            }
        }
    }

    /**
     * @brief 打印所有叶子节点(按顺序)
     */
    void print_leaves() const {
        std::cout << "Leaf nodes: ";
        NodePointer current = first_leaf;
        while (current) {
            std::cout << "[";
            for (int i = 0; i < current->key_count; i++) {
                std::cout << current->keys[i];
                if (i < current->key_count - 1) std::cout << ", ";
            }
            std::cout << "]";
            if (current->next) std::cout << " -> ";
            current = current->next;
        }
        std::cout << "\n";
    }
};

#endif // MY_BPLUS_TREE_H_
