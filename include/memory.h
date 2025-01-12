#ifndef MEMORY_H
#define MEMORY_H
#include <bool.h>
#include <stdlib.h>


typedef struct{
	size_t size;
	Block* next;


}BlockHeader;

typedef struct{
	BlockHeader header;
	bool is_free;
	unsigned char data[];
}Block;

typedef struct{
	void* memory;
	Block* free;
	size_t size;
}MemoryPool;

void memory_pool_init(MemoryPool* pool);
MemoryPool* create_pool(size_t total_size);
void destroy_pool(MemoryPool* pool);
void reset_pool(MemoryPool* pool);

void* pool_alloc(MemoryPool* pool, size_t size);
void pool_free(MemoryPool* pool, void* ptr);
bool coalesce_free_blocks(MemoryPool* pool);

Block* find_free_block(MemoryPool* pool, size_t size);
void split_block(Block* block, size_t size);  // Split if requested size < block size
bool is_address_in_pool(MemoryPool* pool, void* ptr);
size_t get_block_size(Block* block);


#endif
