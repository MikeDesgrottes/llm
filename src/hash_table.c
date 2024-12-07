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
int get_value(HashTable* hash_table, const char* key, int (cmpfunc)(const char*, const char*)) {
    size_t index = hash_function(key, hash_table->capacity);
    size_t step = 0;
    while (step < hash_table->capacity) {
	    size_t probing_index = (index + step) % hash_table->capacity;
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

int insert_into_hash_table(HashTable* table, const char* key, size_t value){
	// Compute the initial hash index
    size_t index = hash_function(key, hash_table->capacity);
    size_t step = 0;

    // Use linear probing to find an empty slot or existing key
    while (step < hash_table->capacity) {
        size_t probing_index = (index + step) % hash_table->capacity;
        HashEntry* entry = hash_table->entries[probing_index];

        if (entry == NULL) {
            // Insert a new entry if the slot is empty
            entry = (HashEntry*)malloc(sizeof(HashEntry));
            if (entry == NULL) {
                fprintf(stderr, "Error allocating memory for hash table entry\n");
                return -1;  // Error allocating memory
            }
            entry->key = strdup(key);
            if (entry->key == NULL) {
                fprintf(stderr, "Error allocating memory for hash table key\n");
                free(entry);
                return -1;  // Error allocating memory
            }
            entry->value = value;

            // Assign the new entry to the probing index
            hash_table->entries[probing_index] = entry;
            hash_table->size++;
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
    fprintf(stderr, "Error: Hash table is full, unable to insert key: %s\n", key);
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
