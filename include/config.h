#ifndef CONFIG_H
#define CONFIG_H

#define MAX_VOCAB_SIZE 10000
#define MAX_TOKEN_LEN 32
#define PAD_TOKEN "<PAD>"
#define UNK_TOKEN "<UNK>"
#define INITIAL_VOCAB_SIZE (1 << 20)  // ~1 million tokens
#define INITIAL_PAIR_FREQ_SIZE 300
#endif

