/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013
 */

#pragma once

#include "core/core.h"
#include <stdint.h>

/*----------------------------------------------------------------------------
 * Define the terrain objects.
 *----------------------------------------------------------------------------*/
enum {
    EMPTY = 0x00,
    GROUND = 0x01,
    CRATE = 0x02,
    WODOX = 0x03,
    SMALL = 0x04,
    BELTLF = 0x05,
    BELTBK = 0x06,
    BELTRT = 0x07,
    BELTFT = 0x08,
    MOVING = 0x09,
    BUTTON = 0x0a,
    SWITCH = 0x0b,
    TUBE = 0x11,
    WARP = 0x12,
};

/*----------------------------------------------------------------------------
 * A compiled circuit
 *----------------------------------------------------------------------------*/
struct circuit {
    uint16_t size;
    uint16_t *tree;
};

/* Format a valid equation from a circuit */
int circuit_to_text(char *buf, struct circuit *c, uint16_t node, int is_right);

/* Create a valid circuit from an equation */
int text_to_circuit(char *buf, struct circuit *c, uint16_t node);

/*----------------------------------------------------------------------------
 * The scene is represented by two different maps:
 *
 * The static map defines the type of item in each space. During the game 
 * itself more than one object can occupy the same space, but not in the 
 * editor.
 *
 * The circuit map holds pointers to the circuits that connect each space
 *----------------------------------------------------------------------------*/
struct level {
    const char *path;
    uint8_t static_map[SIZE][SIZE][SIZE];
    struct circuit circuit_map[SIZE][SIZE][SIZE];

    struct coord cursor;
    int8_t object;
    int keep_going;
};

extern struct level level;

/*----------------------------------------------------------------------------
 * Aliases for quick iteration on all elements and low level manipulation.
 *----------------------------------------------------------------------------*/
#define S_MAP ((uint8_t *)level.static_map)
#define C_MAP ((struct circuit *)level.circuit_map)

/* Load a level */
int editor_load(const char *path);

/* Save a level */
int editor_save(const char *path);

/* Destroy a level */
void editor_destroy(void);

/* Edit a circuit */
int editor_edit_circuit(uint16_t idx);

/* Render entire level */
void editor_render_level(void);

/* Render circuit currently being edited */
void editor_render_circuit(void);

/* Shift the entire level in one direction */
void editor_shift_bk(void);
void editor_shift_ft(void);
void editor_shift_lf(void);
void editor_shift_rt(void);
void editor_shift_up(void);
void editor_shift_dn(void);

/* Rotate the entire level on the vertical axis */
void editor_rotate_lf(void);
void editor_rotate_rt(void);

