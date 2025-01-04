#include <assert.h>
#include "../src/dataset.h"

void test_create_dataset() {
    Dataset* dataset = create_dataset(10);
    assert(dataset != NULL);
    assert(dataset->num_categories == 0);
    assert(dataset->capacity == 10);
    free_dataset(dataset);
}

void test_load_from_dataset(){
	Dataset* dataset = create_dataset(10);
	if(load_dataset_from_directory(dataset,"/mnt/data/llm/datasets") == 0){
		assert(dataset != NULL);
		assert(dataset->num_categories == 1);

	}	
	free_dataset(dataset);
}


// Other dataset tests here...

void run_dataset_tests() {
    test_create_dataset();
    test_load_from_dataset();
    // Call other dataset test functions...
}
