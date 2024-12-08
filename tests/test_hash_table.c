#include <assert.h>
#include <string.h>
#include "../src/hash_table.h"

void test_create_hash_table() {
    HashTable* hash_table = create_hash_table(10);
    assert(hash_table != NULL);
    assert(hash_table->capacity == 10);
    assert(hash_table->size == 0);
    free_hash_table(hash_table);
}

void test_increment_frequency() {
    HashTable* hash_table = create_hash_table(10);
    increment_frequency_hash_table(hash_table, "key1");
    assert(get_value(hash_table, "key1", strcmp) == 1);
    free_hash_table(hash_table);
}

// Other hash table tests here...

void run_hash_table_tests() {
    test_create_hash_table();
    test_increment_frequency();
    // Call other hash table test functions...
}

