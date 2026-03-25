#ifndef COMFY_META_H
#define COMFY_META_H

#include <stdio.h>

// Extract metadata associated with a key in a comfyUI generated file
// Return dynamically allocated string or NULL on failure
char *get_metadata(FILE *fp, const char *key);

#endif