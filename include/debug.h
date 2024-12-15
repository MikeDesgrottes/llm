// debug.h
#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

// Debug categories - using powers of 2 allows combining categories with bitwise OR
#define DEBUG_NONE          0
#define DEBUG_MEMORY        1    // Memory allocation/deallocation
#define DEBUG_TOKENIZER     2    // Tokenizer operations
#define DEBUG_VOCAB         4    // Vocabulary management
#define DEBUG_PAIRS         8    // Pair counting and merging
#define DEBUG_HASH_TABLE         16    // Hash table operations
#define DEBUG_ALL          31    // All categories enabled

// Set default debug level if not defined
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL DEBUG_NONE
#endif

// Main debug macro
#define DEBUG_PRINT(category, fmt, ...) \
    do { \
        if (DEBUG_LEVEL & category) { \
            fprintf(stderr, "[%s:%d] " fmt "\n", \
                __FILE__, __LINE__, ##__VA_ARGS__); \
        } \
    } while (0)

// Category-specific debug macros for cleaner code
#define DEBUG_MEM(fmt, ...)   DEBUG_PRINT(DEBUG_MEMORY, fmt, ##__VA_ARGS__)
#define DEBUG_TOK(fmt, ...)   DEBUG_PRINT(DEBUG_TOKENIZER, fmt, ##__VA_ARGS__)
#define DEBUG_VOC(fmt, ...)   DEBUG_PRINT(DEBUG_VOCAB, fmt, ##__VA_ARGS__)
#define DEBUG_PAIR(fmt, ...)  DEBUG_PRINT(DEBUG_PAIRS, fmt, ##__VA_ARGS__)
#define DEBUG_HASH(fmt, ...)  DEBUG_PRINT(DEBUG_HASH_TABLE, fmt, ##__VA_ARGS__)

#endif // DEBUG_H
