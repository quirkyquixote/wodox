/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013
 */

#include "rotate.h"

/*
 * Rotate an object in some direction. This basically swaps one type of
 * conveyor belt for another.
 */
static uint8_t rotate_object_lf(uint8_t p);
static uint8_t rotate_object_rt(uint8_t p);

/*
 * Rotate a circuit in some direction.
 */
static struct circuit rotate_circuit_lf(struct circuit c);
static struct circuit rotate_circuit_rt(struct circuit c);

/*
 * The following methods rotate the level.
 */
void
rotate_lf(void)
{
    uint8_t i, j, k, m, n, tmp;
    struct circuit tmp2;
    for (j = 0; j < SIZE; ++j) {
	for (i = MIN, m = MAX; i < m; ++i, --m)
	    for (k = MIN, n = MAX; k < n; ++k, --n) {
		tmp = level.static_map[j][i][k];
		level.static_map[j][i][k] = rotate_object_lf(level.static_map[j][n][i]);
		level.static_map[j][n][i] = rotate_object_lf(level.static_map[j][m][n]);
		level.static_map[j][m][n] = rotate_object_lf(level.static_map[j][k][m]);
		level.static_map[j][k][m] = rotate_object_lf(tmp);

		tmp2 = level.circuit_map[j][i][k];
		level.circuit_map[j][i][k] = rotate_circuit_lf(level.circuit_map[j][n][i]);
		level.circuit_map[j][n][i] = rotate_circuit_lf(level.circuit_map[j][m][n]);
		level.circuit_map[j][m][n] = rotate_circuit_lf(level.circuit_map[j][k][m]);
		level.circuit_map[j][k][m] = rotate_circuit_lf(tmp2);
	    }
    }
}

void
rotate_rt(void)
{
    uint8_t i, j, k, m, n, tmp;
    struct circuit tmp2;
    for (j = 0; j < SIZE; ++j) {
	for (i = MIN, m = MAX; i < m; ++i, --m)
	    for (k = MIN, n = MAX; k < n; ++k, --n) {
		tmp = level.static_map[j][i][k];
		level.static_map[j][i][k] = rotate_object_rt(level.static_map[j][k][m]);
		level.static_map[j][k][m] = rotate_object_rt(level.static_map[j][m][n]);
		level.static_map[j][m][n] = rotate_object_rt(level.static_map[j][n][i]);
		level.static_map[j][n][i] = rotate_object_rt(tmp);

		tmp2 = level.circuit_map[j][i][k];
		level.circuit_map[j][i][k] = rotate_circuit_rt(level.circuit_map[j][k][m]);
		level.circuit_map[j][k][m] = rotate_circuit_rt(level.circuit_map[j][m][n]);
		level.circuit_map[j][m][n] = rotate_circuit_rt(level.circuit_map[j][n][i]);
		level.circuit_map[j][n][i] = rotate_circuit_rt(tmp2);
	    }
    }
}

/*
 * These rotate objects whose orientation matters.
 */
uint8_t
rotate_object_lf(uint8_t p)
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
rotate_object_rt(uint8_t p)
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
rotate_circuit_lf(struct circuit n)
{
    uint16_t *it;
    uint8_t i, j, k;
    if (n.tree)
	for (it = n.tree; it < n.tree + n.size; ++it)
	    if (*it < NONE) {
		i = X(*it);
		j = Y(*it);
		k = Z(*it);
		*it = OFF(k, j, MAX - i);
	    }
    return n;
}

struct circuit
rotate_circuit_rt(struct circuit n)
{
    uint16_t *it;
    uint8_t i, j, k;
    if (n.tree)
	for (it = n.tree; it < n.tree + n.size; ++it)
	    if (*it < NONE) {
		i = X(*it);
		j = Y(*it);
		k = Z(*it);
		*it = OFF(MAX - k, j, i);
	    }
    return n;
}

