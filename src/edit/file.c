/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "edit_private.h"

/*
 * Load level.
 */
int
editor_load(const char *path)
{
    FILE *f;
    uint16_t n;
 
    level.path = path;
    memset(level.static_map, EMPTY, sizeof(uint8_t) * SIZE_3);
    memset(level.circuit_map, 0, sizeof(struct circuit) * SIZE_3);

    if ((f = fopen(path, "rb")) == NULL) {
        perror(path);
        return 0;
    }

    fread(level.static_map, sizeof(uint8_t), SIZE_3, f);

    while (fread(&n, sizeof(uint16_t), 1, f) == 1 && n < SIZE_3) {
        fread(&C_MAP[n].size, sizeof(uint16_t), 1, f);
        C_MAP[n].tree = malloc(sizeof(uint16_t) * C_MAP[n].size);
        fread(C_MAP[n].tree, sizeof(uint16_t), C_MAP[n].size, f);
    }

    fclose(f);
    return 1;
}

/*
 * Save level.
 */
int
editor_save(const char *path)
{
    FILE *f;
    uint16_t n;

    if ((f = fopen(path, "wb")) == NULL) {
        perror(path);
        return 0;
    }

    fwrite(level.static_map, sizeof(uint8_t), SIZE_3, f);

    for (n = 0; n < SIZE_3; ++n) {
        if (C_MAP[n].tree) {
            fwrite(&n, sizeof(uint16_t), 1, f);
            fwrite(&C_MAP[n].size, sizeof(uint16_t), 1, f);
            fwrite(C_MAP[n].tree, sizeof(uint16_t), C_MAP[n].size, f);
        }
    }

    fclose(f);
    return 1;
}

void
editor_destroy(void)
{
    int i;

    for (i = 0; i < SIZE_3; ++i) 
        if (C_MAP[i].tree) 
            free(C_MAP[i].tree);
}

