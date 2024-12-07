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
    tokenizer->token_map = create_hash_table(max_vocab_size);
    return tokenizer;
}

// Add a token to the vocabulary
void add_to_vocabulary(Tokenizer* tokenizer, const char* token) {
    // Check if the token is already in the vocabulary
    int index = token_exists_in_vocabulary(tokenizer->vocabulary, token);
    if(index >= 0){
    	tokenizer->vocabulary[index]->frequency++;//Token already exists increment its frequency by one.
	return;
    }

    Token* new_token = create_token(token);
    if (new_token == NULL) {
        fprintf(stderr, "Error allocating memory for new token\n");
        return;  // Handle memory allocation failure for token
    }

    // Expand the vocabulary if needed

    if (tokenizer->vocab_size == 0) {
        // Initial allocation for the vocabulary
        tokenizer->vocabulary = (Token**)malloc(sizeof(Token*) * 100);
        if (tokenizer->vocabulary == NULL) {
            fprintf(stderr, "Error in allocating memory for vocabulary\n");
            free(new_token);
            return;
        }
        tokenizer->max_vocab_size = 100;  // Set initial max size to 1
    } else if (tokenizer->vocab_size >= tokenizer->max_vocab_size) {
        // Double the size of the vocabulary if it's full
        size_t new_max_size = tokenizer->max_vocab_size * 2;
        Token** new_vocabulary = (Token**)realloc(tokenizer->vocabulary, sizeof(Token*) * new_max_size);
        if (new_vocabulary == NULL) {
            fprintf(stderr, "Error while reallocating memory for vocabulary\n");
            free(new_token);
            return;
        }
        tokenizer->vocabulary = new_vocabulary;
        tokenizer->max_vocab_size = new_max_size;
    }

    size_t hash_index = hash_function(token, tokenizer->max_vocab_size);
    size_t final_index = hash_index;
    size_t step = 0;

    while(step < tokenizer->max_vocab_size){
    	if(tokenizer->vocabulary[final_index] == NULL){
		tokenizer->vocabulary[final_index] = new_token;
		tokenizer->vocab_size++;
        	int result = insert_into_hash_table(tokenizer->token_map, token, final_index);
		if (result != 0) {
    		// Rollback changes in vocabulary if token_map insertion fails
    		fprintf(stderr, "Error inserting token into token_map\n");
    		tokenizer->vocabulary[final_index] = NULL;
		free_token(new_token);
		}
		return;
	}

	final_index = (final_index + step) % tokenizer->max_vocab_size;
	step++;
    }
    fprintf(stderr,"Error: Unable to find an empty slot in vocabulary after probing\n");
    free_token(new_token);
}

// Tokenize input text
Token** tokenize(const Dataset* dataset, const char* delimiters, size_t* num_tokens) {
	if(delimiters == NULL || dataset == NULL){
		return NULL;
	}

	if(strlen(delimiters) == 0){
		size_t count = 0;
		Token** tokens = (Token**)malloc(sizeof(Token*));

		if(tokens == NULL){
			fprintf(stderr,"Error: failed to allocate memory for tokens\n");
			return NULL;
		}
		// tokenize at chracter-level
		for(size_t i = 0; i < dataset->num_lines;i++){
			char* line = dataset->lines[i];
			if(line == NULL || strlen(line) == 0){
				continue;
			}
			char* copy = strdup(line);
			if (copy == NULL) {
            			fprintf(stderr, "Error: Failed to duplicate line\n");
            			// Free already allocated tokens
            			for (size_t j = 0; j < count; j++) {
                			free_token(tokens[j]);
            			}
            			free(tokens);
            			return;
        		}
			char* token = strtok(copy," ");
			while(token != NULL){
				size_t step = 0;
				char** text = split_by_character((const char*)token);
				if (text == NULL) {
                			fprintf(stderr, "Error: Failed to split token into characters\n");
                			// Free memory and clean up
                			free(copy);
                			for (size_t j = 0; j < count; j++) {
                    				free_token(tokens[j]);
                			}
                			free(tokens);
                			return;
            			}
				while(text[step] != NULL){
					Token* tok = create_token(text[step]);
					tokens[count] = tok;
					count++;
					step++;
					tokens = (Token**)realloc(tokens,sizeof(Token*) * (count + 1));
					if(tokens == NULL){
						// implement error reporting here
						fprintf(stderr, "Error: Failed to reallocate memory for tokens\n");
                    				for (size_t j = 0; j < count; j++) {
                        				free_token(tokens[j]);
                    				}
                    				for (size_t k = 0; k <= step; k++) {
                        				free(text[k]);
                    				}
                    				free(text);
                    				free(copy);
                    				return;
					}
				}
				Token* sperator = create_token("_");
				if(sperator == NULL){
					fprintf(stderr,"Error: Failed to create sperator token\n");
					for (size_t j = 0; j < count; j++) {
            					free_token(tokens[j]);
        				}
        				free(copy);
        				free(tokens);
					return;
				}
				tokens[count++] = seperator;
				token = strtok(NULL," ");
			}

			for (size_t k = 0; k < step; k++) {
               			free(text[k]);
            		}
            		free(text);
            		
			free(copy);
		}
		tokens[count] = NULL;
		*num_tokens = count;
		return tokens;
	}else if (delimiters != NULL && strlen(delimiters) > 0) {
    	size_t count = 0;
    	size_t capacity = 10; // Initial capacity
    	Token** tokens = (Token**)malloc(sizeof(Token*) * capacity);

    	if (tokens == NULL) {
        	fprintf(stderr, "Error: Failed to allocate memory for tokens\n");
        	return NULL;
    	}

    // Tokenize by normal delimiters
    for (size_t i = 0; i < dataset->num_lines; i++) {
        char* line = dataset->lines[i];
        if (line == NULL || strlen(line) == 0) {
            continue; // Skip empty lines
        }

        char* copy = strdup(line);
        if (copy == NULL) {
            fprintf(stderr, "Error: Failed to duplicate line\n");
            for (size_t j = 0; j < count; j++) {
                free_token(tokens[j]);
            }
            free(tokens);
            return NULL;
        }

        // Split line into tokens using strtok
        char* token = strtok(copy, delimiters);
        while (token != NULL) {
            	Token* tok = create_token(token);
            	if (tok == NULL) {
                	fprintf(stderr, "Error: Failed to create token\n");
                	for (size_t j = 0; j < count; j++) {
                    	free_token(tokens[j]);
                	}
                	free(copy);
                	free(tokens);
                	return NULL;
            	}

            	// Add token to tokens array
            	if (count >= capacity) {
                	capacity *= 2;
                	tokens = (Token**)realloc(tokens, sizeof(Token*) * capacity);
                	if (tokens == NULL) {
                    	fprintf(stderr, "Error: Failed to reallocate memory for tokens\n");
                    	for (size_t j = 0; j < count; j++) {
                        	free_token(tokens[j]);
                    	}
                    	free(copy);
                    	return NULL;
                	}
            	}

            	tokens[count++] = tok;

            	// Get the next token
            	token = strtok(NULL, delimiters);
        	}

        	free(copy); // Free the duplicated line
    	}

   	tokens[count] = NULL; // Null-terminate tokens array
    	*num_tokens = count;  // Update the count of tokens
    	return tokens;
	}
}

// Split a string character wise.

char** split_by_character(const char* input){
	if(input == "" || input == NULL){
		return NULL;
	}
	size_t length = strlen(input);
	char** text = (char**)malloc(sizeof(char*)*(length + 1));

	if(!text){
		fprintf(stderr,"Error: Failed to allocate memory for strings\n");
		return NULL;
	}

	for(size_t i = 0;i < length;i++){
		char* character = (char*)malloc(2);
		if (!character) {
            fprintf(stderr, "Error: Failed to allocate memory for character token\n");

            // Free already allocated memory before returning NULL
            	for (size_t	j = 0; j < i; j++) {
                	free(text[j]);
            	}
            	free(text);
            	return NULL;
        	}
		character[0] = input[i];
		character[1] = '\0';
		text[i] = character;
	}

	text[length] = NULL;
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

void initialize_vocabulary(Tokenzier* tokenizer, Dataset* dataset){
	if(tokenizer == NULL || dataset == NULL){
		return; // nothing to do here
			// in the future create a new tokenizer if dataset exists but tokenizer is null and have the pointer points to the new address
	}
	size_t num_tokens = 0;
	Token** tokens = tokenize(dataset,"",&num_tokens);
	if(tokens == NULL){
		fprintf(stderr, "Error: the dataset could not be tokenize at character level\n");
		return;
	}
	if(num_tokens == 0){
		return; // No tokens where found
	}
	
	// Add the tokens to the library.
	for (size_t i = 0; i < num_tokens; i++) {
        	add_to_vocabulary(tokenizer,(const char*) tokens[i]->text);
        	free_token(tokens[i]); // Free the token after adding it to the vocabulary
    	} 
	free(tokens);
}


void count_pairs(Tokenizer* tokenizer){
	 if (tokenizer == NULL || tokenizer->token_map == NULL || tokenizer->pair_freqs == NULL) {
        	fprintf(stderr, "Error: Tokenizer or hash tables not initialized\n");
        	return;
    	}
	 reset_hash_table(tokenizer->pair_freqs); // Clear existing frequencies

    // Iterate through the vocabulary
    for (size_t i = 0; i < tokenizer->vocab_size; i++) {
        Token* token = tokenizer->vocabulary[i];
        if (token == NULL || token->length < 2) {
            continue; // Skip empty or single-character tokens
        }

        // Count adjacent character pairs
        for (size_t j = 0; j < token->length - 1; j++) {
            char pair[3] = {token->text[j], token->text[j + 1], '\0'};

            int frequency = get_value(tokenizer->pair_freqs, pair, strcmp);
            if (frequency >= 0) {
                insert_into_hash_table(tokenizer->pair_freqs, pair, frequency + 1);
            } else {
                insert_into_hash_table(tokenizer->pair_freqs, pair, 1);
            }
        }
    }
}

HahEntry* find_most_freq_pairs(HashTable* hash_table){
	HashEntry* entry = NULL;
	if(hash_table == NULL){
		fprintf(stderr, "Error: invalid hash table!\n");
		return NULL;
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




// Code to implement for tokens
//

Token* create_token(const char* text){
	if(text == NULL){
		fprintf(stderr,"Error invalid text to create token\n");
		return NULL;
	}
	Token* token = (Token*)malloc(sizeof(Token));
	if(!token){
		fprintf(stderr, "Error allocating memory for token\n");
		return NULL;
	}

	token->length = strlen(text);
	token->text = strdup(text);
	if(!token->text){
		fprintf(stderr,"Failed to allocate memory for token text\n");
		free(token);
		return NULL;
	}
	token->frequency  = 0;
	return token;
}


void free_token(Token* token){
	if(token){
		free(token->text);
		free(token);	
	}
}


void increment_frequency(Token* token){
	if(token){
		token->frequency++;
	}
}

void reset_token_frequency(Token* token){
	token->frequency = 0;
}

Token* merge_tokens(Token* token1, Token* token2){
	if(token1 == NULL || token2 == NULL){
		fprintf(stderr,"Error invalid token arguments\n"):
		return NULL;
	}
	char* new_string = (char*)malloc(token1->length + token2->length + 1);
	if(!new_string){
		fprintf(stderr,"Error allocating memory for concating two strings\n");
		return NULL;
	}

	sprintf(new_string, "%s%s", token1->text, token2->text);
	Token* new_token = create_token((const char*) new_string);
	free(new_string);
	return new_token;
}

int token_exists_in_vocabulary(Tokenizer* tokenizer, const char* text){
	return get_value(tokenizer->token_map,text,strcmp);
}

Token* find_most_frequent_token_pair(Token** tokens, size_t num_tokens){
	size_t max_freq = 0;
	size_t index = 0;
	for(size_t i = 0; i < num_tokens;i++){
		Token* token = tokens[i];
		if(token == NULL){
			// skipping empty slots
		}else{
			if(token->frequency > max_freq){
				max_freq = token->frequency;
				index = i;
			}
		}
	}
	return tokens[index];
}
