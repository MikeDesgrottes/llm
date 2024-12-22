#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stddef.h>

// Core Operations that can be customized for different data types
typedef struct HashOperations {
    // Creates a hash value from a key
    size_t (*hash_function)(const void* key, size_t key_size);
    
    // Compares two keys for equality
    int (*compare_keys)(const void* key1, const void* key2);
    
    // Creates copies of keys and values
    void* (*duplicate_key)(const void* key);
    void* (*duplicate_value)(const void* value);
    
    // Frees memory for keys and values
    void (*free_key)(void* key);
    void (*free_value)(void* value);
    
    // For debugging and visualization
    void (*print_key)(const void* key);
    void (*print_value)(const void* value);
} HashOperations;

// Each entry in our hash table
typedef struct HashEntry {
    void* key;           // Generic key
    void* value;         // Generic value
    size_t key_size;     // Size of key in bytes
    size_t value_size;   // Size of value in bytes
    bool is_occupied;    // Flag for quadratic probing
} HashEntry;

// The main hash table structure
typedef struct HashTable {
    HashEntry** entries;      // Array of entries
    size_t size;             // Current number of items
    size_t capacity;         // Total capacity
    HashOperations ops;      // Operations for this table
    float load_factor;       // When to resize
    bool allow_resize;       // Whether to allow automatic resizing
} HashTable;

// Iterator structure
//
typedef struct {
    const HashTable* table;    // Reference to the hash table
    size_t current_index;      // Current position
    size_t items_returned;     // Number of items iterated over
} HashTableIterator;

// Iterator functions
//
HashTableIterator* create_iterator(const HashTable* table);
bool has_next(HashTableIterator* iterator);
HashEntry* get_next(HashTableIterator* iterator);
void free_iterator(HashTableIterator* iterator);
// Memory Management
//
HashEntry* create_hash_entry(const void* key, size_t key_size, const void* value, size_t value_size);
void free_hash_entry(HashEntry* entry);
// Creates a new hash table with the specified capacity
HashTable* create_hash_table(size_t capacity,HashOperations ops, size_t key_size, size_t value_size);

// Frees all memory associated with the hash table
void free_hash_table(HashTable* hash_table);

// Increments the frequency of a given key
void increment_frequency_hash_table(HashTable* hash_table, const char* key);

// Retrieves the frequency of a given key
int get_value(HashTable* hash_table, const void* key, void* dest);

void resize_hash_table(HashTable* hash_table);

int insert_into_hash_table(HashTable* table, const void* key, const void* value);

bool validate_ops_func(HashTable* table);

void reset_hash_table(HashTable* hash_table);
size_t hash_function(const char* key, size_t capacity);
size_t hash2(const char* key, size_t size);
#endif // HASH_TABLE_H

