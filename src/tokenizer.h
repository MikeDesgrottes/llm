#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stddef.h> // For size_t

// Define the Tokenizer struct
typedef struct {
    char** vocabulary;        // Dynamic array of vocabulary tokens
    size_t vocab_size;        // Number of tokens in the vocabulary
    size_t max_vocab_size;    // Maximum vocabulary size
    hash_table *pair_freqs;
} Tokenizer;

// Function declarations
Tokenizer* create_tokenizer(size_t max_vocab_size);
void add_to_vocabulary(Tokenizer* tokenizer, const char* token);
char** tokenize(const Tokenizer* tokenizer, const char* text, const char* delimiters, size_t* num_tokens);
void free_tokenizer(Tokenizer* tokenizer);
char* split_by_character(const char* input);
void free_tokens(char** tokens, size_t num_tokens);
char*** tokenize_dataset_to_characters(const char** dataset, size_t num_lines, const char* delimiter);
void initialize_vocabulary(Tokenizer* tokenizer, const char** dataset);
void initialize_freq(Tokenizer* tokenizer, size_t rows, size_t columns);
void count_pairs(Tokenzier* tokenizer, size_t rows, size_t columns, const char** text);
void find_most_freq_pairs(Tokenizer* tokenizer, size_t rows, size_t columns, size_t *best_row, size_t *best_column, size_t *max_freq);
char* create_pair_keys(const char* token1, const char* token2);

#endif // TOKENIZER_H

