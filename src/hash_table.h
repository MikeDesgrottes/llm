#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stddef.h>

// Represents a key-value pair in the hash table
typedef struct {
    char* key;       // Token pair (e.g., "an_a")
    size_t value;    // Frequency of the token pair
} HashEntry;

// Represents the hash table
typedef struct {
    HashEntry** entries;  // Array of pointers to HashEntry
    size_t capacity;      // Total number of slots in the hash table
    size_t size;          // Current number of entries
} HashTable;

// Creates a new hash table with the specified capacity
HashTable* create_hash_table(size_t capacity);

// Frees all memory associated with the hash table
void free_hash_table(HashTable* hash_table);

// Increments the frequency of a given key
void increment_frequency_hash_table(HashTable* hash_table, const char* key);

// Retrieves the frequency of a given key
int get_value(HashTable* hash_table, const char* key, int (cmpfunc)(const char*,const char*));

void resize_hash_table(HashTable* hash_table);

int insert_into_hash_table(HashTable* table, const char* key, size_t value);

void reset_hash_table(HashTable* hash_table);
size_t hash_function(const char* key, size_t capacity);
size_t hash2(const char* key, size_t size);
#endif // HASH_TABLE_H

