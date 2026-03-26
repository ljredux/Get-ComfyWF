#include "comfy_meta.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static bool is_png(FILE *fp);
static uint32_t read_be32(FILE *fp);
static char *read_png_tEXt_chunk(FILE *fp, const char *key);
static char *read_mp4_ilst_data(FILE *fp, const char *key);

// Main public function
char *get_metadata(FILE *fp, const char *key) {
    if (!fp) return NULL;

    char *result = NULL;

    if (is_png(fp)) {
        result = read_png_tEXt_chunk(fp, key);
    } else {
        // Assume MP4
        rewind(fp);
        result = read_mp4_ilst_data(fp, key);
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

// Returns the value of a PNG tEXt chunk matching the given key or null if not found.
// The returned string must be freed.
static char *read_png_tEXt_chunk(FILE *fp, const char *key) {
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

// TODO: Revisit MP4 metadata parsing (below). Nested structure makes extraction tricky. 
// It assumes metadata is stored as key/value pairs in ilst.data (as Comfy does it).
typedef struct {
    uint64_t size;
    char type[5];
    long start;
} Box;

static int read_box(FILE *fp, Box *box) {
    box->start = ftell(fp);

    uint32_t size32 = read_be32(fp);
    if (fread(box->type, 1, 4, fp) != 4) return 0;
    box->type[4] = '\0';

    if (size32 == 1) {
        uint64_t hi = read_be32(fp);
        uint64_t lo = read_be32(fp);
        box->size = (hi << 32) | lo;
    } else {
        box->size = size32;
    }

    if (box->size < 8) return 0;
    return 1;
}

// Returns the value of a ComfyUI MP4 metadata entry matching the given key or NULL if not found.
// The returned string must be freed.
static char *read_mp4_ilst_data(FILE *fp, const char *key) {
    fseek(fp, 0, SEEK_SET);
    Box box;

    // find moov
    while (read_box(fp, &box)) {
        if (!strcmp(box.type, "moov")) break;
        fseek(fp, box.start + box.size, SEEK_SET);
    }
    if (strcmp(box.type, "moov")) return NULL;

    long moov_end = box.start + box.size;

    // find udta > meta > ilst
    while (ftell(fp) < moov_end && read_box(fp, &box)) {
        if (!strcmp(box.type, "udta")) {
            long udta_end = box.start + box.size;
            while (ftell(fp) < udta_end && read_box(fp, &box)) {
                if (!strcmp(box.type, "meta")) {
                    fseek(fp, 4, SEEK_CUR); // version/flags
                    long meta_end = box.start + box.size;
                    while (ftell(fp) < meta_end && read_box(fp, &box)) {
                        if (!strcmp(box.type, "ilst")) {
                            long ilst_end = box.start + box.size;
                            while (ftell(fp) < ilst_end && read_box(fp, &box)) {
                                // read data box inside each item
                                long item_end = box.start + box.size;
                                while (ftell(fp) < item_end && read_box(fp, &box)) {
                                    if (!strcmp(box.type, "data")) {
                                        fseek(fp, 8, SEEK_CUR); // type/locale
                                        uint32_t len = box.size - 16;
                                        char *text = malloc(len + 1);
                                        fread(text, 1, len, fp);
                                        text[len] = 0;

                                        // check if matches key
                                        if (strstr(text, key)) return text;
                                        free(text);
                                    }
                                    fseek(fp, box.start + box.size, SEEK_SET);
                                }
                                fseek(fp, box.start + box.size, SEEK_SET);
                            }
                        }
                        fseek(fp, box.start + box.size, SEEK_SET);
                    }
                }
                fseek(fp, box.start + box.size, SEEK_SET);
            }
        }
        fseek(fp, box.start + box.size, SEEK_SET);
    }

    return NULL;
}