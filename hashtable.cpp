#pragma once

#include <vector>
#include <functional>
#include <utility>

using namespace std;

template <class K, class V>
class HashTable
{
	struct DataNode
	{
		K key;
		V value;
		DataNode *prev;
		DataNode *next;
	};

private:
	static const int kBucketNum;
	static const double kLoadFactor;

	int size_;
	int bucket_num_;
	int used_bucket_num_;
	bool auto_rehash_;
	double load_factor_;
	DataNode *head;
	DataNode **table;
	hash<K> hash_fn_;

	int index(K&);
	void put(DataNode*);


public:
	HashTable(bool = true, int = kBucketNum, double = kLoadFactor);
	~HashTable();
	bool empty();
	bool rehash();
	bool has_key(K&&);
	bool has_key(const K&);
//	bool has_value(V&&);
//	bool has_value(const V&);
	int size();
	bool put(const pair<K, V>&, bool = true);
	bool remove(K&&);
	bool remove(const K&);
	V* get(K&&);
	V* get(const K&);
	vector<K>& get_all_keys();
	vector<V>& get_all_values();
};

template<class K, class V>
const int HashTable<K, V>::kBucketNum = 13;

template<class K, class V>
const double HashTable<K, V>::kLoadFactor = 0.75;

template <class K, class V>
HashTable<K, V>::HashTable(bool auto_rehash, int bucket_num, double load_factor)
{
	if (bucket_num < 1)
		bucket_num = 13;
	if (load_factor <= 0)
		load_factor = 0.75;
	this->bucket_num_ = bucket_num;
	this->load_factor_ = load_factor;
	this->auto_rehash_ = auto_rehash;
	size_ = 0;
	head = NULL;
	table = new DataNode *[bucket_num];
	for (int i = 0; i < bucket_num; i++)
		table[i] = NULL;
}


template <class K, class V>
HashTable<K, V>::~HashTable()
{
	while (head != NULL)
	{
		DataNode * curr = head;
		head = head->next;
		delete curr; // need to check
	}
	delete table;
	delete this;
}

template<class K, class V>
int HashTable<K, V>::index(K &key)
{
	return hash_fn_(key) % bucket_num_;
}

// this method is for rehash-use only
template<class K, class V>
void HashTable<K, V>::put(DataNode *new_node)
{
	int idx = index(new_node->key);
	DataNode * node = table[idx];
	if (node == NULL)
	{
		// if no node exists
		new_node->prev = NULL;
		new_node->next = head;
		if (head != NULL)
			head->prev = new_node;
		head = new_node;
		table[idx] = new_node;
		used_bucket_num_++;
	}
	else // if a collision occurs
	{
		// note that same key will not already exist since all data is from the
		// previous hash table
		// therefore, we can add the new node to the front of the bucket
		new_node->prev = node->prev;
		new_node->next = node;
		node->prev = new_node;
		table[idx] = new_node;
		if (new_node->prev != NULL)
			new_node->prev->next = new_node;
		else if (node == head)
			head = new_node;
	}
}

template<class K, class V>
bool HashTable<K, V>::empty()
{
	return size_ == 0;
}

template<class K, class V>
bool HashTable<K, V>::rehash()
{
	// backup old values
	int old_bucket_num = bucket_num_;
	DataNode * node = head;
	DataNode ** old_table = table;
	// create a new table
	head = NULL;
	used_bucket_num_ = 0;
	bucket_num_ = bucket_num_ * 2 + 1;
	table = new DataNode *[bucket_num_];
	for (int i = 0; i < bucket_num_; i++)
		table[i] = NULL;
	// fill the new table with old data
	while (node != NULL)
	{
		DataNode * curr = node;
		node = node->next;
		put(curr);
	}
	delete old_table;
	return true;
}

template<class K, class V>
bool HashTable<K, V>::has_key(K &&key)
{
	int idx = index(key);
	DataNode * node = table[idx];
	while (node != NULL && index(node->key) == idx)
	{
		if (node->key == key)
			return true;
		node = node->next;
	}
	return false;
}

template<class K, class V>
bool HashTable<K, V>::has_key(const K &key)
{
	int idx = index(key);
	DataNode * node = table[idx];
	while (node != NULL && index(node->key) == idx)
	{
		if (node->key == key)
			return true;
		node = node->next;
	}
	return false;
}

/* may be rewritten / removed since the function involves comparing values
template<class K, class V>
bool HashTable<K, V>::has_value(V &&value)
{
	DataNode * node = head;
	while (node != NULL)
	{
		if (value == node->value)
			return true;
		node = node->next;
	}
	return false;
}

template<class K, class V>
bool HashTable<K, V>::has_value(const V &value)
{
	DataNode * node = head;
	while (node != NULL)
	{
		if (value == node->value)
			return true;
		node = node->next;
	}
	return false;
}
*/

template<class K, class V>
int HashTable<K, V>::size()
{
	return size_;
}

template<class K, class V>
bool HashTable<K, V>::put(const pair<K, V> &kv_pair, bool replacing)
{
	K key = std::get<0>(kv_pair);
	V value = std::get<1>(kv_pair);
	if (auto_rehash_ && (double)used_bucket_num_ / (double)bucket_num_ > load_factor_)
		rehash();

	int idx = index(key);
	DataNode * node = table[idx];
	if (node == NULL)
	{
		// if no node exists
		DataNode * new_node = new DataNode;
		new_node->key = key;
		new_node->value = value;
		new_node->prev = NULL;
		new_node->next = head;
		if (head != NULL)
			head->prev = new_node;
		head = new_node;
		table[idx] = new_node;
		size_++;
		used_bucket_num_++;
		return false;
	}
	else // if a collision occurs
	{
		while (node != NULL && index(node->key) == idx)
		{
			if (node->key == key)    // if same key already exists
			{
				if (replacing)
				{
					node->value = value; // update its value
					return true;
				}
				else
					return false;
			}
			node = node->next;
		}
		// if not exist, add a new node to the front of the bucket
		node = table[idx];

		DataNode * new_node = new DataNode;
		new_node->key = key;
		new_node->value = value;
		new_node->prev = node->prev;
		new_node->next = node;
		node->prev = new_node;
		table[idx] = new_node;
		if (new_node->prev != NULL)
			new_node->prev->next = new_node;
		else if (node == head)
			head = new_node;
		size_++;
		return false;
	}
}

template<class K, class V>
V* HashTable<K, V>::get(K &&key)
{
	int idx = index(key);
	DataNode * node = table[idx];

	while (node != NULL && index(node->key) == idx)
	{
		if (node->key == key)
			return &node->value;
		node = node->next;
	}
	return NULL;
}

template<class K, class V>
V * HashTable<K, V>::get(const K &key)
{
	int idx = index(key);
	DataNode * node = table[idx];

	while (node != NULL && index(node->key) == idx)
	{
		if (node->key == key)
			return &node->value;
		node = node->next;
	}
	return NULL;
}

template<class K, class V>
bool HashTable<K, V>::remove(K &&key)
{
	int idx = index(key);
	DataNode * node = table[idx];
	if (node == NULL)
		return false;
	else
	{
		// find the node with the key
		while (node != NULL && node->key != key && index(node->key) == idx)
			node = node->next;
		if (node == NULL || index(node->key) != idx)
			return false; // if cannot find key, nothing is performed
		else
		{
			if (node->prev != NULL)
				node->prev->next = node->next;
			else if (node == head)
				head = node;
			if (node->next != NULL)
				node->next->prev = node->prev;
			if (node == table[idx])
			{
				if (node->next == NULL)
				{
					used_bucket_num_--;
					table[idx] = node->next;
				}
				else if (idx == index(node->next->key))
					table[idx] = node->next;
			}
			V value = node->value;
			delete node; // if found, delete it
			size_--;
			return true;
		}
	}
}

template<class K, class V>
bool HashTable<K, V>::remove(const K &)
{
	int idx = index(key);
	DataNode * node = table[idx];
	if (node == NULL)
		return false;
	else
	{
		// find the node with the key
		while (node != NULL && node->key != key && index(node->key) == idx)
			node = node->next;
		if (node == NULL || index(node->key) != idx)
			return false; // if cannot find key, nothing is performed
		else
		{
			if (node->prev != NULL)
				node->prev->next = node->next;
			else if (node == head)
				head = node;
			if (node->next != NULL)
				node->next->prev = node->prev;
			if (node == table[idx])
			{
				if (node->next == NULL)
				{
					used_bucket_num_--;
					table[idx] = node->next;
				}
				else if (idx == index(node->next->key))
					table[idx] = node->next;
			}
			V value = node->value;
			delete node; // if found, delete it
			size_--;
			return true;
		}
	}
}

template<class K, class V>
vector<K>& HashTable<K, V>::get_all_keys()
{
	vector<K> result;
	DataNode * node = head;
	while (node != NULL)
	{
		result.push_back(node->key);
		node = node->next;
	}
	return result;
}

template<class K, class V>
vector<V>& HashTable<K, V>::get_all_values()
{
	vector<V> result = vector<V>();
	DataNode * node = head;
	while (node != NULL)
	{
		result.push_back(node->value);
		node = node->next;
	}

	return result;
}