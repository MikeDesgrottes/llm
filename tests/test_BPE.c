#include <stdio.h>
#include <stdlib.h>
#include "../src/tokenizer.h"
#include "../src/dataset.h"

void print_vocabulary(Tokenizer* tokenizer, size_t* valid_count);

// Helper function to print tokenized data
void print_tokenized_data(Token** tokenized_data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        printf("%s ", tokenized_data[i]->text);
    }
    printf("\n");
}

void test_BPE() {
    printf("Running BPE Tests...\n");
	size_t count = 0;
    // Edge Case 1: Empty dataset
    Dataset* empty_dataset = initialize_dataset(0);
    Tokenizer* empty_tokenizer = create_tokenizer(10);
    BPE(empty_tokenizer, empty_dataset);
    printf("\nEdge Case 1: Empty Dataset\n");
    print_vocabulary(empty_tokenizer,&count);

    free_tokenizer(empty_tokenizer);
    free_dataset(empty_dataset);

    // Edge Case 2: Single-line dataset
    Dataset* single_line_dataset = initialize_dataset(1);
    add_line(single_line_dataset, "a b c d");
    Tokenizer* single_line_tokenizer = create_tokenizer(10);
    BPE(single_line_tokenizer, single_line_dataset);
    printf("\nEdge Case 2: Single-line Dataset\n");
    print_vocabulary(single_line_tokenizer,&count);

    free_tokenizer(single_line_tokenizer);
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
    free_tokenizer(general_tokenizer);
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
    printf("Checking token_map consistency...\n");
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
    printf("Running test_add_to_vocabulary...\n");

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
    free_tokenizer(tokenizer);

    printf("test_add_to_vocabulary completed.\n");
}
