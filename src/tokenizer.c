#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokenizer.h"
#include "utils.h"
#include "hash_table.h"
#include "../include/config.h"
#include "../include/debug.h"


/*
 * tokenizer.c
 *
 * Implements tokenization for text processing and BPE (Byte Pair Encoding).
 * Main components:
 * - Character-level and delimiter-based tokenization
 * - Token management (creation, deletion, merging)
 * - Vocabulary tracking with token_map
 * - Pair frequency counting for BPE
 */



/* Token: Represents a single token in the vocabulary
 * - text: The actual token string
 * - length: Length of token text
 * - frequency: How often token appears
 */

/* Tokenizer: Main tokenizer state
 * - vocabulary: Array of unique tokens
 * - vocab_size: Current number of tokens
 * - max_vocab_size: Maximum allowed tokens
 * - pair_freqs: Tracks BPE pair frequencies
 * - token_map: Maps tokens to vocabulary indices
 */

/* tokenize(): Converts text into tokens
 * Modes:
 * - Character mode: splits into individual chars (empty delimiter)
 * - Delimiter mode: splits on specified delimiters
 * Adds '_' separator between tokens
 */
#define CLEANUP() {  \
	if (text) { \
		for (size_t k = 0; k < step; k++) {\
			if (text[k] != NULL) { \
				free(text[k]); \
			}\
		}\
		free(text);\
	}\
	free(copy); \
	for (size_t j = 0; j < count; j++) {\
		free_token(tokens[j]);\
	}\
	free(tokens);\
}

//TODO: Centralize the logic for resizing a tokenizer's vocabulary.
//	Functions to consider changing are resize_vocabulary, resize_hash_table, resize_dataset
//
//
//
// Create a tokenizer instance
Tokenizer* create_tokenizer(size_t max_vocab_size) {
    Tokenizer* tokenizer = (Tokenizer*)malloc(sizeof(Tokenizer));
    DEBUG_MEM("Allocating tokenizer structure at %p", (void*)tokenizer);

    DEBUG_VOC("Allocating vocabulary with initial size %zu", max_vocab_size);
    tokenizer->vocabulary = (Token**)calloc(max_vocab_size, sizeof(Token*));
    if (tokenizer->vocabulary == NULL) {
        DEBUG_MEM("Failed to allocate memory for vocabulary");
        free(tokenizer);
        return NULL;
    }
    tokenizer->vocab_size = 0;
    tokenizer->max_vocab_size = max_vocab_size;
    tokenizer->pair_freqs = create_hash_table(INITIAL_PAIR_FREQ_SIZE);
    tokenizer->token_map = create_hash_table(max_vocab_size);
    if (!tokenizer->pair_freqs || !tokenizer->token_map) {
    	// Clean up and return NULL
   	if (tokenizer->pair_freqs) free_hash_table(tokenizer->pair_freqs);
   	if (tokenizer->token_map) free_hash_table(tokenizer->token_map);
    	free(tokenizer->vocabulary);
    	free(tokenizer);
    	return NULL;
    }
    tokenizer->token_map->allow_resize = false;
    return tokenizer;
}

// Add a token to the vocabulary
void add_to_vocabulary(Tokenizer* tokenizer, const char* token) {
	// Sanity check
	if(tokenizer ==NULL || token == NULL){
		DEBUG_VOC("Invalid tokenizer or tokens");
		return;
	}
    // Check if the token is already in the vocabulary
        HashTable* table = tokenizer->token_map;
	// sanity checks
	if(table == NULL){
		DEBUG_VOC("Error: Invalid token map");
		return;
	}

	Iterator* it = create_iterator(table);
	if (!it) {  // Should check if iterator creation succeeded
    		DEBUG_VOC("Error: Could not create iterator");
   		return;
	}
	while(has_next(it)) {
    		HashEntry* entry = get_next(it);
		if(!entry) break;
    		if(table->ops.compare_keys(entry->key, (void*)token) == 0) {
        		DEBUG_VOC("Token %s already in the vocabulary. Frequency: %zu.",
            		tokenizer->vocabulary[entry->value]->text, 
            		tokenizer->vocabulary[entry->value]->frequency);
        		tokenizer->vocabulary[entry->value]->frequency++;
        		free_iterator(it);
        		return;
    		}
	}
	free_iterator(it);	
	Token* new_token = create_token(token);
    if (new_token == NULL) {
        DEBUG_MEM("Error allocating memory for new token");
        return;  // Handle memory allocation failure for token
    }
	new_token->frequency = 1;
	DEBUG_VOC("New Token created Text: %s Frequency: %zu.",new_token->text, new_token->frequency);
    // Expand the vocabulary if needed

    if (tokenizer->vocab_size == 0) {
	for(size_t i = 0; i< tokenizer->max_vocab_size;i++){
		tokenizer->vocabulary[i] = NULL;
	}
    } else if (tokenizer->vocab_size >= tokenizer->max_vocab_size) {
	    DEBUG_VOC("Error: Vocabulary at maximum capacity");
	    return;
    }

    size_t hash_index = table->ops.hash_function((const void*)token) %tokenizer->max_vocab_size;
    //size_t index1 = hash2(token,tokenizer->max_vocab_size);


    size_t step = 0;
    size_t final_index = (hash_index + step*step) % tokenizer->max_vocab_size;
    while(step < tokenizer->max_vocab_size){
	final_index = (hash_index + step*step) % tokenizer->max_vocab_size;

    	if(tokenizer->vocabulary[final_index] == NULL){
		tokenizer->vocabulary[final_index] = new_token;
		tokenizer->vocab_size++;
        	int result = insert_into_token_map(tokenizer->token_map, token, final_index);
		if (result != 0) {
    		// Rollback changes in vocabulary if token_map insertion fails
    			DEBUG_VOC("Error inserting token \"%s\" into token_map",token);
			free_token(tokenizer->vocabulary[final_index]);
    			tokenizer->vocabulary[final_index] = NULL;
			tokenizer->vocab_size--;
		}
		return;
	}
	step++;
    }
    DEBUG_VOC("Error: Unable to find an empty slot in vocabulary after probing");
    free_token(new_token);
}
/*
 * token_map is used to track occupied slots in the vocabulary array.
 * Unlike a traditional hash table, we use sequential insertion since:
 * 1. The map will be much smaller than vocabulary capacity
 * 2. Sequential storage optimizes iteration performance
 * 3. Primarily used for quick lookup of existing tokens and their indices
 */
int insert_into_token_map(HashTable* table, const char* key, size_t value){
	// Sanity Check
	if(table == NULL || key == NULL){
		DEBUG_VOC("Error: Invalid hash table or key");
		return -1;
	}
	size_t v = value;
	HashEntry* entry = create_hash_entry((const void*)key, sizeof(char*), (const void*)&v, sizeof(size_t));
	if(entry == NULL){
		DEBUG_MEM("Error: Could not allocate enough memory for hash entry");
		return -1;
	}

	// Check to see if the key is already in the token_map. If so, update its value.
	HashIterator* it  = create_iterator(table);
	if(!it){
		DEBUG_VOC("Error: Could not create a HashTableIterator.\n");
		return -1;
	}

	while(has_next(it)){
		HashEntry* entry1 = get_next(it);
		if(!entry){
			return -1;//  NULL from get_next indicate error not empty slots.
		}
		if(table->ops.compare_keys((const void*)entry1->key, (const void*)key) == 0){
			table->ops.free_value(table->entries[it->current_index]->value);
			table->entries[it->current_index]->value = table->ops.duplicate_value((const void*)&v);
			free_iterator(it);
			return 0;
		}
	}
	free_iterator(it);	

	// Ensure the table has enough capacity
    if (table->size >= table->capacity) {
	    if(table->allow_resize == false){
	    	free_hash_entry(entry);
		return -1;
	    }
        size_t new_capacity = table->capacity * 2;
        HashEntry** new_entries = realloc(table->entries, new_capacity * sizeof(HashEntry*));
        if (new_entries == NULL) {
            DEBUG_MEM("Error: Could not resize hash table");
            return -1;
        }
        // Initialize new slots to NULL
        for (size_t i = table->capacity; i < new_capacity; i++) {
            new_entries[i] = NULL;
        }
        table->entries = new_entries;
        table->capacity = new_capacity;
    }
	entry->key = table->ops.duplicate_key((const void*)key);
	if(entry->key == NULL){
		DEBUG_MEM("Error duplicating key %s",key);
		free(entry);
		return -1;
	}
	//entry->value = value;
	entry->value = table->ops.duplicate_value((const void*)&v);
	table->entries[table->size] = entry; // sequential insertion.
	table->size++;
	return 0; // success
}

// Helper function to resize token array
Token** resize_tokens(Token** tokens, size_t* capacity) {
    size_t new_capacity = (*capacity) * 2;
    Token** resized_tokens = (Token**)realloc(tokens, sizeof(Token*) * new_capacity);
    if (resized_tokens == NULL) {
        DEBUG_MEM("Error: Failed to resize tokens array.");
        return NULL;
    }
    *capacity = new_capacity;
    return resized_tokens;
}

// Helper function to add a token
int add_token(Token** tokens, size_t* count, size_t* capacity, Token* token) {
    if (*count >= *capacity) {
        tokens = resize_tokens(tokens, capacity);
        if (tokens == NULL) {
            return -1; // Failed to resize
        }
    }
    tokens[(*count)++] = token;
    return 0;
}
// Tokenize input text:wq
//
Token** tokenize(const TextFile* file, const char* delimiters, size_t* num_tokens) {
	// Sanity checks
       	if (delimiters == NULL || file == NULL) { return NULL; }
       	size_t count = 0; 
	size_t capacity = 100;
	size_t step = 0;
	
	// Initial capacity for token array 
	Token** tokens = (Token**)calloc(capacity, sizeof(Token*));	
	char** text = NULL;
        char** line = (char**)malloc(sizeof(char*));
	if(!line) return NULL;

	int res;	
	if (tokens == NULL) { 
		fprintf(stderr, "Error: failed to allocate memory for tokens\n"); 
		return NULL; 
	}



	while ((res = read_line(file,line)) == 0) { 
		//char* line = dataset->lines[i]; 
		if (line == NULL || strlen(line) == 0) { continue; } // skip empty lines 
		char* copy = strdup(*line); 
		if (copy == NULL) { 
			fprintf(stderr, "Error: Failed to duplicate line\n"); 
			CLEANUP(); 
			return NULL; 
		}


		char* token; 
		if (strlen(delimiters) == 0) { 
			// Character-level tokenization 
			token = strtok(copy, " "); 
			while (token != NULL) { 
				step = 0; 
				text = split_by_character((const char*)token); 
				if (text == NULL) { 
					fprintf(stderr, "Error: Failed to split token into characters\n"); 
					CLEANUP(); 
					return NULL; 
				} 
				while (text[step] != NULL) { 
					Token* tok = create_token(text[step]); 
					if (tok == NULL) { 
						fprintf(stderr, "Error: Failed to create token\n"); 
						CLEANUP(); 
						return NULL; 
					} 
					if (count >= capacity) { 
						capacity *= 2; 
						Token** tmp_tokens = (Token**)realloc(tokens, sizeof(Token*) * capacity); 
						if (tmp_tokens == NULL) {
							fprintf(stderr, "Error: Failed to reallocate memory for tokens\n"); 
							free_token(tok); CLEANUP(); 
							return NULL;
					       	} 
						tokens = tmp_tokens; 
					} 
					tokens[count++] = tok; 
					step++; 
				} 
				// Free text and its elements 
				for (size_t k = 0; k < step; k++) { 
					if (text[k] != NULL) { 
						free(text[k]); 
					} 
				} 
				free(text); 
				text = NULL; 
				Token* separator = create_token("_"); 
				if (separator == NULL) { 
					fprintf(stderr, "Error: Failed to create separator token\n"); 
					CLEANUP(); 
					return NULL; 
				} 
				if (count >= capacity) { 
					capacity *= 2; 
					Token** tmp_tokens = (Token**)realloc(tokens, sizeof(Token*) * capacity); 
					if (tmp_tokens == NULL) 
					{ 
						fprintf(stderr, "Error: Failed to reallocate memory for tokens\n"); 
						free_token(separator); CLEANUP(); return NULL; 
					} 
					tokens = tmp_tokens; 
				} 
				tokens[count++] = separator; 
				token = strtok(NULL, " "); 
			} 
		} else { 
			// Normal delimiter-based tokenization 
			token = strtok(copy, delimiters); 
			while (token != NULL) { 
				Token* tok = create_token(token); 
				if (tok == NULL) { 
					fprintf(stderr, "Error: Failed to create token\n"); 
					CLEANUP(); 
					return NULL; 
				} if (count >= capacity) { 
					capacity *= 2; 
					Token** tmp_tokens = (Token**)realloc(tokens, sizeof(Token*) * capacity); 
					if (tmp_tokens == NULL) { 
						fprintf(stderr, "Error: Failed to reallocate memory for tokens\n"); 
						free_token(tok); CLEANUP(); 
						return NULL; 
					} 
					tokens = tmp_tokens; 
				} 
				tokens[count++] = tok; 
				Token* separator = create_token("_"); 
				if (separator == NULL) { 
					fprintf(stderr, "Error: Failed to create separator token\n"); 
					CLEANUP(); 
					return NULL; 
				} 
				if (count >= capacity) { 
					capacity *= 2; 
					Token** tmp_tokens = (Token**)realloc(tokens, sizeof(Token*) * capacity); 
					if (tmp_tokens == NULL) { 
						fprintf(stderr, "Error: Failed to reallocate memory for tokens\n"); 
						free_token(separator); 
						CLEANUP(); 
						return NULL; 
					} tokens = tmp_tokens; 
				} 
				tokens[count++] = separator; 
				token = strtok(NULL, delimiters); 
			} 
		}
	       free(*line);
       		free(line);	       
		free(copy); 
	} 
	if(res == -1){
		fprintf(stderr,"Error while reading line in the textfile.\n");
	}else if(res == -2){
		free(*line);
		free(line);	
	}
	tokens[count] = NULL; 
	*num_tokens = count; 
	return tokens;
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

void free_tokens(Token** tokens, size_t num_tokens) {
    for (size_t i = 0; i < num_tokens; i++) {
	    if(tokens[i] != NULL){
        	free_token(tokens[i]);
	    }
    }
    free(tokens);
}
// Free the tokenizer and its resources
void free_tokenizer(Tokenizer** tokenizer) {
	// Sanity checks
	if((*tokenizer) == NULL){
		return; // skip Null tokenizers.
	}

	for (size_t i = 0; i < (*tokenizer)->max_vocab_size; i++) {
    		if ((*tokenizer)->vocabulary[i] != NULL) {
			DEBUG_MEM("Freeing the vocabulary at %zu ",i);
        		free_token((*tokenizer)->vocabulary[i]);
        		(*tokenizer)->vocabulary[i] = NULL;
    		}
	}
	free_hash_table((*tokenizer)->pair_freqs);
	(*tokenizer)->pair_freqs = NULL;
	free_hash_table((*tokenizer)->token_map);
    	(*tokenizer)->token_map = NULL;
	DEBUG_MEM("Freeing the Vocabulary itself %p", (void*)(*tokenizer)->vocabulary);
    free((*tokenizer)->vocabulary);
    (*tokenizer)->vocabulary = NULL;
    free((*tokenizer));
    *tokenizer = NULL;
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
		DEBUG_VOC("Adding token %s to vocabulary.",tokens[i]->text);

        	add_to_vocabulary(tokenizer,(const char*) tokens[i]->text);

		DEBUG_VOC("Token %s added to vocabulary.\n",tokens[i]->text);
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
			return;
        	}

        	// Update the pair frequency
        	int frequency = get_value(tokenizer->pair_freqs, pair_key, strcmp);
        	if (frequency >= 0) {
            		//insert_into_hash_table(tokenizer->pair_freqs, pair_key, frequency + 1);
			increment_frequency_hash_table(tokenizer->pair_freqs, pair_key);
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
	Token* token = (Token*)calloc(1,sizeof(Token));
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
	DEBUG_TOK("Token created: %p, text: %s", (void*)token, token->text);
	return token;
}


void free_token(Token* token){
	if(token != NULL){
		if(token->text != NULL){
			free(token->text);
			token->text = NULL;
			DEBUG_TOK("Token text freed: %p", (void*)token);
		}
		free(token);
	}
        DEBUG_TOK("Token freed: %p", (void*)token);
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

char*  merge_most_freq_pair(Token** tokenized_data, HashEntry* most_freq_pair, size_t* size){
	char* text = strdup(most_freq_pair->key);

	if(!text){
		fprintf(stderr,"Error while duplicating most frequent pair\n");
		return NULL;
	}
	Token* token1 = create_token(strtok(text," "));
	Token* token2  = create_token(strtok(NULL, " "));

	if(!token1 || !token2){
		fprintf(stderr, "Error while tokenizing most frequent pair\n");
		free(text);
		if(token1) free_token(token1);
		if(token2) free_token(token2);
		return NULL;
	}
	Token* merged_token = merge_tokens(token1,token2);
	if(merged_token == NULL){
		fprintf(stderr,"Error: Failed to merge tokens %s and %s\n",token1->text, token2->text);
		free_token(token1);
		free_token(token2);
		return NULL;
	}

	for(size_t i = 0; i < *size - 1;i++){
		Token* current = tokenized_data[i];
		Token* next = tokenized_data[i + 1];

		if(current && next && strcmp(current->text, token1->text) == 0 && strcmp(next->text, token2->text) == 0){
			free_token(current);
			tokenized_data[i] = create_token(merged_token->text);

			free_token(next);
			tokenized_data[i + 1] = NULL;
			for(size_t j = i + 1; j < *size - 1; j++){
				tokenized_data[j] = tokenized_data[j + 1];
			}
			tokenized_data[*size - 1] = NULL;
			(*size)--;
			i--;
		}
	}
	free_token(token1);
	free_token(token2);
	//tokenized_data[size] = NULL;
	// Cleanup
	//free_dataset(dataset);
	free(text);
	char* result = strdup(merged_token->text);
	if(!result){
		DEBUG_MEM("Failed to duplicate merged tokens string.");
		return NULL;
	}

	free_token(merged_token);
	return result;
	//free(tokens);

}

Token* create_token_with_frequency(const char* text, size_t freq){
	Token* res = create_token(text);
	if(!res){ DEBUG_TOK("Error could not create new token."); return NULL;}

	res->frequency = freq;
	return res;
}


void add_merged_token(Tokenizer* tokenizer, const char* text, size_t freq){
	int index = get_value(tokenizer->token_map, text, strcmp);

	if(index >=0){
		tokenizer->vocabulary[index]->frequency += freq;
	}else{
		index = hash_function(text,tokenizer->max_vocab_size);
		size_t step = 0;
		while(step < tokenizer->max_vocab_size){
			size_t probing_index = (index + step*step) % tokenizer->max_vocab_size;
			if(tokenizer->vocabulary[probing_index] == NULL){
				Token* token = create_token_with_frequency(text, freq);
				if(!token){ DEBUG_TOK("Error: Failed to create token for %s", text); return;}
				tokenizer->vocabulary[probing_index] = token;
				insert_into_token_map(tokenizer->token_map, text, probing_index);
				tokenizer->vocab_size++;
				return;
			}
			step++;
		}
		DEBUG_TOK("Failed to add merged token %s to the vocabulary",text);
	}

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
	
	printf("Starting BPE with %zu lines of text\n", dataset->num_lines);
    	printf("Initial tokens: %zu\n", num_tokens);
	
	// Initialize the vocabulary to include tokens consisting of individual characters
	DEBUG_TOK("Initializing Vocabulary...");

	initialize_vocabulary(tokenizer,dataset);
	size_t merges = 0;
	DEBUG_TOK("Vocabulary initialized.");
	while(tokenizer->vocab_size < MAX_VOCAB_SIZE){
		count_pairs(tokenizer,tokenized_data,num_tokens);
		HashEntry* most_freq_pair = find_most_freq_pairs(tokenizer->pair_freqs);
		if(most_freq_pair == NULL){
			break; // no more frequent pairs left.
		}

		char* res  = merge_most_freq_pair(tokenized_data, most_freq_pair,&num_tokens);
		if(res == NULL){
			DEBUG_MEM("Error: Failed to merged most frequent pair tokens");
			free_tokens(tokenized_data,num_tokens);
			return;
		}
		add_merged_token(tokenizer,(const char*) res, most_freq_pair->value);
		free(res);

		if(merges % 1000 == 0) {  // Print every 1000 merges
