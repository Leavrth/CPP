//
// Created by legen on 2020/9/8.
//

/*
 * B+Tree:
 *  An M-order B+ tree has several characteristics:
 *  1.  The intermediate node who has k subtrees involves
 *      k elements only to index.
 *  2.  All the leaves contain the whole data and the pointers to these.
 *      Besides, the leaves is linked from the smallest key to the largest one.
 *  3.  The intermediate node's data is also exist in the sub-node, where
 *      the data is the largest one.
 *  e.g.
 *                          [8   15]
 *                              |
 *                 +------------+------------+
 *                 |                         |
 *            [2   5   8]                [11   15]
 *                 |                        |
 *       +---------+---------+         +-----------+
 *       |         |         |         |           |
 *    [1  2] -> [3  5] -> [6  8] -> [9  11] -> [13  15]
 *
 *  4.  Each node has at most m sub-nodes.
 *  5.  Each node (not leaves) has at least ceil(M/2) sub-nodes.
 */
#ifndef UNTITLED_B_TREE_H
#define UNTITLED_B_TREE_H
#include <memory>
#include <cassert>

template<typename K, typename V>
class BNode {
public:
    typedef std::pair<K, std::shared_ptr<BNode<K, V>>> block_t;
    explicit BNode(bool is_leaf, int m) : is_leaf_(is_leaf), m_(m), next_index_(0), block_(new block_t[m + 1]) {}
    virtual void clear() {}
    virtual ~BNode() = default;

    inline bool isLeaf() const { return is_leaf_; }
    inline bool isFull() const { return next_index_ == m_ + 1; }
    std::shared_ptr<BNode<K, V>> getChildNode(int index) const { return block_[index].second; }
    int getChildPointerIndex(K, bool = false) const;
    void addElem(K key);
    void addIndex(const std::shared_ptr<BNode<K, V>> &, int);
    std::shared_ptr<BNode<K, V>> split();
private:
    BNode() = default;
    bool is_leaf_;
    const int m_;
    int next_index_;
    std::unique_ptr<block_t[]> block_;
    std::weak_ptr<K> next_block_;
};

template<typename K, typename V>
int BNode<K, V>::getChildPointerIndex(K key, bool isSet) const {
    assert(is_leaf_ == false);  // This is for Intermediate Node
    const int last_index = next_index_ - 1;
    if (isSet && key > block_[last_index].first) {
        block_[last_index].first = key;
        return block_[last_index].second;
    }
    int l = 0;
    int r = last_index;
    while (l < r) {
        int m = l + (r - l) / 2;
        if (block_[m].first < key)  l = m + 1;
        else r = m;
    }
    // Notice that if key > block_[next_index_ - 1], l is also equal to r.
    // So we choose to the last block, and modify it after finish 'set'.
    assert(l == r); // We can promise now l == r.
    return l;
}

template<typename K, typename V>
void BNode<K, V>::addElem(K key) {
    assert(is_leaf_ == true);   // This is for Leaf Node
    assert(next_index_ <= m_);  // It can't be over the array.
    // What if the key is already exist?
    int l = 0;
    int r = next_index_ - 1;
    if (l < r) {
        int m = l + (r - l) / 2;
        if (block_[m].first == key) { return; }
        if (block_[m].first < key) l = m + 1;
        else r = m;
    }
    assert(l == r);
    if (block_[l].first == key) { return; }
    // Notice that the leaf does not have sub-nodes.
    // So block_[i].second should be ignored.
    for (int i = next_index_++; i != 0; --i) {  // We should update next_index_
        if (block_[i-1].first < key) {
            block_[i].first = key;
            return;
        }
        block_[i].first = block_[i-1].first;
    }

    // Now we know the key is the smallest one.
    block_[0].first = key;

}

template<typename K, typename V>
void BNode<K, V>::addIndex(const std::shared_ptr<BNode<K, V>> &node, int index) {
    assert(index >= -1 && index < m_ && next_index_ > index);
    for (int i = next_index_++; i != index + 1; --i) {  // We should update next_index_
        block_[i] = block_[i-1];
    }
    block_[index + 1] = std::make_shared(node);
}

template<typename K, typename V>
std::shared_ptr<BNode<K, V>> BNode<K, V>::split() {
    assert(isFull());   // only full node can be split
    int m = m_ / 2; // m is the last of the first sub-block
    next_index_ = m + 1;    // the node itself becomes the first sub-block
    std::shared_ptr<BNode<K, V>> block_nd(new BNode<K, V>(is_leaf_, m_));
    block_nd->next_index_ = m_ - m;
    if (is_leaf_) {
        for (int i = next_index_; i <= m_; ++i)
            block_nd->block_[i - next_index_].first = block_[i];
    } else {
        for (int i = next_index_; i <= m_; ++i) {
            block_t &entry_w = block_nd->block_[i - next_index_];
            block_t &entry_r = block_[i];
            entry_w.first = entry_r.first;
            entry_w.second = entry_w.second;
        }
    }
    block_nd->next_block_ = next_block_;
    next_block_ = block_nd;
    return block_nd;
}

/****************************************************************/

#define DISK_BLOCK 512
#define DATA_SIZE_B 4
#define MIN_M 16    // It shouldn't be too small.
template<typename K, typename V>
class BPlusTree {
public:

    explicit BPlusTree(int m = DISK_BLOCK / DATA_SIZE_B);
    BPlusTree(const BPlusTree &) = delete;
    void operator=(const BPlusTree &) = delete;
    ~BPlusTree() { root_->clear(); }

    void set(K key, V value);
    V get(K key);
    void print();
private:
    //BPlusTree() = default;
    bool set(const std::shared_ptr<BNode<K, V>> &, K, V);
    const int m_;
    std::shared_ptr<BNode<K, V>> root_;
    std::weak_ptr<BNode<K, V>> head_;
};


template<typename K, typename V>
BPlusTree<K, V>::BPlusTree(int m)
    : m_(m > MIN_M ? m : MIN_M),
      root_(new BNode<K, V>(true, m_)),
      head_(root_) { }

template<typename K, typename V>
void BPlusTree<K, V>::set(K key, V value) {
    bool isFull = set(root_, key, value);
    if (isFull) {
        std::shared_ptr<BNode<K, V>> node(new BNode<K, V>(false, m_));
        std::shared_ptr<BNode<K, V>> node_nd = root_->split();
        node->addIndex(root_, -1);
        node->addIndex(node_nd, 0);
        root_ = node;
    }
}

template<typename K, typename V>
bool BPlusTree<K, V>::set(const std::shared_ptr<BNode<K, V>> &root, K key, V value) {
    if (root->isLeaf()) {
        root->addElem(key);
        return root->isFull();
    }

    int index = root->getChildPointerIndex(key, true);
    std::shared_ptr<BNode<K, V>> node = root->getChildNode(index);
    bool isFull = set(node, key, value);
    if (isFull) {   // node is full
        node = node->split();
        root->addIndex(node, index);
    }
    return root->isFull();
}

template<typename K, typename V>
V BPlusTree<K,V>::get(K key) {

}


template<typename K, typename V>
void BPlusTree<K, V>::print() {

}

#endif //UNTITLED_B_TREE_H
