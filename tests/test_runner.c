#include <stdio.h>
#include <assert.h>
#include "test_BPE.h"

// Declare the functions to run dataset and hash table tests
void run_dataset_tests();
void run_hash_table_tests();
void test_BPE();
void test_add_to_vocabulary();
void test_free_tokenizer();
void test_memory_leak();
void test_create_tokenizer_memory_leak();
void test_combined_memory_leak();

int main() {
    printf("Running Dataset Tests...\n");
    run_dataset_tests();

    printf("Running Hash Table Tests...\n");
    run_hash_table_tests();

    printf("Running Free Tokenizer Memory Tests....\n");
    //test_memory_leak();
    //test_create_tokenizer_memory_leak();
    //test_combined_memory_leak();
    //test_free_tokenizer();
    //test_add_to_vocabulary();
    //test_tokenize_functionality();
    //test_tokenize_memory_leaks();
    //test_initialize_vocabulary_memory_leak();
    //test_count_pairs();
    //test_find_most_freq_pairs();
    //test_merge_most_freq_pair();
    test_BPE();
    printf("All tests completed.\n");
    return 0;
}

