#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <tokenizer.h>
#include <dataset.h>
#include <config.h>


void assert_token_equals(Token* token, const char* expected_text, size_t expected_freq);
TextFile* create_test_file(const char* content);
void test_BPE_empty_input();

void test_BPE_single_character();
void test_BPE_repeated_sequence();
void test_tokenizer_empty();
void test_tokenizer_max_length();
void test_memory_leaks();
void test_BPE();
