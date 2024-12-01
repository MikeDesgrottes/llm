#include <stdio.h>
#include <stdlib.h>
#include "tokenizer.h"
#include "utils.h"

int main() {
    // Create a tokenizer
    Tokenizer* tokenizer = create_tokenizer(100);

    // Tokenize a sample sentence
    const char* text = "We are Klingons.";
    size_t num_tokens;
    char** tokens = tokenize(tokenizer, text, " .", &num_tokens);

    // Print tokens
    printf("Tokens:\n");
    for (size_t i = 0; i < num_tokens; i++) {
        printf("%s\n", tokens[i]);
        add_to_vocabulary(tokenizer, tokens[i]); // Add to vocabulary
        free(tokens[i]); // Free individual token
    }
    free(tokens);

    // Save vocabulary
    save_vocabulary("vocab/vocab.txt", tokenizer->vocabulary, tokenizer->vocab_size);

    // Print vocabulary
    printf("\nVocabulary:\n");
    for (size_t i = 0; i < tokenizer->vocab_size; i++) {
        printf("%s\n", tokenizer->vocabulary[i]);
    }

    // Free tokenizer
    free_tokenizer(tokenizer);
    return 0;
}

