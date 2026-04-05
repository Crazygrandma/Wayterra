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

    char* buffer = malloc(size + 1);
    if (!buffer) {
        fclose(f);
        return NULL;
    }

    size_t read = fread(buffer, 1, size, f);
    if (read != size) {
        printf("Failed to read full file: %s\n", path);
        free(buffer);
        fclose(f);
        return NULL;
    }

    buffer[size] = '\0'; // null-terminate
    fclose(f);
    return buffer;
}
