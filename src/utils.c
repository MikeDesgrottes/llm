#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utils.h>

// Load vocabulary from a file
char** load_dataset(const char* file_path, size_t* lines) {
    FILE* file = fopen(file_path, "r");
    if (!file) {
        perror("Failed to open file");
        return NULL;
    }

    char** dataset = NULL;
    char buffer[256];
    size_t count = 0;
    size_t capacity = 10;

    while (fgets(buffer, sizeof(buffer), file)) {
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline
	if(count >= capacity){
		capacity *=2;
        	char** tmp_dataset = (char**)realloc(dataset, sizeof(char*) * (count + 1));
		if(tmp_dataset == NULL){
			fprintf(stderr,"Error: Failed to allocate enough memory for a new dataset\n");
			return NULL;
		}

		dataset = tmp_dataset;
	}
        dataset[count++] = strdup(buffer);
    }

    fclose(file);
    *lines = count;
    return dataset;
}

// Save vocabulary to a file
void save_dataset(const char* file_path, char** dataset, size_t lines) {
    FILE* file = fopen(file_path, "w");
    if (!file) {
        perror("Failed to open file");
        return;
    }

    for (size_t i = 0; i < lines; i++) {
        fprintf(file, "%s\n", dataset[i]);
    }

    fclose(file);
}

