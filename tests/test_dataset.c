#include <assert.h>
#include "../src/dataset.h"

void test_create_dataset() {
    Dataset* dataset = initialize_dataset(10);
    assert(dataset != NULL);
    assert(dataset->num_lines == 0);
    assert(dataset->capacity == 10);
    free_dataset(dataset);
}

void test_add_line() {
    Dataset* dataset = initialize_dataset(10);
    assert(add_line(dataset, "Line1") == 0);
    assert(dataset->num_lines == 1);
    assert(strcmp(dataset->lines[0], "Line1") == 0);
    free_dataset(dataset);
}

// Other dataset tests here...

void run_dataset_tests() {
    test_create_dataset();
    test_add_line();
    // Call other dataset test functions...
}

