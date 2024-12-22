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

int flush_buffer(TextFile* file){
	if (!file || !file->is_open) {
    		return -1; // Can't flush if we don't have an open file
	}

	// We need to have a buffer with actual content
	if (!file->buffer || file->buffer_size == 0) {
    		return 0; // Nothing to flush (this is not an error)
	}

	// Attempt to write the entire buffer to disk
	if (fwrite(file->buffer, 1, file->buffer_size, file->file_handle) != file->buffer_size) {
    		// If we couldn't write everything, something went wrong
    		return -1;
	}

	// Make sure the data actually gets written to disk
	fflush(file->file_handle);

	// Since we've written everything to disk, we can clear the buffer
	clear_buffer(file);
	return 0;
}

int read_next_chunk(TextFile* file, size_t chunk_size){
	// Check file validity
	if (!file || !file->is_open || !file->file_handle) {
	    return -1; // Can't read from invalid/closed file
	}

	// Verify chunk_size is reasonable
	if (chunk_size == 0) {
	    return -1; // Can't read zero bytes
	}

	// If buffer doesn't exist or is too small
	if (!file->buffer || file->buffer_size < chunk_size) {
    	// Need to either initialize or resize buffer
    		if (resize_buffer(file, chunk_size) != 0) {
       			return -1; // Buffer allocation failed
		}
	}

	size_t bytes_read = fread(file->buffer, 1, chunk_size, file->file_handle);
	if (bytes_read == 0) {
    		if (feof(file->file_handle)) {
        		return -2; // End of file reached
    		} else if (ferror(file->file_handle)) {
        		return -1; // Read error occurred
    		}
	}

	return bytes_read;
}

int read_line(TextFile* file, char** line){
// Input validation
    if (!file || !file->is_open || !file->file_handle || !line) {
        return -1;
    }

    // Initialize or clear the line buffer
    size_t buffer_size = 256;  // Initial size, can grow
    *line = malloc(buffer_size);
    if (!*line) {
        return -1;
    }

    // Read character by character
    size_t pos = 0;
    int c;
    while ((c = fgetc(file->file_handle)) != EOF) {
        if (pos + 1 >= buffer_size) {
            // Need more space
            buffer_size *= 2;
            char* temp = realloc(*line, buffer_size);
            if (!temp) {
                free(*line);
                *line = NULL;
                return -1;
            }
            *line = temp;
        }
        
        (*line)[pos++] = c;
        if (c == '\n') {
            break;
        }
    }

    if (pos == 0 && c == EOF) {
        free(*line);
        *line = NULL;
        return -2;  // EOF reached
    }

    (*line)[pos] = '\0';
    return 0;  // Success	
}


// Metadata Operations:

void update_metadata(TextFile* file){
// Input validation
    if (!file || !file->filepath) {
        return -1;
    }

    struct stat file_info;
    if (stat(file->filepath, &file_info) != 0) {
        return -1;  // stat failed
    }

    // Free old filename if it exists
    if (file->metadata.filename) {
        free(file->metadata.filename);
    }

    // Update metadata
    file->metadata.size = file_info.st_size;
    file->metadata.last_modified = file_info.st_mtime;
    file->metadata.filename = get_file_name(file->filepath);
    
    if (!file->metadata.filename) {
        fprintf(stderr, "Error: Could not extract filename from %s\n", file->filepath);
        return -1;
    }

    return 0;  // Success	
}

int get_file_size(TextFile* file, size_t* size) {
    if (!file) {
        fprintf(stderr, "Error: Invalid file.\n");
        return -1;
    }
    
    if (!file->is_open) {
        fprintf(stderr, "Error: File is not open.\n");
        return -1;
    }

    // Update metadata to ensure we have current size
    if (update_metadata(file) != 0) {
        fprintf(stderr, "Error: Could not update file metadata.\n");
        return -1;
    }

    *size = file->metadata.size;
    return 0;
}

int get_modification_time(TextFile* file, time_t* time){
	if(!file){
		fprintf(stderr,"Error: Invalid file.\n");
		return -1;
	}

	if(!file->is_open){
		fprintf(stderr,"Error: File is not open.\n");
		return -1;
	}

	if(update_metadata(file) != 0){
		fprintf(stderr, "Error: Could not update file metadata.\n");
		return -1;
	}

	*time = file->metadata.last_modified;
	return 0;
}	


// Dataset Management
//

// Category Management
Category* create_category(const char* name, size_t initial_capacity) {
    Category* category = malloc(sizeof(Category));
    if (!category) {
        fprintf(stderr, "Error: Failed to allocate category.\n");
        return NULL;
    }

    category->files = malloc(sizeof(TextFile*) * initial_capacity);
    if (!category->files) {
        fprintf(stderr, "Error: Failed to allocate files array.\n");
        free(category);
        return NULL;
    }

    category->categoryname = strdup(name);
    if (!category->categoryname) {
        fprintf(stderr, "Error: Failed to duplicate category name.\n");
        free(category->files);
        free(category);
        return NULL;
    }

    category->num_files = 0;
    category->capacity = initial_capacity;
    return category;
}

int add_file_to_category(Category* category, const char* filepath) {
    if (!category || !filepath) {
        return -1;
    }

    if (category->num_files >= category->capacity) {
        size_t new_capacity = category->capacity * 2;
        TextFile** new_files = realloc(category->files,
                                     sizeof(TextFile*) * new_capacity);
        if (!new_files) return -1;
        category->files = new_files;
        category->capacity = new_capacity;
    }

    TextFile* file = create_text_file(filepath, 1024); // Default buffer size
    if (!file) return -1;

    category->files[category->num_files++] = file;
    return 0;
}


// Dataset Management
Dataset* create_dataset(size_t initial_capacity) {
    Dataset* dataset = malloc(sizeof(Dataset));
    if (!dataset) return NULL;

    dataset->categories = malloc(sizeof(Category*) * initial_capacity);
    if (!dataset->categories) {
        free(dataset);
        return NULL;
    }

    dataset->num_categories = 0;
    dataset->capacity = initial_capacity;
    return dataset;
}

int add_category_to_dataset(Dataset* dataset, const char* category_name) {
    if (!dataset || !category_name) return -1;

    if (dataset->num_categories >= dataset->capacity) {
        size_t new_capacity = dataset->capacity * 2;
        Category** new_cats = realloc(dataset->categories,
                                    sizeof(Category*) * new_capacity);
        if (!new_cats) return -1;
        dataset->categories = new_cats;
        dataset->capacity = new_capacity;
    }

    Category* category = create_category(category_name, 10); // Default initial capacity
    if (!category) return -1;

    dataset->categories[dataset->num_categories++] = category;
    return 0;
}

int load_dataset_from_directory(Dataset* dataset, const char* directory_path) {
    DIR* dir = opendir(directory_path);
    if (!dir) return -1;
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && 
            strcmp(entry->d_name, ".") != 0 && 
            strcmp(entry->d_name, "..") != 0) {
            
            // Each subdirectory is a category
            add_category_to_dataset(dataset, entry->d_name);
            
            // Build path to category directory
            char category_path[PATH_MAX];
            snprintf(category_path, PATH_MAX, "%s/%s", 
                    directory_path, entry->d_name);
            
            // Get the newly added category
            Category* category = dataset->categories[dataset->num_categories - 1];
            
            // Load files in the category
            DIR* cat_dir = opendir(category_path);
            if (cat_dir) {
                struct dirent* file_entry;
                while ((file_entry = readdir(cat_dir)) != NULL) {
                    if (file_entry->d_type == DT_REG) {
                        char file_path[PATH_MAX];
                        snprintf(file_path, PATH_MAX, "%s/%s", 
                                category_path, file_entry->d_name);
                        add_file_to_category(category, file_path);
                    }
                }
                closedir(cat_dir);
            }
        }
    }
    closedir(dir);
    return 0;
}

void free_dataset(Dataset* dataset) {
    if (!dataset) return;
    
    for (size_t i = 0; i < dataset->num_categories; i++) {
        Category* category = dataset->categories[i];
        if (category) {
            for (size_t j = 0; j < category->num_files; j++) {
                destroy_text_file(&category->files[j]);
            }
            free(category->files);
            free(category->categoryname);
            free(category);
        }
    }
    free(dataset->categories);
    free(dataset);
}

