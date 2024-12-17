#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "dataset.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

//File Creation and Management
//
//
char* get_file_name(const char* filepath){
	if(filepath == NULL || strlen(filepath) == 0) {
		fprintf(stderr,"Error: Empty filepath.\n");
		return NULL;
	}
	char* ptr = strrchr(filepath,"/");
	if(ptr){
		ptr++;
		if(ptr != '\0'){ 
			return strdup(ptr);
		}else{
			fprintf(stderr,"Error: Filepath %s is actually a directory.\n", filepath);
			return NULL;
		}
	}else{
		return strdup(filepath);
	}
}

bool validate_mode_pattern(const char* mode) {
    // First check length
    size_t len = strlen(mode);
    if (len == 0 || len > 3) return false;

    // First character must be r, w, or a
    if (mode[0] != 'r' && mode[0] != 'w' && mode[0] != 'a')
        return false;

    // If we have a second character
    if (len > 1) {
        // It can be either + or b or t
        if (mode[1] != '+' && mode[1] != 'b' && mode[1] != 't')
            return false;

        // If we have a third character
        if (len == 3) {
            // If second was +, third must be b or t
            if (mode[1] == '+' && mode[2] != 'b' && mode[2] != 't')
                return false;
            // If second was b or t, third must be +
            if ((mode[1] == 'b' || mode[1] == 't') && mode[2] != '+')
                return false;
        }
    }
    return true;
}
TextFile* create_text_file(const char* filepath, size_t initial_buffer_size){
	TextFile* file = (TextFile*)malloc(sizeof(TextFile));
	if(!file){
		fprintf(stderr,"Error while allocating space for new file %s.\n",filepath);
		return NULL;
	}
	file->filepath = strdup(filepath);
	if(!file->filepath){
		fprintf(stderr,"Error while duplicating filepath: %s\n",filepath);
		free(file);
		return NULL;
	}
	file->file_handle = NULL;
	file->buffer = NULL;
	file->buffer_size = initial_buffer_size;
	file->is_open = false;

	struct stat file_info;
	if(stat(file->filepath,&file_info) == 0){
		file->metadata.size = file_info.st_size;
		file->metadata.last_modified = file_info.st_mtime;
		file->metadata.filename = get_file_name(filepath);
		if(!file->metadata.filename) {
    			fprintf(stderr, "Error: Could not extract filename from %s\n", filepath);
    			free(file->filepath);
    			free(file);
    			return NULL;
		}
	}else{
		free(file->filepath);
		free(file);
		return NULL;
	}
	return file;
}

int open_text_file(TextFile* file, const char* mode){
	if(file == NULL || mode == NULL || strlen(mode) == 0){
		fprintf(stderr,"Error: Invalid arguments to open text file.\n");
		return -1; // failure
	}else if(file->is_open == true){
		return 0;
	}else if(validate_mode_pattern(mode) == false){
		fprintf(stderr,"Error: Invalid file mode.\n");
		return -1;// failure
	}

	FILE* tmp = fopen(file->filepath,mode);
	if(tmp){
		file->file_handle = tmp;
		file->is_open = true;
		return 0; // success
	}else{
		fprintf(stderr,"Error while opening file %s.\n",file->filepath);
		return -1; // Failure
	}
}

void close_text_file(TextFile* file){
	if(file == NULL){
		fprintf(stderr, "Error: Invalid file.\n");
		return;
	}else if(file->is_open == false){
		return; // do nothing file is already closed.
	}

	if(file->file_handle != NULL){
		if(fclose(file->file_handle) != 0) {
            		fprintf(stderr, "Warning: Error occurred while closing file: %s\n",
                    	file->filepath);
            		// Even if close failed, we still want to update our internal state
        	}
        	file->file_handle = NULL;
		file->is_open = false;
		return;
	}
}

void destroy_text_file(TextFile** file){
	if(file == NULL || *file == NULL){
		return; // do nothing file doesn't exists.

	}

	if((*file)->is_open == true){
		close_text_file(*file);
	}
	free((*file)->filepath);
	if((*file)->buffer){
		free((*file)->buffer);
	}
	
	if((*file)->metadata.filename){
		free((*file)->metadata.filename);
	}
	if((*file)->metadata.category){
		free((*file)->metadata.category);
	}

	free(*file);
	*file = NULL;
}

// Buffer Management:

void initialize_buffer(TextFile* file, size_t size){
	if(file == NULL || size == 0){
		fprintf(stderr,"Error: Invalid file or zero size.\n");
		return;//nothing to do here
	}

	if(file->buffer){
		clear_buffer(file);
	}
	char* tmp = malloc(sizeof(char)*size);
	if(tmp == NULL){
		fprintf(stderr,"Error: Could not allocate memory for buffer.\n");
		return;
	}

	file->buffer = tmp;
	file->buffer_size = size;
}

int resize_buffer(TextFile* file, size_t new_size){
	if(file == NULL || new_size == 0){
                fprintf(stderr,"Error: Invalid file or zero size.\n");
                return -1;//nothing to do here
	}

	char* tmp = realloc(file->buffer, sizeof(char)*new_size);
	if(!tmp){
		fprintf(stderr,"Error: Coould not reallocate enough memory for buffer.\n");
		return -1;
	}

	file->buffer = tmp;
	file->buffer_size = new_size;
	return 0;
}

void clear_buffer(TextFile* file){
	if(file == NULL || file->buffer == NULL){
		return;
	}

	free(file->buffer);
	file->buffer_size = 0;
	file->buffer = NULL;
}
Dataset *initialize_dataset(size_t initial_capacity) {
    Dataset *dataset = (Dataset *)malloc(sizeof(Dataset));
    if (!dataset) {
        perror("Failed to allocate memory for dataset");
        return NULL;
    }
    dataset->lines = (char **)malloc(initial_capacity * sizeof(char *));
    if (!dataset->lines) {
        perror("Failed to allocate memory for lines");
        free(dataset);
        return NULL;
    }
    dataset->num_lines = 0;
    dataset->capacity = initial_capacity;
    return dataset;
}


int add_line(Dataset *dataset, const char *line) {
    if (dataset->num_lines == dataset->capacity) {
        size_t new_capacity = dataset->capacity * 2;
        char **new_lines = (char **)realloc(dataset->lines, new_capacity * sizeof(char *));
        if (!new_lines) {
            perror("Failed to reallocate memory for lines");
            return -1;
        }
        dataset->lines = new_lines;
        dataset->capacity = new_capacity;
    }
    dataset->lines[dataset->num_lines] = strdup(line);
    if (!dataset->lines[dataset->num_lines]) {
        perror("Failed to duplicate line");
        return -1;
    }
    dataset->num_lines++;
    return 0;
}

void free_dataset(Dataset *dataset) {
    if (!dataset) return;
    for (size_t i = 0; i < dataset->num_lines; i++) {
        free(dataset->lines[i]);
    }
    free(dataset->lines);
    free(dataset);
}

int load_dataset_from_file(Dataset *dataset, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        return -1;
    }
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file)) {
        // Remove newline character if present
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        if (add_line(dataset, buffer) != 0) {
            fclose(file);
            return -1;
        }
    }
    fclose(file);
    return 0;
}

void remove_line(Dataset* dataset, size_t index){
	if(dataset == NULL ||dataset->lines == NULL || index >= dataset->num_lines){
		return;
	}
	if(dataset->lines[index] == NULL){
		return;
	}
	free(dataset->lines[index]);
	for(size_t i = index;i < dataset->num_lines - 1;i++){
		char* next = dataset->lines[i + 1];
		dataset->lines[i] = next;
	}
	dataset->lines[dataset->num_lines - 1] = NULL;
	dataset->num_lines--;
}

char* get_line(Dataset* dataset, size_t index){
	if(index == 0 || index > dataset->num_lines){
		return NULL;
	}

	return dataset->lines[index - 1];
}

void clear_dataset(Dataset* dataset){
	for(size_t i = 0; i < dataset->num_lines;i++){
		free(dataset->lines[i]);
	}
}

void update_line(Dataset* dataset,size_t line_number, char* line){
	free(dataset->lines[line_number - 1]);
	dataset->lines[line_number - 1] = line;
}
