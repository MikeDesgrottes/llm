#include "hash_table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// A simple hash function for strings
static size_t hash_function(const char* key, size_t capacity) {
    size_t hash = 0;
    while (*key) {
        hash = (hash * 31 + *key) % capacity; // Multiply by a prime number
        key++;
    }
    return hash;
}

// Creates a new hash table with the specified capacity
HashTable* create_hash_table(size_t capacity) {
    HashTable* table = malloc(sizeof(HashTable));
    if (!table) return NULL;

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
void increment_frequency(HashTable* hash_table, const char* key) {
	if ((float)hash_table->size / hash_table->capacity > 0.7) {
        resize_hash_table(hash_table);
    }
    size_t index = hash_function(key, hash_table->capacity);

    // Linear probing for collision resolution
    while (hash_table->entries[index] != NULL) {
        if (strcmp(hash_table->entries[index]->key, key) == 0) {
            hash_table->entries[index]->value++;
            return;
        }
        index = (index + 1) % hash_table->capacity; // Move to next slot
    }

    // Add a new entry
    HashEntry* new_entry = malloc(sizeof(HashEntry));
    new_entry->key = strdup(key); // Duplicate the key
    new_entry->value = 1;
    hash_table->entries[index] = new_entry;
    hash_table->size++;
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

    // Rehash all existing entries into the new table
    for (size_t i = 0; i < hash_table->capacity; i++) {
        if (hash_table->entries[i]) {
            HashEntry* entry = hash_table->entries[i];
            size_t new_index = hash_function(entry->key, new_capacity);

            // Linear probing to resolve collisions in the new table
            while (new_entries[new_index] != NULL) {
                new_index = (new_index + 1) % new_capacity;
            }

            new_entries[new_index] = entry; // Move entry to the new table
        }
    }

    // Free the old entries array (entries themselves remain intact)
    free(hash_table->entries);

    // Update hash table properties
    hash_table->entries = new_entries;
    hash_table->capacity = new_capacity;
}

// Retrieves the frequency of a given key
size_t get_frequency(HashTable* hash_table, const char* key, int (cmpfunc)(const char*, const char*)) {
    size_t index = hash_function(key, hash_table->capacity);
    size_t step = 0;
    while (step < hash_table->capacity) {
	    size_t probing_index = (index + step) % hash_table->capacity;
	    HashEntry* entry = hash_table->entries[probing_index];
	    if(entry ==NULL){
	    	return 0;
	    }
        if (entry->key != NULL && cmpfunc((const char* )entry->key, key) == 0) {
            return entry->value;
        }
        step++;
    }

    return 0; // Key not found
}

