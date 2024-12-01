#ifndef UTILS_H
#define UTILS_Hsi

#include <stddef.h>

// Function to load a vocabulary from a file
char** load_dataset(const char* file_path, size_t* lines);

// Function to save a vocabulary to a file
void save_dataset(const char* file_path, char** dataset, size_t lines);
// function to split a dataset into character level tokems


#endif // UTILS_H

