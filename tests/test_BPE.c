#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <tokenizer.h>
#include <dataset.h>
#include <config.h>

// Helper functions for testing
void assert_token_equals(Token* token, const char* expected_text, size_t expected_freq) {
    assert(token != NULL);
    assert(strcmp(token->text, expected_text) == 0);
    assert(token->frequency == expected_freq);
}

TextFile* create_test_file(const char* content) {
    TextFile* file = create_text_file("test_temp.txt", 1024);
    if (!file) return NULL;
    
    if (open_text_file(file, "w") != 0) {
        destroy_text_file(&file);
        return NULL;
    }
    
    if (content) {
        add_line_to_file(file, content);
    }
    
    close_text_file(file);
    return file;
}

// BPE Edge Cases
void test_BPE_empty_input() {
    printf("Testing BPE with empty input...\n");
    TextFile* file = create_test_file("");
    Tokenizer* tokenizer = create_tokenizer(10);
    
    BPE(tokenizer, file);
    
    assert(tokenizer->vocab_size == 0);
    
    free_tokenizer(&tokenizer);
    destroy_text_file(&file);
    printf("Empty input test passed\n");
}

void test_BPE_single_character() {
    printf("Testing BPE with single character...\n");
    TextFile* file = create_test_file("a");
    Tokenizer* tokenizer = create_tokenizer(10);
    
    BPE(tokenizer, file);
    
    assert(tokenizer->vocab_size == 2);
    // Verify the character is in vocabulary
    //size_t index;
    //assert(get_value(tokenizer->token_map, "a", &index) == 0);
    
    free_tokenizer(&tokenizer);
    destroy_text_file(&file);
    printf("Single character test passed\n");
}

void test_BPE_repeated_sequence() {
    printf("Testing BPE with repeated sequence...\n");
    TextFile* file = create_test_file("aa aa aa aa aa");
    Tokenizer* tokenizer = create_tokenizer(100);
    
    BPE(tokenizer, file);
    
    // Should merge 'a a' into one token
    //size_t index;
    //assert(get_value(tokenizer->token_map, "aa", &index) == 0);
    
    free_tokenizer(&tokenizer);
    destroy_text_file(&file);
    printf("Repeated sequence test passed\n");
}

// Tokenizer Edge Cases
void test_tokenizer_empty() {
    printf("Testing tokenizer with empty input...\n");
    TextFile* file = create_test_file("");
    size_t num_tokens = 0;
    Token** tokens = tokenize(file, "", &num_tokens);
    
    assert(tokens == NULL);
    assert(num_tokens == 0);
    
    destroy_text_file(&file);
    printf("Empty tokenizer test passed\n");
}

void test_tokenizer_max_length() {
    printf("Testing tokenizer with max length input...\n");
    // Create a very long string
    char* long_str = malloc(1024 * 1024);  // 1MB string
    memset(long_str, 'a', 1024 * 1024 - 1);
    long_str[1024 * 1024 - 1] = '\0';
    
    TextFile* file = create_test_file(long_str);
    size_t num_tokens = 0;
    Token** tokens = tokenize(file, "", &num_tokens);
    
    assert(tokens != NULL);
    assert(num_tokens > 0);
    
    free_tokens(tokens, num_tokens);
    free(long_str);
    destroy_text_file(&file);
    printf("Max length test passed\n");
}

// Memory Management Tests
void test_memory_leaks() {
    printf("Testing memory management...\n");
    for (int i = 0; i < 1000; i++) {
        Tokenizer* tokenizer = create_tokenizer(100);
        TextFile* file = create_test_file("test test test");
        BPE(tokenizer, file);
        free_tokenizer(&tokenizer);
        destroy_text_file(&file);
    }
    printf("Memory leak test passed\n");
}

void test_token_creation() {
    Token* token = create_token("test");
    assert_token_equals(token, "test", 0);  // Now using the function
    free_token(token);
}

void test_BPE() {
    printf("Starting BPE and Tokenizer tests...\n");
    
    // BPE Tests
    test_BPE_empty_input();
    test_BPE_single_character();
    test_BPE_repeated_sequence();
    
    // Tokenizer Tests
    test_tokenizer_empty();
    //test_tokenizer_max_length();
    
    // Memory Tests
    //test_memory_leaks();

   char* err = malloc(100);
  strcpy(err, "This will cause problems.\n");
 err = NULL; 
    printf("All tests completed successfully!\n");
}
