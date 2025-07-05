#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

void* reallocate(void* pointer, int new_size) {
    if (new_size == 0) {
        free(pointer);
        return NULL;
    }

    void* result = realloc(pointer, new_size);
    if (result == NULL) exit(1);
    return result;
}

char* file_read(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "error: failed to open file: %s\n", filename);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    int length = ftell(file);
    rewind(file);

    char* content = (char*)malloc(sizeof(char) * length + 1);
    if (content == NULL) {
        fprintf(stderr, "error: failed to allocate memory for file: %s\n", filename);
        exit(1);
    }

    fread(content, 1, length, file);
    content[length] = '\0';

    fclose(file);

    return content;
}
