#include <memory.h>
#include <<stddef.h>
#include <debug.h>
#include <bool.h>


MemoryPool* create_pool(size_t total_size){
	MemoryPool* pool = (MemoryPool*)malloc(sizeof(MemoryPool))
		if(!pool){
			DEBUG_MEM("Error allocating memory for Memory Pool.\n");
			return NULL;
		}

	pool->memory = malloc(total_size);
	if(!pool->memory){
		DEBUG_MEM("Error allocating space for Memory Pool's memory.\n");
		free(pool);
		return NULL;
	}
	pool->size = total_size;

	Block* first_block = (Block*)pool->memory;
    first_block->header.size = total_size - sizeof(Block);
    first_block->header.next = NULL;
    first_block->is_free = true;

    // Step 4: Set up free list
    pool->free = first_block;

}
void destroy_pool(MemoryPool* pool) {
    if (!pool) return;

    if (pool->memory) {
        free(pool->memory);
        pool->memory = NULL;
    }

    free(pool);
}

void* pool_alloc(MemoryPool* pool, size_t size){
	if(pool == null || pool->memory = NULL){
		DEBUG_MEM("Error Invalid parameters for pool allocation.\n");
		return NULL;
	}

	if(pool->free == NULL){
		DEBUG_MEM("Error memory exhausted.\n");
		return NULL;
	}

	Block* current = (Block*)pool->free;
	Block* prev = NULL;
	while(current != NULL){
		if(current->is_free && current->header.size >= size){
			if(current->header.size >= 2*size + sizeof(Block)){
                		split_block(current,size);
        		}

			if(prev == NULL){
				pool->free = current->header.next;
			}else{
				prev->header.next = current->header.next;
			}

			cuurent->is_free = false;
			char* data = (char*)current + sizeof(Block);
			return (void*)data;
		}
		prev = current;
		current = current->header.next;
	}
	return NULL;
}

void split_block(Block* block, size_t size){
	if(block == NULL || !block->is_free) return;

	if(block->header.size < sizeof(Block) + size){
		DEBUG_MEM("Not enough space to split the block.\n");
		return;
	}

	char* start_of_second = (char*)block + sizeof(Block) + size;
	Block* block2 = (Block*)start_of_second;
	block2->header.size = block->header.size - size - sizeof(Block);
	block2->is_free = true;
	block2->header.next = block->header.next;

	block->header.next = block2;
	block->header.size = size;

}

bool is_address_in_pool(MemoryPool* pool, void* ptr){
	if(pool == NULL || ptr == NULL){
		DEBUG_MEM("Error invalid paramters.\n");
		return false;
	}

	void* start = pool->memory;
	void* end = (char*)pool->memory + pool->size;

	return (ptr >= start && ptr <= end);
}

void pool_free(MemoryPool* pool, void* ptr){
	if(pool == NULL || ptr == NULL) return;

	if(!is_address_in_pool(pool,ptr)){
		DEBUG_MEM("Pointer address is not in memory pool.\n");
		return;
	}

	Block* block = (Block*)((char*)ptr - offsetof(Block,data));
	if(block->is_free){
		DEBUG_MEM("Error memory is already freed.\n");
		return;
	}
	block->is_free = true;
	block->header.next = pool->free;
	pool->free = block;


}

bool coalesce_free_blocks(MemoryPool* pool){
	if(pool == NULL || pool->free == NULL) return false;

	bool did_coalesce = false;
	Block* current = (Block*)pool->free;

	while(current != NULL && current->header.next != NULL){
		void* block_end = (char*)current + offsetof(Block,data) + current->header.size;

		if(block_end == (void*)current->header.next && current->is_free && current->header.next->is_free){
			current->header.size += current->header.next->header.size + offsetof(Block,data);
            		current->header.next = current->header.next->header.next;
            		did_coalesce = true;
		}else{
			current = current->header.next;
		}
	}

	return did_coalesce;

}
