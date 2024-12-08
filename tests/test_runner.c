#include <stdio.h>

// Declare the functions to run dataset and hash table tests
void run_dataset_tests();
void run_hash_table_tests();
void test_BPE();
void test_add_to_vocabulary();

int main() {
    printf("Running Dataset Tests...\n");
    run_dataset_tests();

    printf("Running Hash Table Tests...\n");
    run_hash_table_tests();

    printf("Running BPE tests....\n");
    test_add_to_vocabulary();
    test_BPE();
    printf("All tests completed.\n");
    return 0;
}

