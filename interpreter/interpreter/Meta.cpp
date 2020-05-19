#include "Meta.h"

void* MetaPage::allocate() {
	if (!current_block)
		create_page();
	void* mem;
	mem = current_block;
	
	current_block = current_block->next_block;
	return mem;
}

void MetaPage::deallocate(void* mem) {
	Block* mem_piece = (Block*)mem;
	mem_piece->next_block = current_block;
	current_block = mem_piece;
}

void MetaPage::create_page() {
	Block* new_page = reinterpret_cast<Block*>(malloc(page_size));

	mem_pages.push_back(new_page);
	
	Block* block = new_page;
	Block* end = (Block*)((byte*)block + page_size - block_size);
	end->next_block = nullptr;

	while (block <= end) {
		Block* next = (Block*)((byte*)block + block_size);
		block->next_block = next;
		block = next;
	}

	current_block = new_page;
}

Meta MetaAllocator::allocate(uint size) {
	
	Meta mem;
	if(size <= max_block_size)
		mem.data = allocator.pages[get_block_index(size)].allocate();
	else {
		mem.data = malloc(size);
	}

	mem.meta_size = size;
	return mem;
}

void MetaAllocator::deallocate(Meta mem) {
	if (mem.meta_size > max_block_size) {
		free(mem.data);
	}
	else {
		allocator.pages[get_block_index(mem.meta_size)].deallocate(mem.data);
	}
}

MetaAllocator MetaAllocator::allocator;

void MetaPool::release_pool(uint index) {
	for (int i = index; i != pool.size(); i++) {
		MetaAllocator::deallocate(pool[i]);
	}
	pool.erase(pool.begin() + index,pool.end());
}