/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013
 */

#pragma once

#include <stdint.h>
#include "lang.h"
#include "user.h"

/*----------------------------------------------------------------------------
 * The wodox application searches for files in two places: 
 *
 * Static files (textures, samples, fonts, levels...) are stored in DATADIR.
 * This path can be specified through the configure script.
 *
 * User specific files (player profiles) are stored in the user home directory:
 * in *NIX systems, this is $HOME/.wodox; in Windows systems, we use
 * %APPDATA%\wodox.
 *----------------------------------------------------------------------------*/
#if defined(__unix__)
#  define PATH_SEPARATOR "/"
#  define DATA_DIR DATADIR PATH_SEPARATOR PACKAGE
#  define USER_DIR getenv ("HOME"), "." PACKAGE
#elif defined(_WIN32) || defined(_WIN64)
#  define PATH_SEPARATOR "\\"
#  define DATA_DIR DATADIR PATH_SEPARATOR PACKAGE
#  define USER_DIR getenv ("APPDATA"), PACKAGE
#else
#  error "Unsupported OS"
#endif

/*----------------------------------------------------------------------------
 * Wodox levels are implemented as three dimensinal arrays of 10x10x10 
 * positions but the individual positions are represented by a single number
 * that is the offset from the first position in the array; The following 
 * macros are used to operate on the maps.
 *----------------------------------------------------------------------------*/
#define SIZE 10
#define SIZE_2 100
#define SIZE_3 1000

#define MIN 0
#define MAX 9

#define OFF(x,y,z) ((y) * SIZE_2 + (x) * SIZE + (z))

/*----------------------------------------------------------------------------
 * Table indices
 *----------------------------------------------------------------------------*/
static inline uint16_t idx_bk (uint16_t idx) { return idx - 1; }
static inline uint16_t idx_ft (uint16_t idx) { return idx + 1; }
static inline uint16_t idx_lf (uint16_t idx) { return idx - SIZE; }
static inline uint16_t idx_rt (uint16_t idx) { return idx + SIZE; }
static inline uint16_t idx_dn (uint16_t idx) { return idx - SIZE_2; }
static inline uint16_t idx_up (uint16_t idx) { return idx + SIZE_2; }

static inline uint8_t idx_x(uint16_t idx) { return (idx % SIZE_2) / SIZE; }
static inline uint8_t idx_y(uint16_t idx) { return idx / SIZE_2; }
static inline uint8_t idx_z(uint16_t idx) { return (idx % SIZE_2) % SIZE; }

/*----------------------------------------------------------------------------
 * World coordinates
 *----------------------------------------------------------------------------*/
struct coord {
    uint8_t x;
    uint8_t y;
    uint8_t z;
};

static inline struct coord
coord(uint8_t x, uint8_t y, uint8_t z)
{
    return (struct coord) { x, y, z };
}

static inline uint16_t 
coord_to_idx(struct coord w)
{
    return (uint16_t)w.y * SIZE_2 + (uint16_t)w.x * SIZE + (uint16_t)w.z;
}

static inline struct coord
idx_to_coord(uint16_t idx)
{
    return coord(idx_x(idx), idx_y(idx), idx_z(idx));
}

static inline int
coordcmp(struct coord a, struct coord b)
{
    return coord_to_idx(a) - coord_to_idx(b);
}

/*----------------------------------------------------------------------------
 * Circuits are implemented as binary trees with a fixed size array where 
 * the left child for position x is 2x + 1 and the right is 2x + 2. 
 * Each node of the tree contains either a world position or an operator.
 *----------------------------------------------------------------------------*/
enum {
    NOT  = 0xffff,
    AND  = 0xfffe,
    OR   = 0xfffd,
    XOR  = 0xfffc,
    NONE = 0xfffb,
};

#define TREE_SIZE 16

static inline uint16_t tree_up(uint16_t idx) { return (idx - 1) / 2; }
static inline uint16_t tree_lf(uint16_t idx) { return (2 * idx) + 1; }
static inline uint16_t tree_rt(uint16_t idx) { return (2 * idx) + 2; }

/*----------------------------------------------------------------------------
 * Steps per square. Abbreviated because it is used so often. This is the
 * number of frames needed for an object to complete its movement from a 
 * space to the adjacen one.
 *----------------------------------------------------------------------------*/
#define SPS 4

/*----------------------------------------------------------------------------
 * To hell with strcpy, strcat and all their relatives...
 *----------------------------------------------------------------------------*/
char * strjoin (char *separator, ...);

/*----------------------------------------------------------------------------
 * To list the contents of a folder.
 *----------------------------------------------------------------------------*/
int list_folder (char * path, char *** files);
int cstring_cmp (const void *a, const void *b);




