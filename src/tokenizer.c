#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokenizer.h"
#include "utils.h"
#include "hash_table.h"
#include "../include/config.h"

// Create a tokenizer instance
Tokenizer* create_tokenizer(size_t max_vocab_size) {
    Tokenizer* tokenizer = (Tokenizer*)malloc(sizeof(Tokenizer));
    tokenizer->vocabulary = (Token**)calloc(max_vocab_size, sizeof(Token*));
    if (tokenizer->vocabulary == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for vocabulary\n");
        free(tokenizer);
        return NULL;
    }
    tokenizer->vocab_size = 0;
    tokenizer->max_vocab_size = max_vocab_size;
    tokenizer->pair_freqs = create_hash_table(300);
    tokenizer->token_map = create_hash_table(max_vocab_size);
    return tokenizer;
}

// Add a token to the vocabulary
void add_to_vocabulary(Tokenizer* tokenizer, const char* token) {
	// Sanity check
	if(tokenizer ==NULL || token == NULL){
		fprintf(stderr,"Invalid tokenizer or tokens\n");
		return;
	}
    // Check if the token is already in the vocabulary
    int index = token_exists_in_vocabulary(tokenizer, token);
    if(index >= 0){
    	tokenizer->vocabulary[index]->frequency++;//Token already exists increment its frequency by one.
	return;
    }

    Token* new_token = create_token(token);
    if (new_token == NULL) {
        fprintf(stderr, "Error allocating memory for new token\n");
        return;  // Handle memory allocation failure for token
    }
	new_token->frequency = 1;
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
        	int result = insert_into_token_map(tokenizer->token_map, token, final_index);
		if (result != 0) {
    		// Rollback changes in vocabulary if token_map insertion fails
    			fprintf(stderr, "Error inserting token into token_map\n");
			free_token(tokenizer->vocabulary[final_index]);
    			tokenizer->vocabulary[final_index] = NULL;
			tokenizer->vocab_size--;
		}
		return;
	}else{
		//TODO: implement duplicate checking here.
	}

	final_index = (final_index + step) % tokenizer->max_vocab_size;
	step++;
    }
    fprintf(stderr,"Error: Unable to find an empty slot in vocabulary after probing\n");
    free_token(new_token);
}

int insert_into_token_map(HashTable* table, const char* key, size_t value){
	// Sanity Check
	if(table == NULL || key == NULL){
		fprintf(stderr, "Error: Invalid hash table or key\n");
		return -1;
	}

	HashEntry* entry =(HashEntry*) malloc(sizeof(HashEntry*));
	if(entry == NULL){
		fprintf(stderr,"Error: Could not allocate enough memory for hash entry\n");
		return -1;
	}
	
	for(size_t i = 0; i < table->size; i++){
		HashEntry* entry = table->entries[i];
		if(entry == NULL){
			continue;
		}else{
			if(strcmp((const char*) entry->key, key) == 0){
				table->entries[i]->value = value;
				return 0;
			}
		}
	}
	entry->key = strdup(key);
	entry->value = value;
	table->entries[table->size] = entry;
	table->size++;
	return 0; // success
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
            			return NULL;
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
                			return NULL;
            			}
				while(text[step] != NULL){
					Token* tok = create_token(text[step]);
					//tok->frequency = 1;
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
                    				return NULL;
					}
				}
				Token* seperator = create_token("_");
				if(seperator == NULL){
					fprintf(stderr,"Error: Failed to create sperator token\n");
					for (size_t j = 0; j < count; j++) {
            					free_token(tokens[j]);
        				}
        				free(copy);
        				free(tokens);
					return NULL;
				}
				tokens[count++] = seperator;
				token = strtok(NULL," ");
				for (size_t k = 0; k < step; k++) {
                                	free(text[k]);
                        	}
                        	free(text);
			}

			
            		
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
		//tok->frequency = 1;
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
	return NULL;
}

// Split a string character wise.

char** split_by_character(const char* input){
	if(strlen(input) == 0 || input == NULL){
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

	free_hash_table(tokenizer->pair_freqs);
    free(tokenizer->vocabulary);
    free(tokenizer);
}

void initialize_vocabulary(Tokenizer* tokenizer, Dataset* dataset){
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


void count_pairs(Tokenizer* tokenizer, Token** tokens, size_t num_tokens){
	 if (tokenizer == NULL || tokens == NULL || tokenizer->pair_freqs == NULL) {
        	fprintf(stderr, "Error: Tokenizer or hash tables not initialized\n");
        	return;
    	}
	 reset_hash_table(tokenizer->pair_freqs); // Clear existing frequencies

	for(size_t i = 0; i < num_tokens - 1;i++){
		Token* current = tokens[i];
		Token* next = tokens[i + 1];
		if(current == NULL || next == NULL || strcmp(current->text,"_") == 0  || strcmp(next->text,"_") == 0){
			continue;
		}


		char* pair_key = create_pair_key(current->text,next->text);
		if (pair_key == NULL) {
            		fprintf(stderr, "Error: Failed to create pair key\n");
            		continue;
        	}

        	// Update the pair frequency
        	int frequency = get_value(tokenizer->pair_freqs, pair_key, strcmp);
        	if (frequency >= 0) {
            		insert_into_hash_table(tokenizer->pair_freqs, pair_key, frequency + 1);
        	}else{
            		insert_into_hash_table(tokenizer->pair_freqs, pair_key, 1);
        	}

        	free(pair_key);
	}
}

char* create_pair_key(const char* token1, const char* token2) {
    if (token1 == NULL || token2 == NULL) {
        fprintf(stderr, "Error: NULL token provided for pair key creation\n");
        return NULL;
    }

    size_t len1 = strlen(token1); // Length of token1 (excluding \0)
    size_t len2 = strlen(token2); // Length of token2 (excluding \0)
    char* pair_key = (char*)malloc(len1 + len2 + 2); // 1 for separator, 1 for \0

    if (pair_key == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for pair key\n");
        return NULL;
    }

    // Combine token1 and token2 into pair_key, separated by a space
    sprintf(pair_key, "%s %s", token1, token2);
    return pair_key;
}
HashEntry* find_most_freq_pairs(HashTable* hash_table){
	HashEntry* entry = NULL;
	if(hash_table == NULL){
		fprintf(stderr, "Error: invalid hash table!\n");
		return NULL;
	}

	if(hash_table->size == 0){
		fprintf(stderr, "No entry in hash table.\n");
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


void increment_token_frequency(Token* token){
	if(token){
		token->frequency++;
	}
}

void reset_token_frequency(Token* token){
	token->frequency = 0;
}

Token* merge_tokens(Token* token1, Token* token2){
	if(token1 == NULL || token2 == NULL){
		fprintf(stderr,"Error invalid token arguments\n");
		return NULL;
	}
	char* new_string = (char*)malloc(token1->length + token2->length + 1);
	if(!new_string){
		fprintf(stderr,"Error allocating memory for concating two strings\n");
		return NULL;
	}

	sprintf(new_string, "%s%s", token1->text, token2->text);
	Token* new_token = create_token((const char*) new_string);
	new_token->frequency = 1;
	free(new_string);
	return new_token;
}

int token_exists_in_vocabulary(Tokenizer* tokenizer, const char* text){
	return get_value(tokenizer->token_map,text,strcmp);
}

void merge_most_freq_pair(Token** tokenized_data, HashEntry* most_freq_pair, size_t size){
	char* text = most_freq_pair->key;
	Dataset* dataset = initialize_dataset(1);
	int res = add_line(dataset,(const char*)text);
	
	if(res == -1){
		fprintf(stderr,"Error: failed to add line \"%s\" to dataset.\n",text);
		return;
	}
	// tokenize the pairs by space since that's the seperator given by count pairs ex: "a b" becomes "a" and "b". num_tokens should always return 3 here.
	size_t num_tokens = 0;
	Token** tokens = tokenize(dataset," ",&num_tokens);

	if(num_tokens != 3){
		fprintf(stderr, "Error: Failed to sperate token pairs %s\n",text);
		free_dataset(dataset);
		for(size_t i = 0; i < num_tokens;i++){
			free_token(tokens[i]);
		}
		free(tokens);
		return;
	}
	Token* merged_token = merge_tokens(tokens[0],tokens[1]);
	if(merged_token == NULL){
		fprintf(stderr,"Error: Failed to merge tokens %s and %s\n",tokens[0]->text, tokens[1]->text);
		free_dataset(dataset);
                for(size_t i = 0; i < num_tokens;i++){
                        free_token(tokens[i]);
                }
                free(tokens);
		return;
	}

	for(size_t i = 0; i < size - 1;i++){
		Token* current = tokenized_data[i];
		Token* next = tokenized_data[i + 1];
		if(strcmp(current->text, tokens[0]->text) == 0 && strcmp(next->text, tokens[1]->text) == 0){
			free_token(current);
			tokenized_data[i] = merged_token;

			free_token(next);
			tokenized_data[i + 1] = NULL;
			for(size_t j = i + 1; j < size - 1; j++){
				tokenized_data[j] = tokenized_data[j + 1];
			}
			size--;
			i--;
		}
	}

	// Cleanup
	free_dataset(dataset);
	for (size_t i = 0; i < num_tokens; i++) {
    		free_token(tokens[i]);
	}
	free(tokens);

}
// Code to implement BPE
//

void BPE(Tokenizer* tokenizer, Dataset* dataset){
	// Null check to avoid uneccessary seg fault.
	if(tokenizer == NULL || dataset ==NULL || dataset->num_lines == 0){
		fprintf(stderr,"Tokenizer or dataset is empty or NULL\n");
		return;
	}
	// Step 1: tokenized the dataset by characters
	size_t num_tokens = 0;
	Token** tokenized_data = tokenize(dataset,"",&num_tokens);
	if(tokenized_data == NULL || num_tokens == 0){
		fprintf(stderr,"Error: Could not tokenize dataset or zero token\n");
		return;
	}
	// Initialize the vocabulary to include tokens consisting of individual characters
	initialize_vocabulary(tokenizer,dataset);

	while(tokenizer->vocab_size < MAX_VOCAB_SIZE){
		count_pairs(tokenizer,tokenized_data,num_tokens);
		HashEntry* most_freq_pair = find_most_freq_pairs(tokenizer->pair_freqs);
		if(most_freq_pair == NULL){
			break; // no more frequent pairs left.
		}

		merge_most_freq_pair(tokenized_data, most_freq_pair,num_tokens);
		add_to_vocabulary(tokenizer,(const char*) most_freq_pair->key);
	}
}
