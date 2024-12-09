#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stddef.h> // For size_t
#include "hash_table.h"
#include "dataset.h"


typedef struct {
        char* text;
        size_t length;
        size_t frequency;
} Token;
// Define the Tokenizer struct
typedef struct {
    Token** vocabulary;        // Dynamic array of vocabulary tokens
    size_t vocab_size;        // Number of tokens in the vocabulary
    size_t max_vocab_size;    // Maximum vocabulary size
    HashTable *pair_freqs;
    HashTable *token_map;
} Tokenizer;

// Function declarations
Tokenizer* create_tokenizer(size_t max_vocab_size);
void add_to_vocabulary(Tokenizer* tokenizer, const char* token);
Token** tokenize(const Dataset* dataset, const char* delimiters, size_t* num_tokens);
void free_tokenizer(Tokenizer** tokenizer);
char** split_by_character(const char* input);
void free_tokens(char** tokens, size_t num_tokens);
char*** tokenize_dataset_to_characters(const char** dataset, size_t num_lines, const char* delimiter);
void initialize_vocabulary(Tokenizer* tokenizer, Dataset* dataset);
void initialize_freq(Tokenizer* tokenizer, size_t rows, size_t columns);
void count_pairs(Tokenizer* tokenizer, Token** tokens, size_t num_tokens);
HashEntry* find_most_freq_pairs(HashTable* hash_table);
char* create_pair_key(const char* token1, const char* token2);
void BPE(Tokenizer* tokenizer, Dataset* dataset);
void merge_most_freq_pair(Token** tokenized_data, HashEntry* most_freq_pair, size_t size);
int insert_into_token_map(HashTable* table, const char* key, size_t value);
// Funcrtion associated with the token struct.

Token* create_token(const char* text);
void free_token(Token* token);
void increment_token_frequency(Token* token);
void reset_token_frequency(Token* token);
Token* merge_tokens(Token* token1, Token* token2);
int token_exists_in_vocabulary(Tokenizer* tokenizer,const char* text);
Token* find_most_frequent_token_pair(Token** tokens, size_t num_tokens);
void free_token_array(Token** tokens, size_t num_tokens);
#endif // TOKENIZER_H

