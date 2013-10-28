/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013 
 */


#pragma once

void render_objects(void);

typedef void transition_func (SDL_Rect *, int);

extern transition_func * TRANSITION_FUNC[];

