#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokenizer.h"
#include "utils.h"
#include "hash_table.h"

// Create a tokenizer instance
Tokenizer* create_tokenizer(size_t max_vocab_size) {
    Tokenizer* tokenizer = (Tokenizer*)malloc(sizeof(Tokenizer));
    tokenizer->vocabulary = NULL; // No tokens initially
    tokenizer->vocab_size = 0;
    tokenizer->max_vocab_size = max_vocab_size;
    tokenizer->pair_freqs = create_hash_table(300);
    return tokenizer;
}

// Add a token to the vocabulary
void add_to_vocabulary(Tokenizer* tokenizer, const char* token) {
    // Check if the token is already in the vocabulary
    for (size_t i = 0; i < tokenizer->vocab_size; i++) {
        if (strcmp(tokenizer->vocabulary[i], token) == 0) {
            return; // Token already exists
        }
    }

    // Expand the vocabulary if needed
    if (tokenizer->vocab_size >= tokenizer->max_vocab_size) {
        printf("Vocabulary size limit reached!\n");
        return;
    }

    tokenizer->vocabulary = (char**)realloc(tokenizer->vocabulary, sizeof(char*) * (tokenizer->vocab_size + 1));
    tokenizer->vocabulary[tokenizer->vocab_size] = strdup(token);
    tokenizer->vocab_size++;
}

// Tokenize input text
char** tokenize(const Tokenizer* tokenizer, const char* text, const char* delimiters, size_t* num_tokens) {
    char* copy = strdup(text); // Create a modifiable copy of the text
    char* token = strtok(copy, delimiters); // Tokenize the text

    char** tokens = NULL;
    size_t count = 0;

    while (token != NULL) {
        tokens = (char**)realloc(tokens, sizeof(char*) * (count + 1));
        tokens[count++] = strdup(token); // Add token to the array
        token = strtok(NULL, delimiters);
    }

    free(copy);
    *num_tokens = count; // Return the number of tokens
    return tokens;
}

// Split a string character wise.

char* split_by_character(const char* input){
	size_t length = strlen(input);
	char* text = (char*)malloc(sizeof(char)*(length+ 1));

	if(!text){
		perror("Memory Allocation Failed!");
	}

	for(size_t i = 0;i < strlen(input);i++){
		text[i] = input[i];
	}
	text[length] = '\0';
	return text;
}

//

char*** tokenize_dataset_to_characters(const char** dataset, size_t num_lines, const char* delimiter) {
    char*** character_dataset = (char***)malloc(num_lines * sizeof(char**));
    if (!character_dataset) {
        perror("Memory allocation failed");
        return NULL;
    }

    for (size_t i = 0; i < num_lines; i++) {
        size_t num_tokens;
        char** tokens = tokenize(dataset[i], delimiter, &num_tokens);
        character_dataset[i] = (char**)malloc((num_tokens * sizeof(char*)) + 1);

        for (size_t j = 0; j < num_tokens; j++) {
            character_dataset[i][j] = split_by_character(tokens[j]);
        }
	character_dataset[num_tokens] = NULL;
        free_tokens(tokens, num_tokens); // Free tokenized words
    }

    return character_dataset;
}

void free_tokens(char** tokens, size_t num_tokens) {
    for (size_t i = 0; i < num_tokens; i++) {
        free(tokens[i]);
    }
    free(tokens);
}
// Free the tokenizer and its resources
void free_tokenizer(Tokenizer* tokenizer) {
    for (size_t i = 0; i < tokenizer->vocab_size; i++) {
        free(tokenizer->vocabulary[i]);
    }

	free_hash_table(tokenizer->pair_freqs;
    free(tokenizer->vocabulary);
    free(tokenizer);
}

void initialize_vocabulary(Tokenzier* tokenizer, const char** dataset){
	
}

void initialize_freqs(Tokenizer* tokenizer, size_t rows, size_t columns){
	tokenizer->pair_freqs = malloc(sizeof(size_t*)*rows);
	for(size_t i = 0; i < rows;i++){
		tokenizer->pair_freqs[i] = malloc(sizeof(size_t)*columns);
		for (size_t j = 0; j < columns; j++) {
            		tokenizer->pair_freqs[i][j] = 0; // Initialize to 0
        	}
	}
}


void count_pairs(Tokenizer* tokenizer, size_t rows, size_t columns, const char** text){
	for(size_t i = 0;i < rows;i++){
		char* tmp = text[i];
		size_t len = strlen(tmp);
		for(size_t j = 0; j < len - 1; j++){
			size_t row = tmp[j] - 'a';
			size_t column = tmp[j + 1] - 'a';
			if(row < 26 && column <26){
				tokenizer->pair_freqs[row][column]++;
			}
		}
	}
}

HahEntry* find_most_freq_pairs(HashTable* hash_table){
	HashEntry* entry = NULL;
	if(hash_table == NULL){
		fprintf(stderr, "Error: invalid hash table!\n");
		exit(EXIT_FAILURE);
	}

	if(hash_table->size == 0){
		fprintf(stderr, "No entry in hash table.");
		return NULL;
	}
	for(size_t i = 0; i < hash_table->capacity;i++){
		if(hash_table->entries[i] != NULL){
			if(entry == NULL || hash_table->entries[i]->value > entry->value){
				entry = hash_table->entries[i];
			}
		}
	}
	return entry;
}


char*  merge_pairs(Tokenizer* tokenizer,size_t *best_row, size_t* best_column){
	if(tokenizer->vocabulary == NULL){
		tokenizer->vocabulary = (char**)malloc(sizeof(char*));
	}	

}

char* create_pair_keys(const char* token1, const char* token2){
	size_t len1 = strlen(token1);
	size_t len2 = strlen(token2);
	char* key = malloc(len1 + len2 + 2);
	if(key == NULL){
		printf("Key creation failed!");
	}

	sprintf(key,"%s_%s",token1,token2);
}
void initial_BPE(Tokenizer* tokenizer, const char** dataset, size_t lines){
	if(tokenizer->vocabulary == NULL){
                tokenizer->vocabulary = (char**)malloc(sizeof(char*));
        }
	size_t num_of_tokens;
	char*** dataset_characters = tokenize_dataset_to_characters(dataset, lines,' ');
	for(size_t i = 0; i < lines;i++){
		size_t t = count_tokens_in_line(dataset_to_characters[i]);
		for(size_t j = 0;j < t;j++){
			char* token = dataset_characters[i][j];
			for(size_t k = 0; k <strlen(token) - 1;k++){
				char* pair_key =  create_pair_keys(dataset_characters[i][j][k],dataset_characters[i][j][k+1]);
				increment_frequency(tokenizer->pair_freqs,pair_key);

			}
		}
	}
	
}

size_t count_tokens_in_line(char** tokens) {
    size_t count = 0;
    while (tokens[count] != NULL) { // Assume NULL terminator
        count++;
    }
    return count;
}
//initialize the vocabulary

