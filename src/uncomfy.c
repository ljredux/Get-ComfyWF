#include "comfy_meta.h"
#include <limits.h>
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
char *get_workflow_filename(const char *input_filename);
char *get_workflows_path(void);
int save_workflow(const char *input_filename, const char *jsondata);

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
        save_workflow(argv[1], metadata);
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

// take input filename, strip path, replace extension with .json
char *get_workflow_filename(const char *input_filename) {
    const char *base = get_basename(input_filename);

    const char *dot = strrchr(base, '.');
    size_t base_len = dot ? (size_t)(dot - base) : strlen(base);

    size_t new_len = base_len + 5; // ".json"

    char *result = malloc(new_len + 1);
    if (!result) return NULL;

    memcpy(result, base, base_len);
    memcpy(result + base_len, ".json", 5);

    result[new_len] = '\0';

    return result;
}

// If COMFYUI_PATH env var exists, assemble & return the workflows path,
// otherwise, return the current working path.
char *get_workflows_path(void) {
    const char *base = getenv("COMFYUI_PATH");

    char *result = malloc(PATH_MAX);
    if (!result) return NULL;

    if (base) {
        int n = snprintf(result, PATH_MAX, "%s" PATH_SEP "user" PATH_SEP "default" PATH_SEP "workflows", base);

        if (n > 0 && n < PATH_MAX && dir_exists(result)) {
            return result;
        }
    }

    char *cwd = getcwd(NULL, 0);
    if (!cwd) {
        result[0] = '\0';
        return result;
    }

    snprintf(result, PATH_MAX, "%s", cwd);

    free(cwd);
    return result;
}

int save_workflow(const char *input_filename, const char *jsondata) {

    if (!input_filename || !jsondata) {
        return -1;
    }

    char *workflows_path = get_workflows_path();
    char *workflow_filename = get_workflow_filename(input_filename);

    if (!workflows_path || !workflow_filename) {
        free(workflows_path);
        free(workflow_filename);
        return -1;
    }

    char full_path[PATH_MAX];
    int n = snprintf(full_path, sizeof(full_path), "%s" PATH_SEP "%s", workflows_path, workflow_filename);

    free(workflows_path);
    free(workflow_filename);

    if (n < 0 || n >= (int)sizeof(full_path)) {
        return -1;
    }

    FILE *fp = fopen(full_path, "wb");
    if (!fp) {
        perror("fopen");
        return -1;
    }

    size_t len = strlen(jsondata);
    if (fwrite(jsondata, 1, len, fp) != len) {
        perror("fwrite");
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}
