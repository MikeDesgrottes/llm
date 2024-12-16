#include <stdio.h>
#include <stdlib.h>
#include "../src/tokenizer.h"
#include "../src/dataset.h"
#include "../include/config.h"
#include "../include/debug.h"
#include "../src/utils.h"
#include <assert.h>

void print_vocabulary(Tokenizer* tokenizer, size_t* valid_count);

// Helper function to print tokenized data
void print_tokenized_data(Token** tokenized_data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        printf("%s ", tokenized_data[i]->text);
    }
    printf("\n");
}

void test_BPE() {
    printf("\nRunning BPE Tests...\n");
	size_t count = 0;
    // Edge Case 1: Empty dataset
    Dataset* empty_dataset = initialize_dataset(0);
    Tokenizer* empty_tokenizer = create_tokenizer(10);
    BPE(empty_tokenizer, empty_dataset);
    printf("\nEdge Case 1: Empty Dataset\n");
    print_vocabulary(empty_tokenizer,&count);

    free_tokenizer(&empty_tokenizer);
    free_dataset(empty_dataset);

    // Edge Case 2: Single-line dataset
    Dataset* single_line_dataset = initialize_dataset(1);
    add_line(single_line_dataset, "a b c d");
    Tokenizer* single_line_tokenizer = create_tokenizer(10);
    BPE(single_line_tokenizer, single_line_dataset);
    
    printf("\nEdge Case 2: Single-line Dataset\n");
    print_vocabulary(single_line_tokenizer,&count);

    free_tokenizer(&single_line_tokenizer);
    free_dataset(single_line_dataset);

    // General Case: Larger dataset with varied patterns and multi-character words
    Dataset* general_dataset = initialize_dataset(5);
    add_line(general_dataset, "cat dog bird dog cat bird");
    add_line(general_dataset, "apple orange banana apple orange banana");
    add_line(general_dataset, "blue red green blue red green");
    add_line(general_dataset, "happy sad joyful happy sad joyful");
    add_line(general_dataset, "dog apple cat orange bird banana");

    Tokenizer* general_tokenizer = create_tokenizer(20);
    BPE(general_tokenizer, general_dataset);

    printf("\nGeneral Case: Larger Dataset with Varied Patterns\n");
    print_vocabulary(general_tokenizer,&count);

    // Validate the tokenized dataset after BPE
    size_t num_tokens = 0;
    Token** tokenized_data = tokenize(general_dataset, "", &num_tokens);
    printf("\nFinal Tokenized Dataset:\n");
    print_tokenized_data(tokenized_data, num_tokens);

    // Cleanup
    free_tokenizer(&general_tokenizer);
    free_dataset(general_dataset);
    for (size_t i = 0; i < num_tokens; i++) {
        free_token(tokenized_data[i]);
    }
    free(tokenized_data);

    printf("BPE Tests Completed.\n");
}


void print_vocabulary(Tokenizer* tokenizer, size_t* valid_count) {
	size_t tmp_count = 0;
    printf("Vocabulary (size: %zu, max size: %zu):\n", tokenizer->vocab_size, tokenizer->max_vocab_size);
    for (size_t i = 0; i < tokenizer->token_map->size; i++) {
        HashEntry* entry = tokenizer->token_map->entries[i];
	if(entry != NULL){
		if(entry->key != NULL){
			printf("  Token %zu: %s (freq: %zu)\n", entry->value, tokenizer->vocabulary[entry->value]->text, tokenizer->vocabulary[entry->value]->frequency);
			tmp_count++;
		}
        }
    }
    *valid_count = tmp_count;
}
void check_token_map(Tokenizer* tokenizer) {
    printf("\nChecking token_map consistency...\n");
    for (size_t i = 0; i < tokenizer->max_vocab_size; i++) {
        if (tokenizer->vocabulary[i] != NULL) {
            int index = get_value(tokenizer->token_map, tokenizer->vocabulary[i]->text, strcmp);
            if (index != i) {
                printf("Error: token_map mismatch for token '%s'. Expected %zu, got %d\n",
                    tokenizer->vocabulary[i]->text, i, index);
            } else {
                printf("Token '%s' correctly mapped to index %zu\n", tokenizer->vocabulary[i]->text, i);
            }
        }
    }
    printf("Token map consistency check complete.\n");
}
void test_add_to_vocabulary() {
    printf("\nRunning test_add_to_vocabulary...\n");

    // Step 1: Initialize a tokenizer
    size_t max_vocab_size = 5;
    Tokenizer* tokenizer = create_tokenizer(max_vocab_size);

    // Step 2: Add normal tokens
    add_to_vocabulary(tokenizer, "hello");
    add_to_vocabulary(tokenizer, "world");
    add_to_vocabulary(tokenizer, "test");
    add_to_vocabulary(tokenizer, "token");

    // Step 3: Add duplicate token
    add_to_vocabulary(tokenizer, "hello"); // Frequency should increase

    // Step 4: Add edge case: NULL token
    add_to_vocabulary(tokenizer, NULL); // Should handle gracefully and ignore

    // Step 5: Add token exceeding max_vocab_size
    add_to_vocabulary(tokenizer, "extra");

    // Step 6: Validate results
    size_t count = 0;
    print_vocabulary(tokenizer,&count);

    // Assertions
    if (count != tokenizer->vocab_size) {
        fprintf(stderr, "Error: Expected valid tokens to match vocab size. Found %zu, expected %zu\n", count, tokenizer->vocab_size);
    }
    // Check specific tokens
    for (size_t i = 0; i < tokenizer->max_vocab_size; i++) {
        if (tokenizer->vocabulary[i] != NULL && strcmp(tokenizer->vocabulary[i]->text, "hello") == 0) {
            if (tokenizer->vocabulary[i]->frequency != 2) {
                fprintf(stderr, "Error: 'hello' token frequency mismatch. Expected 2, got %zu\n", tokenizer->vocabulary[i]->frequency);
            }
        }
    }

    check_token_map(tokenizer);
        // Cleanup
    free_tokenizer(&tokenizer);

    printf("test_add_to_vocabulary completed.\n");
}

void test_free_tokenizer() {
    Tokenizer* tokenizer = create_tokenizer(3);

    // Add tokens to the tokenizer
    add_to_vocabulary(tokenizer, "hello");
    add_to_vocabulary(tokenizer, "world");
    add_to_vocabulary(tokenizer, "token");

    // Free the tokenizer
    free_tokenizer(&tokenizer);
    // Verify that everything was freed
    //assert(tokenizer == NULL); // Optional: Change the function to use a double pointer
}
void test_memory_leak() {
    //Tokenizer* tokenizer = create_tokenizer(10);
    Token* token1 = create_token("hello");
    Token* token2 = create_token("world");
	free_token(token1);
	free_token(token2);
    //add_to_vocabulary(tokenizer, (const char*)token1->text);
    //add_to_vocabulary(tokenizer, (const char*)token2->text);

    //free_tokenizer(&tokenizer);
}

void test_create_tokenizer_memory_leak() {
    fprintf(stderr, "\nTesting create_tokenizer memory management...\n");

    // Create a tokenizer
    Tokenizer* tokenizer = create_tokenizer(100);
    if (!tokenizer) {
        fprintf(stderr, "Failed to create tokenizer\n");
        return;
    }

    // Verify tokenizer and fields are allocated
    fprintf(stderr, "Tokenizer created: %p\n", (void*)tokenizer);
    fprintf(stderr, "Vocabulary allocated: %p\n", (void*)tokenizer->vocabulary);
    fprintf(stderr, "Pair frequencies hash table allocated: %p\n", (void*)tokenizer->pair_freqs);
    fprintf(stderr, "Token map hash table allocated: %p\n", (void*)tokenizer->token_map);

    // Free the tokenizer
    free_tokenizer(&tokenizer);

    // Verify tokenizer is nullified
    if (tokenizer == NULL) {
        fprintf(stderr, "Tokenizer successfully freed and nullified.\n");
    } else {
        fprintf(stderr, "Tokenizer not nullified properly.\n");
    }

    printf("Testing tokenizer_create memory leaks completed. \n\n");
}

void test_combined_memory_leak() {
    fprintf(stderr, "\nTesting combined create_token and create_tokenizer memory management...\n");

    // Step 1: Create the tokenizer
    Tokenizer* tokenizer = create_tokenizer(10);
    if (!tokenizer) {
        fprintf(stderr, "Failed to create tokenizer\n");
        return;
    }
    fprintf(stderr, "Tokenizer created: %p\n", (void*)tokenizer);

    // Step 2: Create tokens and add them to the tokenizer
    Token* token1 = create_token("hello");
    Token* token2 = create_token("world");

    if (!token1 || !token2) {
        fprintf(stderr, "Failed to create tokens\n");
        free_token(token1);
        free_token(token2);
        free_tokenizer(&tokenizer);
        return;
    }

    fprintf(stderr, "Token1 created: %p, text: %p\n", (void*)token1, (void*)token1->text);
    fprintf(stderr, "Token2 created: %p, text: %p\n", (void*)token2, (void*)token2->text);

    // Add tokens to the tokenizer's vocabulary
    add_to_vocabulary(tokenizer, (const char*)token1->text);
    add_to_vocabulary(tokenizer, (const char*)token2->text);

    free_token(token1);
    free_token(token2);

    fprintf(stderr, "Tokens added to tokenizer.\n");

    // Step 3: Free the tokenizer, which should also free the tokens
    free_tokenizer(&tokenizer);

    // Verify tokenizer is nullified
    if (!tokenizer) {
        fprintf(stderr, "Tokenizer successfully freed and nullified.\n");
    } else {
        fprintf(stderr, "Tokenizer was not nullified properly.\n");
    }

    printf("Testing combined Memory completed.\n\n");
}

void test_tokenize_functionality() {
    printf("\nTesting tokenize functionality...\n");

    // Step 1: Create a dataset
    Dataset* dataset = initialize_dataset(3);
    if (!dataset) {
        fprintf(stderr, "Failed to create dataset.\n");
        return;
    }

    // Add lines to the dataset
    add_line(dataset, "hello world");
    add_line(dataset, "foo bar");
    add_line(dataset, "");

    // Step 2: Tokenize the dataset
    size_t num_tokens = 0;
    Token** tokens = tokenize(dataset, " ", &num_tokens);
    if (!tokens) {
        fprintf(stderr, "Tokenize returned NULL.\n");
        free_dataset(dataset);
        return;
    }

    // Step 3: Verify tokens
    const char* expected_tokens[] = {"hello", "_", "world", "_", "foo", "_", "bar", "_"};
    size_t expected_count =8;

    if (num_tokens != expected_count) {
        printf("\nExpected %zu tokens, but got %zu tokens.\n", expected_count, num_tokens);
    } else {
        for (size_t i = 0; i < num_tokens; i++) {
            if (strcmp(tokens[i]->text, expected_tokens[i]) != 0) {
                printf("\nToken mismatch at index %zu: expected '%s', got '%s'.\n",
                        i, expected_tokens[i], tokens[i]->text);
            } else {
                printf("\nToken %zu matches: %s.\n", i, tokens[i]->text);
            }
        }
    }

    // Step 4: Free tokens
    for (size_t i = 0; i < num_tokens; i++) {
        free_token(tokens[i]);
    }
    free(tokens);

    // Step 5: Free the dataset
    free_dataset(dataset);
}

void test_tokenize_memory_leaks() {
    printf("\nTesting tokenize for memory leaks...\n");

    // Step 1: Create a dataset with multiple lines
    Dataset* dataset = initialize_dataset(22);
    if (!dataset) {
        fprintf(stderr, "Failed to create dataset.\n");
        return;
    }

    add_line(dataset, "this is a test");
    add_line(dataset, "another line");
    add_line(dataset, "edge-case , delimiters");

    // Step 2: Tokenize with delimiters
    size_t num_tokens = 0;
    Token** tokens = tokenize(dataset, " ,", &num_tokens);

    if (tokens) {
        // Free tokens
        for (size_t i = 0; i < num_tokens; i++) {
            free_token(tokens[i]);
        }
        free(tokens);
    } else {
        printf("\nTokenize returned NULL unexpectedly.\n");
    }

    // Step 3: Free the dataset
    free_dataset(dataset);
}

void test_initialize_vocabulary_memory_leak() {
    printf("\nTesting initialize_vocabulary for memory leaks...\n");

    // Step 1: Create a dataset
    Dataset* dataset = initialize_dataset(10);
    if (!dataset) {
        fprintf(stderr, "Failed to create dataset.\n");
        return;
    }

    // Add sample data to the dataset
    add_line(dataset, "hello");
    add_line(dataset, "world");

    // Step 2: Create a tokenizer
    Tokenizer* tokenizer = create_tokenizer(10);
    if (!tokenizer) {
        fprintf(stderr, "Failed to create tokenizer.\n");
        free_dataset(dataset);
        return;
    }

    // Step 3: Initialize vocabulary
    initialize_vocabulary(tokenizer, dataset);

    // Step 4: Free resources
    free_tokenizer(&tokenizer);
    free_dataset(dataset);

    // Verify tokenizer is nullified
    if (!tokenizer) {
        fprintf(stderr, "Tokenizer successfully freed and nullified.\n");
    } else {
        fprintf(stderr, "Tokenizer was not nullified properly.\n");
    }
}

void test_count_pairs(){
	printf("\nTesting count_pairs function for memory leaks and functionality....\n");

	Dataset* dataset = initialize_dataset(10);
	if(!dataset){
		fprintf(stderr,"\nFailed to create dataset.\n");
		return;
	}

	add_line(dataset,"Hello");
	add_line(dataset,"world.");

	Tokenizer* tokenizer = create_tokenizer(10);
	if (!tokenizer) {
        	fprintf(stderr, "Failed to create tokenizer.\n");
        	free_dataset(dataset);
        	return;
	}

	size_t num_tokens;
	Token** tokenized_data = tokenize(dataset, "",&num_tokens);
	count_pairs(tokenizer,tokenized_data,num_tokens);

	free_tokens(tokenized_data,num_tokens);
	free_tokenizer(&tokenizer);
	free_dataset(dataset);

	if (!tokenizer) {
        	fprintf(stderr, "Tokenizer successfully freed and nullified.\n");
    	} else {
        	fprintf(stderr, "Tokenizer was not nullified properly.\n");
    	}
}

void print_test_result(const char* test_name, int result) {
    printf("%s: %s\n", test_name, result ? "PASSED" : "FAILED");
}

// Unit test for find_most_freq_pairs
void test_find_most_freq_pairs() {
    printf("Running find_most_freq_pairs Unit Test...\n");

    // Step 1: Create and populate the hash table
    HashTable* hash_table = create_hash_table(10);
    if (hash_table == NULL) {
        fprintf(stderr, "Failed to create hash table\n");
        return;
    }

    // Insert some pairs into the hash table
    insert_into_hash_table(hash_table, "a b", 3);
    insert_into_hash_table(hash_table, "b c", 5); // Most frequent
    insert_into_hash_table(hash_table, "c d", 2);

    // Step 2: Call the function to test
    HashEntry* most_frequent = find_most_freq_pairs(hash_table);

    // Step 3: Validate the result
    int result = 1; // Assume test passes initially
    if (most_frequent == NULL) {
        fprintf(stderr, "Failed to find the most frequent pair\n");
        result = 0;
    } else if (strcmp(most_frequent->key, "b c") != 0 || most_frequent->value != 5) {
        fprintf(stderr, "Expected 'b c' with frequency 5, but got '%s' with frequency %zu\n",
                most_frequent->key, most_frequent->value);
        result = 0;
    }

    // Print the test result
    print_test_result("find_most_freq_pairs", result);

    // Step 4: Clean up
    free_hash_table(hash_table);
}

// Helper function to print tokenized data
void print_tokens(Token** tokens, size_t size) {
    printf("Tokenized Data: ");
    for (size_t i = 0; i < size; i++) {
        if (tokens[i] != NULL) {
            printf("%s ", tokens[i]->text);
        }
    }
    printf("\n");
}

// Unit test for merge_most_freq_pair
void test_merge_most_freq_pair() {
    printf("Running merge_most_freq_pair Unit Test...\n");

    // Step 1: Create tokenized data
    size_t size = 5;
    Token* tokenized_data[] = {
        create_token("a"),
        create_token("b"),
        create_token("c"),
        create_token("a"),
        create_token("b")
    };

    // Step 2: Create most frequent pair
    HashEntry most_freq_pair = {.key = strdup("a b"), .value = 2};

    // Step 3: Print initial tokens
    print_tokens(tokenized_data, size);

    // Step 4: Call the function to test
    char* res = merge_most_freq_pair(tokenized_data, &most_freq_pair, &size);

    // Step 5: Validate the results
    int result = 1; // Assume test passes
    if (strcmp(tokenized_data[0]->text, "ab") != 0) {
        fprintf(stderr, "Expected first token to be 'ab', got '%s'\n", tokenized_data[0]->text);
        result = 0;
    }
    if (strcmp(tokenized_data[1]->text, "c") != 0) {
        fprintf(stderr, "Expected second token to be 'c', got '%s'\n", tokenized_data[1]->text);
        result = 0;
    }
    if (strcmp(tokenized_data[2]->text, "ab") != 0) {
        fprintf(stderr, "Expected third token to be 'a', got '%s'\n", tokenized_data[2]->text);
        result = 0;
    }
    if (tokenized_data[4] != NULL) {
        fprintf(stderr, "Expected last token to be NULL, but it's not\n");
        result = 0;
    }

    // Print final tokens and test result
    print_tokens(tokenized_data, size); // Size reduced by 1
    print_test_result("merge_most_freq_pair", result);

    // Step 6: Clean up
    for (size_t i = 0; i < size; i++) {
        if (tokenized_data[i] != NULL) {
            free_token(tokenized_data[i]);
        }
    }
    free(most_freq_pair.key);
    free(res);
}

void test_BPE1(){
	Dataset* dataset = initialize_dataset(10000);
	int res = load_dataset_from_file(dataset,"/mnt/data/llm/datasets/ebk2.txt");
	if(res == -1){
		DEBUG_TOK("Error while loading file.");
		return;
	}

	Tokenizer* tokenizer = create_tokenizer(MAX_VOCAB_SIZE);
	BPE(tokenizer, dataset);

	//  save the vocabulary to a file
	size_t num_tokens = 0;
	print_vocabulary(tokenizer,&num_tokens);
	//Cleanup
	free_tokenizer(&tokenizer);
	free_dataset(dataset);
}
