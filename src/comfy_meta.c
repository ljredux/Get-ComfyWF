#include "comfy_meta.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static bool is_png(FILE *fp);
static uint32_t read_be32(FILE *fp);
static char *extract_from_png(FILE *fp, const char *key);
static char *extract_from_mp4(FILE *fp, const char *key);

// Main public function
char *get_metadata(FILE *fp, const char *key) {
    if (!fp) return NULL;

    char *result = NULL;

    if (is_png(fp)) {
        result = extract_from_png(fp, key);
    } else {
        // Assume MP4
        rewind(fp);
        result = extract_from_mp4(fp, key);
    }

    return result;
}

static bool is_png(FILE *fp) {
    unsigned char sig[8];

    long pos = ftell(fp);

    // Read and check file signature
    if (fread(sig, 1, 8, fp) != 8) {
        fseek(fp, pos, SEEK_SET);
        return false;
    }
    static const unsigned char png_sig[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    bool result = (memcmp(sig, png_sig, 8) == 0);

    fseek(fp, pos, SEEK_SET);

    return result;
}

// Reads a 32-bit big-endian integer from a file
static uint32_t read_be32(FILE *fp) {
    unsigned char b[4];
    if (fread(b, 1, 4, fp) != 4) return 0;
    return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
}

// key: the keyword to search for in the PNG's tEXt chunks
// Returns the text value (allocated string) if found, or NULL
static char *extract_from_png(FILE *fp, const char *key) {
    fseek(fp, 8, SEEK_SET); // Skip PNG signature

    while (1) {
        // Read chunk length (4 bytes, big-endian)
        uint32_t length = read_be32(fp);
        if (length == 0) break; // Early exit on read error

        // Read chunk type (4 bytes, e.g. "IHDR", "tEXt", "IEND")
        char type[5] = {0};
        if (fread(type, 1, 4, fp) != 4) break;

        // Allocate buffer for chunk data
        unsigned char *data = malloc(length);
        if (!data) break;

        // Read chunk data
        if (fread(data, 1, length, fp) != length) {
            free(data);
            break;
        }
        read_be32(fp); // Skip the CRC

        // Is this a tEXt chunk?
        if (strcmp(type, "tEXt") == 0) {
            char *sep = memchr(data, 0, length);
            // Does the keyword match the one we want?
            if (sep && strcmp((char *)data, key) == 0) {
                char *result = strdup(sep + 1);
                free(data);
                return result;
            }
        }

        // Clean up and loop to next chunk or break
        free(data);
        if (strcmp(type, "IEND") == 0) break;
    }

    return NULL;
}

// Placeholder method
static char *extract_from_mp4(FILE *fp, const char *key) {
    (void)fp;
    (void)key;
    return NULL;
}