/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013 
 */

#pragma once 

#include "types.h"

struct object *object_new(uint8_t type, uint16_t off);
void object_free (struct object * o);

void object_move(struct object *o, int dir);
int object_try_move(struct object *o, int dir);

