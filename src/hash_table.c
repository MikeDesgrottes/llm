#include "hash_table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <debug.h>

#define HASH_SEED_1 0xc70f6907    // For primary hash
#define HASH_SEED_2 0x8e055be3    // For secondary hash
static inline uint32_t murmur_32_scramble(uint32_t k) {
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    return k;
}
uint32_t murmur3_32(const uint8_t* key, size_t len, uint32_t seed)
{
	uint32_t h = seed;
    uint32_t k;
    /* Read in groups of 4. */
    for (size_t i = len >> 2; i; i--) {
        // Here is a source of differing results across endiannesses.
        // A swap here has no effects on hash properties though.
        memcpy(&k, key, sizeof(uint32_t));
        key += sizeof(uint32_t);
        h ^= murmur_32_scramble(k);
        h = (h << 13) | (h >> 19);
        h = h * 5 + 0xe6546b64;
    }
    /* Read the rest. */
    k = 0;
    for (size_t i = len & 3; i; i--) {
        k <<= 8;
        k |= key[i - 1];
    }
    // A swap is *not* necessary here because the preceding loop already
    // places the low bytes in the low places according to whatever endianness
    // we use. Swaps only apply when the memory is copied in a chunk.
    h ^= murmur_32_scramble(k);
    /* Finalize. */
	h ^= len;
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
	return h;
}
// A implementation of FNV Hash function for strings given the capacity. 
size_t hash_function(const char* key, size_t capacity) {
    return murmur3_32((const uint8_t*)key, strlen(key), HASH_SEED_1) % capacity;
}

size_t hash2(const char* key, size_t size) {
	return (1 + murmur3_32((const uint8_t*) key, strlen(key), HASH_SEED_2) % (size - 1));
}

// Memory Management:

HashEntry* create_hash_entry(const void* key, size_t key_size, const void* value, size_t value_size){
	HashEntry* entry = malloc(sizeof(HashEntry));
	if(!entry){
		fprintf(stderr,"Error while allocating memory for HashEntry.\n");
		return NULL;
	}

	void* tmp_key = malloc(key_size);
	if(!tmp_key){
		free(entry);
		fprintf(stderr,"Error while allocating memory for HashEntry key.\n");
		return NULL;
	}

	entry->key = tmp_key;

	void* tmp_value = malloc(value_size);
	if(!tmp_value){
		free(entry->key);
		free(entry);
		fprintf(stderr,"Error while allocating memory for HashEntry value.\n");
		return NULL;
	}

	entry->value = tmp_value;
	memcpy(entry->key, key, key_size);
	memcpy(entry->value, value, value_size);

	entry->key_size = key_size;
	entry->value_size = value_size;
	entry->is_occupied = true;
	return entry;
}


// Creates a new hash table with the specified capacity. This is the main function that is responsible for allocating memory
// for new hash table. it returns a pointer to the structure giving up authority to whoever called the function. Note that you are responsible for freeing the memory after usage.
HashTable* create_hash_table(size_t capacity) {

	// Allocate enough memory for a table
    HashTable* table = malloc(sizeof(HashTable));
    if (!table) return NULL;

	// Note that entries of the hash table has been initialized to NULL.
    table->entries = calloc(capacity, sizeof(HashEntry*));
    if (!table->entries) {
        free(table);
        return NULL;
    }
    table->capacity = capacity;
    table->size = 0;
    create_standard_ops(table);
    table->load_factor = 0;
    return table;
}

// Frees all memory associated with the hash table
void free_hash_table(HashTable* hash_table) {
	if(!hash_table){
		return;// do nothing
	}
    for (size_t i = 0; i < hash_table->capacity; i++) {
        if (hash_table->entries[i]&& hash_table->entries[i]->is_occupied) {
            hash_table->ops.free_key(hash_table->entries[i]->key);
	    hash_table->ops.free_value(hash_table->entries[i]->value);
            free(hash_table->entries[i]);
        }
    }
    free(hash_table->entries);
    free(hash_table);
}

// Increments the frequency of a given key
void increment_frequency_hash_table(HashTable* hash_table, const char* key) {
	if ((float)hash_table->size / hash_table->capacity > 0.7) {
        resize_hash_table(hash_table);
    }
    size_t index = hash_table->ops.hash_function((const void*)key) % hash_table->capacity;
    //size_t index1 = hash2(key,hash_table->capacity);

    size_t step = 0;
    // double hashing for collision resolution
    while (step < hash_table->capacity) {
	    size_t probing_index = (index + step*step) % hash_table->capacity;
	    HashEntry* entry = hash_table->entries[probing_index];
	    if(entry == NULL){
		    fprintf(stderr, "Error: Key '%s' not found in hash table\n", key);
	    	return; // the key is not there.
	    }
        if (hash_table->ops.compare_keys(entry->key, key) == 0) {
            size_t* ptr = (size_t*)hash_table->entries[probing_index]->value;
	    (*ptr)++;
            return;
        }
        step++;
    }
}

void resize_hash_table(HashTable* hash_table) {

	if(!hash_table || !hash_table->allow_resize || !hash_table->entries){
		DEBUG_HASH("Error invalid table or resize is not allowed in this table.\n");
		return;
	}
    	// Double the capacity
    	size_t new_capacity = 2 * hash_table->capacity;

    	// Allocate memory for the new entries array
    	HashEntry** new_entries = calloc(new_capacity, sizeof(HashEntry*));
    	if (!new_entries) {
        	fprintf(stderr, "Failed to allocate memory during hash table resizing.\n");
        	return;
    	}

    	size_t moved_entries = 0;
	HashTableIterator* it = create_iterator(hash_table);
	if(!it){
		free(new_entries);
		return;
	}
    	// Rehash all existing entries into the new table
    	while(has_next(it) == true) {
        	HashEntry* entry = get_next(it);
	    	if (entry != NULL) {
            		size_t new_index = hash_table->ops.hash_function(entry->key) % new_capacity;
	    		//size_t new_index1 = hash2(entry->key,new_capacity);
			size_t step = 0;
            	
			// Quadratic probing  to resolve collisions in the new table
            		while (step < new_capacity) {
    	        		size_t probing_index = (new_index + step*step) % new_capacity;
        			if(new_entries[probing_index] == NULL){
					new_entries[probing_index] = entry;
					moved_entries++;
					break;
				}
        			step++;
    	     		} // Move entry to the new table

	    		if(step == new_capacity){
	    			fprintf(stderr, "Failed to place entry %zu during resize\n",it->current_index);
                		// Clean up all entries we've moved so far
                		for (size_t j = 0; j < new_capacity; j++) {
                    			if (new_entries[j] != NULL) {
                        			// Don't free the entries themselves as they're still in the old table
                        			new_entries[j] = NULL;
                    			}
                		}
                		free(new_entries);
				free_iterator(it);
				hash_table->size = (size_t)(hash_table->capacity * 0.75);  // Force another resize soon
                		return;
	    		}
        	}
    	}	

    	printf("Successfully moved %zu entries to new table\n", moved_entries);
    	// Free the old entries array (entries themselves remain intact)
    	free(hash_table->entries);

    	// Update hash table properties
    	hash_table->entries = new_entries;
    	hash_table->capacity = new_capacity;
    	hash_table->load_factor = (float) hash_table->size / hash_table->capacity ;
	free_iterator(it);
}

bool validate_ops_func(HashTable* table){
	if(!table || !table->ops.hash_function || !table->ops.compare_keys || !table->ops.free_key || !table->ops.free_value) return false;

	return true;
}
// Retrieves the frequency of a given key
int get_value(HashTable* hash_table, const void* key, void* dest) {
	if(!hash_table || !key ) return -1;
	if(!validate_ops_func(hash_table)) return -1;
    size_t index =hash_table->ops.hash_function(key) % hash_table->capacity;
    //size_t index1 = hash2(key,hash_table->capacity);

    size_t step = 0;
    while (step < hash_table->capacity) {
	    size_t probing_index = (index + step*step) % hash_table->capacity;
	    HashEntry* entry = hash_table->entries[probing_index];
	    if(entry ==NULL){
	    	return -1;
	    }

        if (entry->key != NULL && hash_table->ops.compare_keys(entry->key, key) == 0) {
		memcpy(dest,entry->value, entry->value_size);
            return 0;
        }
        step++;
    }

    return -1; // Key not found
}
// This function insert an HashEntry into the hash table given the key. Note that if the key already exist in the hash table, it simply update the entry's value to the new value.
int insert_into_hash_table(HashTable* table, const void* key, const void* value, size_t key_size, size_t value_size){

	if(table == NULL || key == NULL || value == NULL){
		DEBUG_HASH("Error invalid table, key or value\n");
		return -1;
	}

	if(key_size == 0 || value_size == 0){
                        fprintf(stderr, "Error: Invalid key size or value size\n");
                        return -1;
                }

	if ((float)table->size / table->capacity > 0.7 && table->allow_resize) {
        resize_hash_table(table);
    }
	// Check to make sure the load factor isn't too high
	if(table->ops.print_key){
		DEBUG_HASH("Insert attempt - Key: "); table->ops.print_key(key, stderr); fprintf(stderr,"\n");
	}
    	DEBUG_HASH("Current state - Size: %zu, Capacity: %zu\n", table->size, table->capacity);
    	float load_factor = (float)table->size / table->capacity;
    	DEBUG_HASH("Load factor: %f\n", load_factor);
    
    // Check resize condition
    if(load_factor > 0.7){
        DEBUG_HASH("Triggering resize at load factor %f\n", load_factor);
        resize_hash_table(table);
        DEBUG_HASH("After resize - New capacity: %zu\n", table->capacity);
    }	
	// Compute the initial hash index
    size_t index = table->ops.hash_function(key) % table->capacity;
    //size_t index1 = hash2(key,table->capacity);
    size_t step = 0;

    // Use double hashing to find an empty slot or existing key
    while (step < table->capacity) {
        size_t probing_index = (index + step*step) % table->capacity;
        HashEntry* entry = table->entries[probing_index];

        if (entry == NULL) {
		
		HashEntry* entry1 = create_hash_entry(key,key_size,value,value_size);
		if(!entry1){
			DEBUG_HASH("Error creating hash Entry with key: ");
			if(table->ops.print_key){
				table->ops.print_key(key, stderr);
			}
			DEBUG_HASH(" with value: ");
			if(table->ops.print_value){
				table->ops.print_value(value,stderr);
			}
			fprintf(stderr,"\n");
			return -1;
		}
            // Assign the new entry to the probing index
            table->entries[probing_index] = entry1;
            table->size++;
	    table->load_factor = (float) table->size / table->capacity;
            return 0;  // Success
        } else if (table->ops.compare_keys(entry->key, key) == 0) {
            // If the key already exists, update its value
            //entry->value += value;
	    if(table->ops.free_value){
	    	table->ops.free_value(entry->value);
		void* tmp = malloc(value_size);
		if(!tmp){
			DEBUG_HASH("Error while creating a value.\n");
			return -1;
		}

		entry->value = tmp;
		memcpy(entry->value, value, value_size);
	    }else{
	    	DEBUG_HASH("Need to provide cleaner function for HashEntries values.\n");
	    }
            return 0;  // Success
        }

        // Move to the next slot (linear probing)
        step++;
    }

    // If we reach here, the hash table is full
    fprintf(stderr, "Error: Hash table is full, unable to insert key: ");
    if(table->ops.print_key){
                                table->ops.print_key(key,stderr);
                        }
    fprintf(stderr,"\n Table size is %zu and its capacity is %zu\n",table->size, table->capacity);
    return -1;  // Hash table is full
}

void reset_hash_table(HashTable* hash_table) {
    if (hash_table == NULL || !hash_table->ops.free_key || !hash_table->ops.free_value) {
        return; // Handle NULL gracefully
    }

    // Iterate through all buckets in the hash table
    for (size_t i = 0; i < hash_table->capacity; i++) {
        HashEntry* entry = hash_table->entries[i];
        if (entry != NULL) {
            // Free the key and entry if it exists
            hash_table->ops.free_key(entry->key);
            entry->key = NULL;

            // Reset value (optional, depending on your use case)
	    hash_table->ops.free_value(entry->value);

            entry->value = NULL;

            free(entry); // Free the entry itself
            hash_table->entries[i] = NULL;
        }
    }

    // Reset metadata
    hash_table->size = 0;
    hash_table->load_factor = 0.0f;
}


// Iterator Implemnetation
//

HashTableIterator* create_iterator(const HashTable* table){
	if(!table){
		DEBUG_HASH("Error: Invalid table.\n");
		return NULL;
	}

	HashTableIterator* iter = malloc(sizeof(HashTableIterator));
	if(!iter){
		DEBUG_HASH("Error: Could not allocate enough memory for new iterator.\n");
		return NULL;
	}

	iter->table = table;
	iter->current_index = 0;
	iter->items_returned = 0;
	return  iter;
}

bool has_next(HashTableIterator* iterator){

	if(!iterator) exit(0);
	if(iterator->items_returned >= iterator->table->size) return false;

	return true;
}

HashEntry* get_next(HashTableIterator* iterator){
	if(!iterator || has_next(iterator) == false) return NULL;

	for(size_t i = iterator->current_index + 1; i < iterator->table->capacity; i++){
		HashEntry* entry = iterator->table->entries[i];
		if(entry && entry->is_occupied == true){
			iterator->items_returned++;
			iterator->current_index = i;
			return entry;
		}
	}
	return NULL;
}

void free_iterator(HashTableIterator* iterator) {
    if(iterator) {
        free(iterator);
    }
}

size_t string_hash(const void* key){
	const char* str = (const char*)key;
	return murmur3_32((const uint8_t*)str, strlen(str), HASH_SEED_1);
}

int string_compare(const void* key1, const void* key2){
	if(!key1 || !key2){
		return -1;
	}

	const char* str1 = (const char*)key1;
	const char* str2 = (const char*)key2;

	return strcmp(str1,str2);
}

void* duplicate_value(const void* value){
	if(!value) return NULL;

	size_t* ret = malloc(sizeof(size_t));
	if(!ret) return NULL;
	memcpy(ret, value, sizeof(size_t));
	return ret;
}
void* string_duplicate(const void* key){
	if(!key) return NULL;

	char* tmp =  strdup((const char*)key);
	if(!tmp) return NULL;
	return tmp;
}

void string_print(const void* key,FILE* stream){
	if(!key) return;
	fprintf(stream,"%s",(const char*)key);
}

void int_print(const void* value, FILE* stream){
	if(!value) return;
	fprintf(stream,"%zu",(size_t) value);
}
void string_free(void* key){
	free(key);
}

void create_standard_ops(HashTable* table){
	if(!table) return;

	table->ops.hash_function = string_hash;
	table->ops.compare_keys = string_compare;
	table->ops.duplicate_key = string_duplicate;
	table->ops.duplicate_value = duplicate_value;
	table->ops.print_key = string_print;
	table->ops.print_value = int_print;
	table->ops.free_key = string_free;
	table->ops.free_value = free;
}
