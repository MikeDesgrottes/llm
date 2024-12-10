#ifndef TEST_BPE_H
#define TEST_BPE_H

#include <stddef.h> // For size_t
#include "../src/tokenizer.h" // Ensure this includes relevant tokenizer declarations
#include "../src/dataset.h"   // Ensure this includes Dataset-related declarations
#include <assert.h>
#include <stdlib.h>

// Function Declarations

/**
 * Test the memory management of the tokenizer and vocabulary.
 * This function tests the integration of memory management for tokens,
 * vocabulary, and hash tables within the tokenizer system.
 */
void test_tokenizer_memory_management();

/**
 * Test the add_to_vocabulary function for proper functionality and memory management.
 * Ensures that tokens are correctly added to the vocabulary without memory leaks.
 */
void test_add_to_vocabulary();

/**
 * Test the initialize_vocabulary function for correctness and memory management.
 * Ensures proper initialization of the vocabulary with a given dataset.
 */
void test_initialize_vocabulary();

/**
 * Test the tokenize function for correctness and memory management.
 * Validates tokenization output against expected results and ensures
 * all memory is properly cleaned up.
 */
void test_tokenize_functionality();

/**
 * Test tokenize function to ensure no memory leaks occur during operation.
 * Runs various scenarios to confirm that memory allocated during tokenization
 * is freed properly.
 */
void test_tokenize_memory_leaks();

/**
 * Test the integration of hash tables with the tokenizer system.
 * Ensures that hash table operations, when combined with tokenizer functionality,
 * operate without errors or memory leaks.
 */
void test_hash_table_tokenizer_integration();

void print_tokenized_data(Token** tokenized_data, size_t size);
void test_BPE();
void print_vocabulary(Tokenizer* tokenizer, size_t* valid_count);
void check_token_map(Tokenizer* tokenizer);
void test_free_tokenizer();
void test_memory_leak();
void test_create_tokenizer_memory_leak();
void test_combined_memory_leak();
void test_tokenize_memory_leaks();
void test_tokenize_functionality();
void test_initialize_vocabulary_memory_leak();

#endif // TEST_BPE_H

