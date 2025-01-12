#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <hash_table.h>

void test_free_hash_table_memory_leak();

void test_create_hash_table() {
    HashTable* hash_table = create_hash_table(10);
    assert(hash_table != NULL);
    assert(hash_table->capacity == 10);
    assert(hash_table->size == 0);
    free_hash_table(hash_table);
}

void test_increment_frequency() {
    HashTable* hash_table = create_hash_table(10);
    const char* key = "key1";
    size_t value = 10;
    insert_into_hash_table(hash_table, (const void*)key,(const void*)&value, strlen(key) + 1,sizeof(size_t));
    insert_into_hash_table(hash_table,(const void*)key,(const void*)&value,strlen(key) + 1,sizeof(size_t) );
    free_hash_table(hash_table);
}

// Other hash table tests here...

void run_hash_table_tests() {
    test_create_hash_table();
    test_increment_frequency();
    test_free_hash_table_memory_leak();
    // Call other hash table test functions...
}

void test_free_hash_table_memory_leak() {
    fprintf(stderr, "\nTesting free_hash_table memory management...\n");

    // Step 1: Create a hash table
    HashTable* table = create_hash_table(10);
    if (!table) {
        fprintf(stderr, "Failed to create hash table\n");
        return;
    }
    fprintf(stderr, "Hash table created: %p\n", (void*)table);

    // Step 2: Add some entries to the hash table
    char* key1 = strdup("key1");
    char* key2 = strdup("key2");
    size_t value1 = 13;
    size_t  value2 = 14;

    if (!key1 || !key2) {
        fprintf(stderr, "Failed to allocate memory for keys or values\n");
        free(key1);
        free(key2);      
        free_hash_table(table);
        return;
    }

    insert_into_hash_table(table, (const void*) key1, (const void*)&value1,strlen(key1) + 1, sizeof(size_t));
    insert_into_hash_table(table, (const void*)key2, (const void*)&value2,strlen(key2) + 1,sizeof(size_t));
    free(key1);
    free(key2);
    fprintf(stderr, "Inserted entries into hash table.\n");

    // Step 3: Free the hash table
    free_hash_table(table);
    fprintf(stderr, "Hash table successfully freed.\n");
}
