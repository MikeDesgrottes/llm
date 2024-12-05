#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Dataset structure
typedef struct {
    char **lines;       // Array of pointers to lines of text
    size_t num_lines;   // Number of lines in the dataset
    size_t capacity;    // Allocated capacity for the lines array
} Dataset;

Dataset *initialize_dataset(size_t initial_capacity);
int add_line(Dataset *dataset, const char *line);
void free_dataset(Dataset *dataset);
int load_dataset_from_file(Dataset *dataset, const char *filename);
void print_dataset(const Dataset *dataset);
void remove_line(Dataset* dataset, size_t index);
char* get_line(Dataset* dataset, size_t index);
void clear_dataset(Dataset* dataset);
size_t search_data(Dataset* dataset, char* target, int (*cmp_func)(char*, char*));
size_t* search_data_all(Dataset* dataset, char* target, int (*cmp_func)(char*, char*), size_t* match_count);
