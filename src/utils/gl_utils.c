#define STB_IMAGE_IMPLEMENTATION
#include "gl_utils.h"

#include <stdio.h>

GLuint load_texture(const char *path) {
    int x, y, n;
    unsigned char *data = stbi_load(path, &x, &y, &n, 0);

    if (data == NULL) {
        fprintf(stderr, "Failed to load image: %s\n", stbi_failure_reason());
        return 0; // 0 = invalid texture
    }

    GLenum format;
    if (n == 3)
        format = GL_RGB;
    else if (n == 4)
        format = GL_RGBA;
    else {
        fprintf(stderr, "Unsupported format: %d\n", n);
        stbi_image_free(data);
        return 0;
    }

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, format, x, y, 0,
                 format, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    stbi_image_free(data);

    return tex;
}

GLuint compile_shader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, NULL, log);
        printf("Shader compile error: %s\n", log);
    }
    return shader;
}
