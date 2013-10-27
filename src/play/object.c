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
    struct object *o = malloc(sizeof(*o));

    memset(o, 0, sizeof(struct object));
    o->next = game.cs.objects;
    o->type = type;
    o->off = off;
    o->dir = STILL;
    o->dsp = 0;

    game.cs.objects = o;
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
    free(o);
} 

