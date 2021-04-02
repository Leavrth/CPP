#pragma once
#include <cassert>
#include <list>
#include <cstdlib>
#include <ctime>
#include <iostream>

template<typename K, typename V>
struct skipnode {
	using node = skipnode<K, V>;
	node *right;
	node *bottom;
	K key;
	V value;
	skipnode(K key_, const V &value_, node *right_ = nullptr, node *bottom_ = nullptr)
		: key(key_), value(value_), right(right_), bottom(bottom_) { }
	skipnode(K key_, node *right_ = nullptr, node *bottom_ = nullptr)
		: key(key_), right(right_), bottom(bottom_) { }
	skipnode(K key_, V &&value_, node *right_ = nullptr, node *bottom_ = nullptr)
		: key(key_), value(std::move(value_)), right(right_), bottom(bottom_) { }
	skipnode(const skipnode<K, V>& rhs)
		: skipnode(rhs.key, rhs.value) { }
	skipnode() : right(nullptr), bottom(nullptr) { }
};

template<typename K, typename V>
struct skiplink {
public:
	using node = skipnode<K, V>;
	explicit skiplink() : hnode(new node()), num(1), bottom(nullptr) { }

	// 找到最后一个不大于key的node
	static node *get_bound(node *pnode, K key) {
		assert(pnode != nullptr);
		while (pnode->right != nullptr) {
			if (pnode->right->key <= key) {
				pnode = pnode->right;
			}
			else break;
		}
		return pnode;
	}

	static node *get_no_bound(node *pnode, K key) {
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
		// 横向delete
		while (hnode) {
			node *tmp = hnode->next;
			delete hnode;
			hnode = tmp;
		}
	}


	node * hnode;
	unsigned int num;
	skiplink *bottom;
};

template<typename K, typename V>
class skiplist {
public:
	using link = skiplink<K, V>;
	using node = skipnode<K, V>;
	skiplist() : head_list(new link()), total(0) { srand((unsigned int) time(0)); }
	void insert(K key, const V &value) {
		link * plink = this->head_list;
		node * pnode = this->head_list->hnode;
		std::list<std::pair<link *, node *>> l;
		while (pnode) {
			pnode = link::get_bound(pnode, key);
			l.emplace_front(std::pair<link *, node *>(plink, pnode));

			// 记录pnode
			// 找到值正好等于key，往下改值
			if (pnode->key == key) {
				while (pnode->bottom) {
					//pnode->value = value;
					pnode = pnode->bottom;
				}
				pnode->value = value;
				
				break;
			}

			// 找到底了，插入该值
			if (pnode->bottom == nullptr) {
				assert(pnode->key != key);
				node * n = new node(key, value, pnode->right);
				pnode->right = n;
				++this->total;
				// 向上
				l.pop_front();
				while (l.size() > 0) {
					int k = rand();
					if (k & 0x1) {
						node *t = l.front().second;
						node *p = new node(key, t->right, n);
						t->right = p;
						n = p;
						l.pop_front();
					}
					else break;
				}
				if (l.size() == 0) {
					while (rand() & 0x1) {
						link *hlink = new link();
						hlink->bottom = this->head_list;
						n = new node(key, nullptr, n);
						hlink->hnode->bottom = this->head_list->hnode;
						hlink->hnode->right = n;
						this->head_list = hlink;
					}
				}

				break;
			}

			pnode = pnode->bottom;
			plink = plink->bottom;
			
		}
	}
	bool find(K key, V &value) {
		node *pnode = this->head_list->hnode;
		while (pnode) {
			pnode = link::get_bound(pnode, key);
			if (pnode->key == key) {
				while (pnode->bottom) {
					pnode = pnode->bottom;
				}
				value = pnode->value;
				return true;
			}

			pnode = pnode->bottom;
		}
		return false;
	}
	bool erase(K key) {
		node *pnode = this->head_list->hnode;
		while (pnode) {
			pnode = link::get_no_bound(pnode, key);
			if (pnode->right && pnode->right->key == key) {
				node *del_node = pnode->right;
				pnode->right = pnode->right->right;
				delete del_node;
			}
			pnode = pnode->bottom;
		}
		return true;
	}

	//int get_level();

	// debug function:
	void print() {
		std::cout << std::endl;
		link *plink = this->head_list;
		while (plink) {
			node *pnode = plink->hnode;
			while (pnode) {
				std::cout << pnode->key << " ";
				pnode = pnode->right;
			}
			
			std::cout << std::endl;
			plink = plink->bottom;
		}
		std::cout << std::endl;
		std::cout << std::endl;
	}

private:
	/*
	head_list_index		link
	0				 *					  -> key5
	1				 *      ->  key1		  -> key5	   -> key13
	2				 *		->  key1 ->...-> key5 ->...-> key13
	*/
	typename link *head_list;
	
	unsigned int total;
};