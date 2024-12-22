#ifndef DATASET_H
#define DATASET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Dataset structure
typedef struct {
    char* filename;            // Name of the file
    size_t size;              // File size
    time_t last_modified;     // When file was last changed
    char* category;           // What category this file belongs to
} FileMetadata;

typedef struct {
    FILE* file_handle;          // The actual connection to the file on disk
    char* filepath;             // Full path to the file
    char* buffer;               // For buffered reading if needed
    size_t buffer_size;
    bool is_open;              // Track if file is currently open
    FileMetadata metadata;      // Information about the file
} TextFile;

typedef struct{
	TextFile** files;
	size_t num_files;
	size_t capacity;
	char* categoryname;
} Category;


typedef struct {
    Category **categories;       // Array of pointers to lines of text
    size_t num_categories;   // Number of lines in the dataset
    size_t capacity;    // Allocated capacity for the lines array
} Dataset;


//File Creation and Management:
TextFile* create_text_file(const char* filenamei, size_t initial_buffer_size);
int open_text_file(TextFile* file, const char* mode);
void close_text_file(TextFile* file);
void destroy_text_file(TextFile* file);
void reset_text_file(TextFile* file);

//Buffer Management:
//
//
void initialize_buffer(TextFile* file, size_t size);
int resize_buffer(TextFile* file, size_t new_size);
void clear_buffer(TextFile* file);
void flush_buffer(TextFile* file)
//Dataset functions
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

int get_modification_time(TextFile* file, time_t* time);
int get_file_size(TextFile* file, size_t* size);
void update_metadata(TextFile* file);
int load_dataset_from_directory(Dataset* dataset, const char* directory_path);
int add_category_to_dataset(Dataset* dataset, const char* category_name);
Dataset* create_dataset(size_t initial_capacity);

// Category Management.
//
int add_file_to_category(Category* category, const char* filepath);
Category* create_category(const char* name, size_t initial_capacity);
#endif
