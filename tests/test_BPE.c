#include <stdio.h>
#include <stdlib.h>
#include "../src/tokenizer.h"
#include "../src/dataset.h"
#include "../include/config.h"
#include "../include/debug.h"
#include "../src/utils.h"
#include "../src/tokenizer.h"
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
    TextFile* empty_file = create_text_file("empty.txt",1024);
    Tokenizer* empty_tokenizer = create_tokenizer(10);
    BPE(empty_tokenizer, empty_file);
    printf("\nEdge Case 1: Empty Dataset\n");
    print_vocabulary(empty_tokenizer,&count);

    free_tokenizer(&empty_tokenizer);
    destroy_text_file(&empty_file);

    // Edge Case 2: Single-line dataset
    TextFile* single_line_file = create_text_file("single.txt",1024);
    if(open_text_file(single_line_file,"a") == -1){
    	fprintf(stderr,"Error opening textfile.\n");
    }
    add_line_to_file(single_line_file, "a b c d");
    Tokenizer* single_line_tokenizer = create_tokenizer(10);
    BPE(single_line_tokenizer, single_line_file);
    
    printf("\nEdge Case 2: Single-line Dataset\n");
    print_vocabulary(single_line_tokenizer,&count);

    free_tokenizer(&single_line_tokenizer);
    close_text_file(single_line_file);
    destroy_text_file(&single_line_file);

    // General Case: Larger dataset with varied patterns and multi-character words
    TextFile* general_file = create_text_file("general.txt",1024);
    if(open_text_file(general_file,"a") == -1){
        fprintf(stderr,"Error opening textfile.\n");
    }
    add_line_to_file(general_file, "cat dog bird dog cat bird");
    add_line_to_file(general_file, "apple orange banana apple orange banana");
    add_line_to_file(general_file, "blue red green blue red green");
    add_line_to_file(general_file, "happy sad joyful happy sad joyful");
    add_line_to_file(general_file, "dog apple cat orange bird banana");

    Tokenizer* general_tokenizer = create_tokenizer(200000);
    BPE(general_tokenizer, general_file);

    printf("\nGeneral Case: Larger Dataset with Varied Patterns\n");
    print_vocabulary(general_tokenizer,&count);

    // Validate the tokenized dataset after BPE
    size_t num_tokens = 0;
    Token** tokenized_data = tokenize(general_file, "", &num_tokens);
    printf("\nFinal Tokenized Dataset:\n");
    print_tokenized_data(tokenized_data, num_tokens);

    // Cleanup
    close_text_file(general_file);
    free_tokenizer(&general_tokenizer);
    destroy_text_file(&general_file);
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
			printf("  Token %zu: %s (freq: %zu)\n", *(size_t*)entry->value, tokenizer->vocabulary[*(size_t*)entry->value]->text, tokenizer->vocabulary[*(size_t*)entry->value]->frequency);
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
    TextFile* file = create_text_file("test.txt",1024);
    if (!file) {
        fprintf(stderr, "Failed to create file.\n");
        return;
    }

    // Add lines to the dataset
    add_line_to_file(file, "hello world");
    add_line_to_file(file, "foo bar");
    add_line_to_file(file, "");

    // Step 2: Tokenize the dataset
    size_t num_tokens = 0;
    Token** tokens = tokenize(file, " ", &num_tokens);
    if (!tokens) {
        fprintf(stderr, "Tokenize returned NULL.\n");
        destroy_text_file(&file);
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
    destroy_text_file(&file);
}



void test_initialize_vocabulary_memory_leak() {
    printf("\nTesting initialize_vocabulary for memory leaks...\n");

    // Step 1: Create a dataset
   TextFile* file = create_text_file("test.txt",1024); 
    if (!file) {
        fprintf(stderr, "Failed to create dataset.\n");
        return;
    }
	if(open_text_file(file,"a") == -1){
		fprintf(stderr,"Error opening file\n");
		return;
	}


    // Add sample data to the dataset
    add_line_to_file(file, "hello");
    add_line_to_file(file, "world");

    // Step 2: Create a tokenizer
    Tokenizer* tokenizer = create_tokenizer(10);
    if (!tokenizer) {
        fprintf(stderr, "Failed to create tokenizer.\n");
        destroy_text_file(&file);
        return;
    }

    // Step 3: Initialize vocabulary
    initialize_vocabulary(tokenizer, file);

    // Step 4: Free resources
    free_tokenizer(&tokenizer);
    destroy_text_file(&file);

    // Verify tokenizer is nullified
    if (!tokenizer) {
        fprintf(stderr, "Tokenizer successfully freed and nullified.\n");
    } else {
        fprintf(stderr, "Tokenizer was not nullified properly.\n");
    }
}

void test_count_pairs(){
	printf("\nTesting count_pairs function for memory leaks and functionality....\n");

	TextFile* file = create_text_file("test.txt",1024);
	if(!file){
		fprintf(stderr,"\nFailed to create dataset.\n");
		return;
	}

	add_line_to_file(file,"Hello");
	add_line_to_file(file,"world.");

	Tokenizer* tokenizer = create_tokenizer(10);
	if (!tokenizer) {
        	fprintf(stderr, "Failed to create tokenizer.\n");
        	destroy_text_file(&file);
        	return;
	}

	size_t num_tokens;
	Token** tokenized_data = tokenize(file, "",&num_tokens);
	count_pairs(tokenizer,tokenized_data,num_tokens);

	free_tokens(tokenized_data,num_tokens);
	free_tokenizer(&tokenizer);
	destroy_text_file(&file);

	if (!tokenizer) {
        	fprintf(stderr, "Tokenizer successfully freed and nullified.\n");
    	} else {
        	fprintf(stderr, "Tokenizer was not nullified properly.\n");
    	}
}

void print_test_result(const char* test_name, int result) {
    printf("%s: %s\n", test_name, result ? "PASSED" : "FAILED");
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


