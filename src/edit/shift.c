/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013
 */

#include "edit_private.h"

/*
 * Shift a circuit in some direction.
 */
static struct circuit editor_shift_circuit_bk(struct circuit c);
static struct circuit editor_shift_circuit_ft(struct circuit c);
static struct circuit editor_shift_circuit_lf(struct circuit c);
static struct circuit editor_shift_circuit_rt(struct circuit c);
static struct circuit editor_shift_circuit_up(struct circuit c);
static struct circuit editor_shift_circuit_dn(struct circuit c);

/*
 * The following methods 'shift' the level in one direction.
 */
void
editor_shift_bk(void)
{
    uint8_t i, j, k, tmp;
    struct circuit tmp2;
    for (i = 0; i < SIZE; ++i)
	for (j = 0; j < SIZE; ++j) {
	    tmp = level.static_map[j][i][MIN];
	    tmp2 = level.circuit_map[j][i][MIN];
	    for (k = MIN; k < MAX; ++k) {
		level.static_map[j][i][k] = level.static_map[j][i][k + 1];
		level.circuit_map[j][i][k] = editor_shift_circuit_bk(level.circuit_map[j][i][k + 1]);
	    }
	    level.static_map[j][i][MAX] = tmp;
	    level.circuit_map[j][i][MAX] = editor_shift_circuit_bk(tmp2);
	}
}

void
editor_shift_ft(void)
{
    uint8_t i, j, k, tmp;
    struct circuit tmp2;
    for (i = 0; i < SIZE; ++i)
	for (j = 0; j < SIZE; ++j) {
	    tmp = level.static_map[j][i][MAX];
	    tmp2 = level.circuit_map[j][i][MAX];
	    for (k = MAX; k > MIN; --k) {
		level.static_map[j][i][k] = level.static_map[j][i][k - 1];
		level.circuit_map[j][i][k] = editor_shift_circuit_ft(level.circuit_map[j][i][k - 1]);
	    }
	    level.static_map[j][i][MIN] = tmp;
	    level.circuit_map[j][i][MIN] = editor_shift_circuit_ft(tmp2);
	}
}

void
editor_shift_lf(void)
{
    uint8_t i, j, k, tmp;
    struct circuit tmp2;
    for (k = 0; k < SIZE; ++k)
	for (j = 0; j < SIZE; ++j) {
	    tmp = level.static_map[j][MIN][k];
	    tmp2 = level.circuit_map[j][MIN][k];
	    for (i = MIN; i < MAX; ++i) {
		level.static_map[j][i][k] = level.static_map[j][i + 1][k];
		level.circuit_map[j][i][k] = editor_shift_circuit_lf(level.circuit_map[j][i + 1][k]);
	    }
	    level.static_map[j][MAX][k] = tmp;
	    level.circuit_map[j][MAX][k] = editor_shift_circuit_lf(tmp2);
	}
}

void
editor_shift_rt(void)
{
    uint8_t i, j, k, tmp;
    struct circuit tmp2;
    for (k = 0; k < SIZE; ++k)
	for (j = 0; j < SIZE; ++j) {
	    tmp = level.static_map[j][MAX][k];
	    tmp2 = level.circuit_map[j][MAX][k];
	    for (i = MAX; i > MIN; --i) {
		level.static_map[j][i][k] = level.static_map[j][i - 1][k];
		level.circuit_map[j][i][k] = editor_shift_circuit_rt(level.circuit_map[j][i - 1][k]);
	    }
	    level.static_map[j][MIN][k] = tmp;
	    level.circuit_map[j][MIN][k] = editor_shift_circuit_rt(tmp2);
	}
}

void
editor_shift_dn(void)
{
    uint8_t i, j, k, tmp;
    struct circuit tmp2;
    for (i = 0; i < SIZE; ++i)
	for (k = 0; k < SIZE; ++k) {
	    tmp = level.static_map[MIN][i][k];
	    tmp2 = level.circuit_map[MIN][i][k];
	    for (j = MIN; j < MAX; ++j) {
		level.static_map[j][i][k] = level.static_map[j + 1][i][k];
		level.circuit_map[j][i][k] = editor_shift_circuit_dn(level.circuit_map[j + 1][i][k]);
	    }
	    level.static_map[MAX][i][k] = tmp;
	    level.circuit_map[MAX][i][k] = editor_shift_circuit_dn(tmp2);
	}
}

void
editor_shift_up(void)
{
    uint8_t i, j, k, tmp;
    struct circuit tmp2;
    for (i = 0; i < SIZE; ++i)
	for (k = 0; k < SIZE; ++k) {
	    tmp = level.static_map[MAX][i][k];
	    tmp2 = level.circuit_map[MAX][i][k];
	    for (j = MAX; j > MIN; --j) {
		level.static_map[j][i][k] = level.static_map[j - 1][i][k];
		level.circuit_map[j][i][k] = editor_shift_circuit_up(level.circuit_map[j - 1][i][k]);
	    }
	    level.static_map[MIN][i][k] = tmp;
	    level.circuit_map[MIN][i][k] = editor_shift_circuit_up(tmp2);
	}
}

/*
 * The following methods 'shift' a circuit in one direction.
 */
struct circuit
editor_shift_circuit_ft(struct circuit n)
{
    uint16_t *it;
    if (n.tree)
	for (it = n.tree; it < n.tree + n.size; ++it)
	    if (*it < NONE) {
		*it = SIZE * (*it / SIZE) + (((*it % SIZE) + 1) % SIZE);
	    }
    return n;
}

struct circuit
editor_shift_circuit_bk(struct circuit n)
{
    uint16_t *it;
    if (n.tree)
	for (it = n.tree; it < n.tree + n.size; ++it)
	    if (*it < NONE) {
		*it =
		    SIZE * (*it / SIZE) +
		    (((*it % SIZE) + SIZE - 1) % SIZE);
	    }
    return n;
}

struct circuit
editor_shift_circuit_up(struct circuit n)
{
    uint16_t *it;
    if (n.tree)
	for (it = n.tree; it < n.tree + n.size; ++it)
	    if (*it < NONE) {
		*it = (*it + SIZE_2) % SIZE_3;
	    }
    return n;
}

struct circuit
editor_shift_circuit_dn(struct circuit n)
{
    uint16_t *it;
    if (n.tree)
	for (it = n.tree; it < n.tree + n.size; ++it)
	    if (*it < NONE) {
		*it = (*it + SIZE_3 - SIZE_2) % SIZE_3;
	    }
    return n;
}

struct circuit
editor_shift_circuit_rt(struct circuit n)
{
    uint16_t *it;
    if (n.tree)
	for (it = n.tree; it < n.tree + n.size; ++it)
	    if (*it < NONE) {
		*it =
		    SIZE_2 * (*it / SIZE_2) +
		    (((*it % SIZE_2) + SIZE) % SIZE_2);
	    }
    return n;
}

struct circuit
editor_shift_circuit_lf(struct circuit n)
{
    uint16_t *it;
    if (n.tree)
	for (it = n.tree; it < n.tree + n.size; ++it)
	    if (*it < NONE) {
		*it =
		    SIZE_2 * (*it / SIZE_2) +
		    (((*it % SIZE_2) + SIZE_2 - SIZE) % SIZE_2);
	    }
    return n;
}
