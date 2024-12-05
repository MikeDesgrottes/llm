#include <stdio.h>

// Declare the functions to run dataset and hash table tests
void run_dataset_tests();
void run_hash_table_tests();

int main() {
    printf("Running Dataset Tests...\n");
    run_dataset_tests();

    printf("Running Hash Table Tests...\n");
    run_hash_table_tests();

    printf("All tests completed.\n");
    return 0;
}

