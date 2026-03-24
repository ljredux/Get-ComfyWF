#include "comfy_meta.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

const char *get_basename(const char *argv0);
void show_usage(const char *argv0);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        show_usage(argv[0]);
        return 1;
    }

    struct stat st;
    if (stat(argv[1], &st) != 0 || !S_ISREG(st.st_mode)) {
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

const char *get_basename(const char *argv0) {
    const char *name = strrchr(argv0, '/');
#ifdef _WIN32
    if (!name) name = strrchr(argv0, '\\');
#endif
    return name ? name + 1 : argv0;
}

void show_usage(const char *argv0) {
    printf("Usage: %s <file>\n", get_basename(argv0));
}