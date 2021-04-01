#pragma once
#include <cassert>



template<typename K, typename V>
struct skipnode {
	using node = skipnode<K, V>;
	node *right;
	node *bottom;
	K key;
	V value;
	skipnode(K key_, const V &value_, node *right_ = nullptr, node *bottom_ = nullptr)
		: key(key_), value(value_), right(right), bottom(bottom_) { }
	skipnode(K key_, V &&value_, node *right_ = nullptr, node *bottom_ = nullptr)
		: key(key_), value(std::move(value_)), right(right), bottom(bottom_) { }
	skipnode(const skipnode<K, V>& rhs)
		: skipnode(rhs.key, rhs.value) { }
	skipnode() : right(nullptr), bottom(nullptr) { }
};

template<typename K, typename V>
class skiplink {
public:
	using node = skipnode<K, V>;
	explicit skiplink() : hnode(new node()), num(1) { }

	// �ҵ����һ��������key��node
	static const node *get_bound(const node *pnode, K key) {
		assert(pnode != nullptr);
		while (pnode->right != nullptr) {
			if (pnode->right->key < key) {
				pnode = pnode->right;
			}
			else break;
		}
		return pnode;
	}

	~skiplink() {
		// ����delete
		while (hnode) {
			node *tmp = hnode->next;
			delete hnode;
			hnode = tmp;
		}
	}

private:
	node * hnode;
	unsigned int num;
};

template<typename K, typename V>
class skiplist {
public:
	using link = skiplink<K, V>;
	using node = skipnode<K, V>;
	skiplist() : head_list(new link()) { }
	void insert(K key, const V &value) {
		const node * pnode = this->head_list->hnode;
		while (pnode) {
			pnode = link::get_bound(pnode, key);
			// ��¼pnode
			// �ҵ�ֵ���õ���key�����¸�ֵ
			if (pnode->key == key) {
				
				break;
			}

			// �ҵ����ˣ������ֵ
			if (pnode->bottom == nullptr) {
				
				break;
			}

			pnode = pnode->bottom;
			
		}
	}
	bool find(K key, V &value);
	bool erase(K key);

	int get_level();

	// debug function:
	void print();

private:
	/*
	head_list_index		link
	0				 *					  -> key5
	1				 *      ->  key1		  -> key5	   -> key13
	2				 *		->  key1 ->...-> key5 ->...-> key13
	*/
	typename link *head_list;

};