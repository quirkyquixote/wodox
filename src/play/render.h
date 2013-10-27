/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013 
 */


#pragma once

void render_objects(void);
void render_foreground(void);

typedef void transition_func (int32_t, int32_t, int);

extern transition_func * TRANSITION_FUNC[];

