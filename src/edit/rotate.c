/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013
 */

#include "edit_private.h"

/*
 * Rotate an object in some direction. This basically swaps one type of
 * conveyor belt for another.
 */
static uint8_t editor_rotate_object_lf(uint8_t p);
static uint8_t editor_rotate_object_rt(uint8_t p);

/*
 * Rotate a circuit in some direction.
 */
static struct circuit editor_rotate_circuit_lf(struct circuit c);
static struct circuit editor_rotate_circuit_rt(struct circuit c);

/*
 * The following methods rotate the level.
 */
void
editor_rotate_lf(void)
{
    uint8_t i, j, k, m, n, tmp;
    struct circuit tmp2;

    for (j = 0; j < SIZE; ++j) {
        for (i = MIN, m = MAX; i < m; ++i, --m)
            for (k = MIN, n = MAX; k < n; ++k, --n) {
                tmp = level.static_map[j][i][k];
                level.static_map[j][i][k] = editor_rotate_object_lf(level.static_map[j][n][i]);
                level.static_map[j][n][i] = editor_rotate_object_lf(level.static_map[j][m][n]);
                level.static_map[j][m][n] = editor_rotate_object_lf(level.static_map[j][k][m]);
                level.static_map[j][k][m] = editor_rotate_object_lf(tmp);

                tmp2 = level.circuit_map[j][i][k];
                level.circuit_map[j][i][k] = editor_rotate_circuit_lf(level.circuit_map[j][n][i]);
                level.circuit_map[j][n][i] = editor_rotate_circuit_lf(level.circuit_map[j][m][n]);
                level.circuit_map[j][m][n] = editor_rotate_circuit_lf(level.circuit_map[j][k][m]);
                level.circuit_map[j][k][m] = editor_rotate_circuit_lf(tmp2);
            }
    }
}

void
editor_rotate_rt(void)
{
    uint8_t i, j, k, m, n, tmp;
    struct circuit tmp2;

    for (j = 0; j < SIZE; ++j) {
        for (i = MIN, m = MAX; i < m; ++i, --m)
            for (k = MIN, n = MAX; k < n; ++k, --n) {
                tmp = level.static_map[j][i][k];
                level.static_map[j][i][k] = editor_rotate_object_rt(level.static_map[j][k][m]);
                level.static_map[j][k][m] = editor_rotate_object_rt(level.static_map[j][m][n]);
                level.static_map[j][m][n] = editor_rotate_object_rt(level.static_map[j][n][i]);
                level.static_map[j][n][i] = editor_rotate_object_rt(tmp);

                tmp2 = level.circuit_map[j][i][k];
                level.circuit_map[j][i][k] = editor_rotate_circuit_rt(level.circuit_map[j][k][m]);
                level.circuit_map[j][k][m] = editor_rotate_circuit_rt(level.circuit_map[j][m][n]);
                level.circuit_map[j][m][n] = editor_rotate_circuit_rt(level.circuit_map[j][n][i]);
                level.circuit_map[j][n][i] = editor_rotate_circuit_rt(tmp2);
            }
    }
}

/*
 * These rotate objects whose orientation matters.
 */
uint8_t
editor_rotate_object_lf(uint8_t p)
{
    switch (p) {
    case BELTLF:
        return BELTFT;
    case BELTBK:
        return BELTLF;
    case BELTRT:
        return BELTBK;
    case BELTFT:
        return BELTRT;
    }
    return p;
}

uint8_t
editor_rotate_object_rt(uint8_t p)
{
    switch (p) {
    case BELTLF:
        return BELTBK;
    case BELTBK:
        return BELTRT;
    case BELTRT:
        return BELTFT;
    case BELTFT:
        return BELTLF;
    }
    return p;
}

/*
 * These rotate circuits.
 */
struct circuit
editor_rotate_circuit_lf(struct circuit n)
{
    uint16_t *it;
    struct coord c;

    if (n.tree) {
        for (it = n.tree; it < n.tree + n.size; ++it) {
            if (*it < NONE) {
                c = idx_to_coord(*it);
                *it = coord_to_idx(coord(c.z, c.y, MAX - c.x));
            }
        }
    }

    return n;
}

struct circuit
editor_rotate_circuit_rt(struct circuit n)
{
    uint16_t *it;
    struct coord c;

    if (n.tree) {
        for (it = n.tree; it < n.tree + n.size; ++it) {
            if (*it < NONE) {
                c = idx_to_coord(*it);
                *it = coord_to_idx(coord(MAX - c.z, c.y, c.x));
            }
        }
    }

    return n;
}

