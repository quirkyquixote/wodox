/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013 
 */

#include "play_private.h"


/*----------------------------------------------------------------------------
 * Grab an object from the unused list and initialize it.
 *----------------------------------------------------------------------------*/
struct object *
object_new(uint8_t type, uint16_t off)
{
    if (game.object_count > OBJECT_POOL_SIZE)
	return NULL;

    struct object *o = &game.cs.objects[game.object_count++];

    memset(o, 0, sizeof(struct object));
    o->type = type;
    o->idx = off;
    o->dir = STILL;
    o->dsp = 0;

    OBJ[o->idx] = o;
    MAP[o->idx] = o->type;
    return o;
}

/*----------------------------------------------------------------------------
 * Drop an object in the unused list.
 *----------------------------------------------------------------------------*/
void
object_free (struct object * o)
{
    OBJ[o->idx] = NULL;
    MAP[o->idx] = EMPTY;
} 

/*----------------------------------------------------------------------------
 * Move object
 *----------------------------------------------------------------------------*/
void
object_move(struct object *o, int dir)
{
  o->dir = dir;
  MAP[o->idx + offset[dir]] = o->type;
  MAP[o->idx] = GHOST;
  OBJ[o->idx + offset[dir]] = o;
  OBJ[o->idx] = NULL;
}

/*----------------------------------------------------------------------------
 * Move object, if possible
 *----------------------------------------------------------------------------*/
int
object_try_move(struct object *o, int dir)
{
    if (MAP[o->idx + offset[dir]] & SOLID)
	return STILL;
    object_move(o, dir);
    return dir;
}

