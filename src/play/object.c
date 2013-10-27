/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013 
 */

#include "types.h"


/*
 * Grab an object from the unused list and initialize it.
 */
struct object *
object_new(Uint8 type, Uint16 off)
{
    if (game.object_count > OBJECT_POOL_SIZE)
	return NULL;

    struct object *o = &game.cs.objects[game.object_count++];

    memset(o, 0, sizeof(struct object));
    o->type = type;
    o->off = off;
    o->dir = STILL;
    o->dsp = 0;

    OBJ[o->off] = o;
    MAP[o->off] = o->type;
    return o;
}

/*
 * Drop an object in the unused list.
 */
void
object_free (struct object * o)
{
    OBJ[o->off] = NULL;
    MAP[o->off] = EMPTY;
} 

