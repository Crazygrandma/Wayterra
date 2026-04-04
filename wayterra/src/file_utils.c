#include "file_utils.h"

char* load_file(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        printf("Failed to open file: %s\n", path);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char* buffer = (char*)malloc(size + 1);
    if (!buffer) {
        fclose(f);
        return NULL;
    }

    
    buffer[size] = '\0';

    fclose(f);
    return buffer;
}
