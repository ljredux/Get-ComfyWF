#include "comfy_meta.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif

void show_usage(const char *argv0);
int dir_exists(const char *path);
int file_exists(const char *path);
const char *get_basename(const char *argv0);
void get_workflows_path(char *out, size_t size);
void join_paths(char *out, size_t size, int count, ...);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        show_usage(argv[0]);
        return 1;
    }

    if (!file_exists(argv[1])) {
        printf("Error: File '%s' does not exist or is not a regular file.\n", argv[1]);
        show_usage(argv[0]);
        return 1;
    }

    // Open file & grab required metadata
    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    char *metadata = get_metadata(fp, "workflow");
    if (metadata) {
        printf("%s\n", metadata);
        free(metadata);
    } else {
        printf("No workflow found.\n");
    }

    fclose(fp);

    return 0;
}

void show_usage(const char *argv0) {
    printf("Usage: %s <file>\n", get_basename(argv0));
}

int dir_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

const char *get_basename(const char *argv0) {
    const char *name = strrchr(argv0, PATH_SEP[0]);
    return name ? name + 1 : argv0;
}

void get_workflows_path(char *out, size_t size) {
    const char *base = getenv("COMFYUI_PATH");
    if (!base) {
        out[0] = '\0';
        return;
    }

    join_paths(out, size, 4, base, "user", "default", "workflows");

    if (!dir_exists(out)) {
        out[0] = '\0';
        return;
    }
}

void join_paths(char *out, size_t size, int count, ...) {
    va_list args;
    va_start(args, count);

    out[0] = '\0';

    for (int i = 0; i < count; i++) {
        const char *part = va_arg(args, const char *);

        if (i > 0 && strlen(out) > 0 && out[strlen(out) - 1] != PATH_SEP[0]) {
            strncat(out, PATH_SEP, size - strlen(out) - 1);
        }

        strncat(out, part, size - strlen(out) - 1);
    }

    va_end(args);
}
