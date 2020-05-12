#pragma once
#include <stdint.h>
#include <vector>
#include "Uncopyable.h"
#include "common.h"
using namespace std;

struct Meta{
	void* data;
	uint meta_size;
};

struct Block {
	Block* next_block;
};

//大小从8byte到128byte的碎片数据就会放到MetaAllocater里
class MetaPage {
public:
	void initialize(uint block_size, uint page_size) {
		this->block_size = block_size;
		this->page_size = page_size * block_size;
	}

	void* allocate();
	void deallocate(void* mem);

	MetaPage() {
		block_size = 0;
		current_block = nullptr;
		page_size = 0;
	}
	~MetaPage() {
		for (auto page : mem_pages)
			free(page);
	}

private:
	void create_page();

	uint block_size;
	vector<void*> mem_pages;
	Block* current_block;
	uint page_size;
};



class MetaAllocator {
public:
	static Meta allocate(uint size);

	static void deallocate(Meta data);

private:
	MetaAllocator() {
		for (int i = 0; i != block_num; i++)
			pages[i].initialize(get_block_size(i), page_size);
	}

	//传说中的微软面试题
	static inline uint round_up_block_size(uint block_size) {
		return (block_size + block_step - 1) & ~(block_step - 1);
	}
	static inline uint get_block_index(uint block_size) {
		return round_up_block_size(block_size) / block_step - 1;
	}

	static inline uint get_block_size(uint block_index) {
		return (block_index + 1) * block_step;
	}

	static MetaAllocator allocator;

	static const uint max_block_size = 128;
	static const uint block_num = 16;
	static const uint block_step = max_block_size / block_num;
	static const uint page_size = 0x80;

	//vector<void*> large_pages;
	MetaPage pages[block_num];
};


//for class need to allocate temparory memory,they can inherit this class
class MetaPool {
public:
	virtual ~MetaPool() { releaseAll(); }
	MetaPool() { pool.reserve(8); }

	Meta allocate(uint size,uint* index = nullptr) {
		Meta new_mem = MetaAllocator::allocate(size);
		pool.push_back(new_mem);
		if (index) *index = pool.size() - 1;
		return new_mem;
	}

	Meta update_pool(uint index,uint size) {
		Meta old_meta = pool[index];
		MetaAllocator::deallocate(old_meta);
		Meta new_meta = MetaAllocator::allocate(size);
		pool[index] = new_meta;
		return new_meta;
	}

	void releaseAll() {
		for (auto m : pool) {
			MetaAllocator::deallocate(m);
		}
		pool.clear();
	}

private:
	vector<Meta> pool;
};