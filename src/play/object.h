/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013 
 */

#pragma once 

#include "types.h"

struct object *object_new(uint8_t type, uint16_t off);
void object_free (struct object * o);
void object_insert(struct object *o);
void object_remove(struct object *o);

