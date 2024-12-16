#include "hash_table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "../include/debug.h"

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
    return table;
}

// Frees all memory associated with the hash table
void free_hash_table(HashTable* hash_table) {
    for (size_t i = 0; i < hash_table->capacity; i++) {
        if (hash_table->entries[i]) {
            free(hash_table->entries[i]->key);
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
    size_t index = hash_function(key, hash_table->capacity);
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
        if (strcmp(entry->key, key) == 0) {
            hash_table->entries[probing_index]->value++;
            return;
        }
        step++;
    }
}

void resize_hash_table(HashTable* hash_table) {
    // Double the capacity
    size_t new_capacity = 2 * hash_table->capacity;

    // Allocate memory for the new entries array
    HashEntry** new_entries = calloc(new_capacity, sizeof(HashEntry*));
    if (!new_entries) {
        fprintf(stderr, "Failed to allocate memory during hash table resizing.\n");
        return;
    }

    size_t moved_entries = 0;
    // Rehash all existing entries into the new table
    for (size_t i = 0; i < hash_table->capacity; i++) {
        
	    if (hash_table->entries[i]) {
            	HashEntry* entry = hash_table->entries[i];

            	size_t new_index = hash_function(entry->key, new_capacity);
	    	//size_t new_index1 = hash2(entry->key,new_capacity);
		size_t step = 0;
            	
		// Double hashing  to resolve collisions in the new table
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
	    	fprintf(stderr, "Failed to place entry %s during resize\n", entry->key);
                // Clean up all entries we've moved so far
                for (size_t j = 0; j < new_capacity; j++) {
                    if (new_entries[j] != NULL) {
                        // Don't free the entries themselves as they're still in the old table
                        new_entries[j] = NULL;
                    }
                }
                free(new_entries);
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
}

// Retrieves the frequency of a given key
int get_value(HashTable* hash_table, const char* key, int (cmpfunc)(const char*, const char*)) {
    size_t index = hash_function(key, hash_table->capacity);
    //size_t index1 = hash2(key,hash_table->capacity);

    size_t step = 0;
    while (step < hash_table->capacity) {
	    size_t probing_index = (index + step*step) % hash_table->capacity;
	    HashEntry* entry = hash_table->entries[probing_index];
	    if(entry ==NULL){
	    	return -1;
	    }
        if (entry->key != NULL && cmpfunc((const char* )entry->key, key) == 0) {
            return entry->value;
        }
        step++;
    }

    return -1; // Key not found
}
// This function insert an HashEntry into the hash table given the key. Note that if the key already exist in the hash table, it simply update the entry's value to the new value.
int insert_into_hash_table(HashTable* table, const char* key, size_t value){
	// Check to make sure the load factor isn't too high
	DEBUG_HASH("Insert attempt - Key: %s\n", key);
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
    size_t index = hash_function(key, table->capacity);
    //size_t index1 = hash2(key,table->capacity);
    size_t step = 0;

    // Use double hashing to find an empty slot or existing key
    while (step < table->capacity) {
        size_t probing_index = (index + step*step) % table->capacity;
        HashEntry* entry = table->entries[probing_index];

        if (entry == NULL) {
            // Insert a new entry if the slot is empty
            HashEntry* entry1 = (HashEntry*)malloc(sizeof(HashEntry));
            if (entry1 == NULL) {
                fprintf(stderr, "Error allocating memory for hash table entry\n");
                return -1;  // Error allocating memory
            }

	    entry = entry1;
            entry->key = strdup(key);
            if (entry->key == NULL) {
                fprintf(stderr, "Error allocating memory for hash table key\n");
                free(entry);
                return -1;  // Error allocating memory
            }
            entry->value = value;

            // Assign the new entry to the probing index
            table->entries[probing_index] = entry;
            table->size++;
            return 0;  // Success
        } else if (strcmp(entry->key, key) == 0) {
            // If the key already exists, update its value
            entry->value = value;
            return 0;  // Success
        }

        // Move to the next slot (linear probing)
        step++;
    }

    // If we reach here, the hash table is full
    fprintf(stderr, "Error: Hash table is full, unable to insert key: %s\n Table size is %zu and its capacity is %zu\n", key,table->size, table->capacity);
    return -1;  // Hash table is full
}

void reset_hash_table(HashTable* hash_table) {
    if (hash_table == NULL) {
        return; // Handle NULL gracefully
    }

    // Iterate through all buckets in the hash table
    for (size_t i = 0; i < hash_table->capacity; i++) {
        HashEntry* entry = hash_table->entries[i];
        if (entry != NULL) {
            // Free the key and entry if it exists
            free(entry->key);
            entry->key = NULL;

            // Reset value (optional, depending on your use case)
            entry->value = 0;

            free(entry); // Free the entry itself
            hash_table->entries[i] = NULL;
        }
    }

    // Reset metadata
    hash_table->size = 0;
}
