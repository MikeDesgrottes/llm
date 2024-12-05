#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dataset.h"

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
	if(dataset->lines[index] == NULL){
		return;
	}
	free(dataset->lines[index]);
	for(size_t i = index;i < dataset->num_lines - 1;i++){
		char* next = dataset->lines[i + 1];
		dataset->lines[i] = next;
		index++;
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
