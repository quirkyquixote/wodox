/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013
 */

#pragma once

#include <SDL/SDL.h>
#include "menu.h"

struct level {
    char file[32];
    int track;
};

struct context {
    struct level *level_list;
    int level_count;

    char **sandbox_list;
    int sandbox_count;

    SDL_Surface *surface_menu;

    int offset;
};

extern const SDL_Color black;
extern const SDL_Color white;

extern struct context ctx;
