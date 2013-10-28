/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013
 */

#pragma once

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

/*----------------------------------------------------------------------------
 * A tree
 *----------------------------------------------------------------------------*/
struct tree {
    int token;
    struct tree *l;
    struct tree *r;
};

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
    uint8_t static_map[SIZE][SIZE][SIZE];
    struct circuit circuit_map[SIZE][SIZE][SIZE];
};

extern struct level level;

/*----------------------------------------------------------------------------
 * Aliases for quick iteration on all elements and low level manipulation.
 *----------------------------------------------------------------------------*/
#define S_MAP ((uint8_t *)level.static_map)
#define C_MAP ((struct circuit *)level.circuit_map)
