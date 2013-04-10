/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2012
 */

#pragma once

#include "lang.h"
#include "user.h"

/*
 * The wodox application searches for files in two places: 
 *
 * Static files (textures, samples, fonts, levels...) are stored in DATADIR.
 * This path can be specified through the configure script.
 *
 * User specific files (player profiles) are stored in the user home directory:
 * in *NIX systems, this is $HOME/.wodox; in Windows systems, we use
 * %APPDATA%\wodox.
 */

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

/*
 * Wodox levels are implemented as three dimensinal arrays of 10x10x10 
 * positions but the individual positions are represented by a single number
 * that is the offset from the first position in the array; The following 
 * macros are used to operate on the maps.
 */

#define SIZE 10
#define SIZE_2 100
#define SIZE_3 1000

#define MIN 0
#define MAX 9

#define Y(off) ((off) / SIZE_2)
#define X(off) (((off) % SIZE_2) / SIZE)
#define Z(off) (((off) % SIZE_2) % SIZE)

#define OFF(x,y,z) ((y) * SIZE_2 + (x) * SIZE + (z))

/* 
 * Circuits are implemented as binary trees with a fixed size array where 
 * the left child for position x is 2x + 1 and the right is 2x + 2. 
 * Each node of the tree contains either a world position or an operator.
 */

#define NOT  0xffff
#define AND  0xfffe
#define OR   0xfffd
#define XOR  0xfffc
#define NONE 0xfffb

#define TREE_SIZE 16

#define PARENT(off) (((off) - 1) / 2)
#define LCHILD(off) (2 * (off) + 1)
#define RCHILD(off) (2 * (off) + 2)

/*
 * Steps per square. Abbreviated because it is used so often. This is the
 * number of frames needed for an object to complete its movement from a 
 * space to the adjacen one.
 */

#define SPS 4

/*
 * From here we have graphics dependent definitions. If we are to reimplement
 * the 3d version, all of this will change.
 */

#define SCREENX(x,y,z) (canvas->w / 2 + 100 + 4 * (x) - 10 * (z))
#define SCREENY(x,y,z) (canvas->h / 2 - 8 * (y) + 6 * (x) + 2 * (z))

#define WORLDX(i,j,y) (((i) + 5 * (j) - canvas->w / 2 - 5 * canvas->h / 2 + 40 * (y) - 100) / 34)
#define WORLDZ(i,j,y) ((2 * (j) - 3 * (i) + 3 * canvas->w / 2 - canvas->h + 300 + 16 * (y)) / 34)

/*
 * To hell with strcpy, strcat and all their relatives...
 */
char * strjoin (char *separator, ...);

/*
 * To list the contents of a folder.
 */
int list_folder (char * path, char *** files);
int cstring_cmp (const void *a, const void *b);




