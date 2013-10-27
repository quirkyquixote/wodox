/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013 
 */

#include "types.h"


/*
 * Grab an object from the unused list and initialize it.
 */
struct object *
object_grab(Uint8 type, Uint16 off)
{
    if (list_empty(&game.cs.unused_objects)) {
	return NULL;
    }

    struct object *o = (struct object *) game.cs.unused_objects.begin;
    list_pop_front(&game.cs.unused_objects);

    memset(o, 0, sizeof(struct object));
    o->type = type;
    o->off = off;
    o->dir = STILL;
    o->dsp = 0;

    return o;
}

/*
 * Drop an object in the unused list.
 */

    void
      object_drop (struct object * o)
      {
      list_push_front (&game.cs.unused_objects, (list_node *)o);
      } 

/*
 * Insert object into object list and maps.
 */
void
object_insert(struct object *o)
{
    list_push_back(&game.cs.objects, &(o->node));
    OBJ[o->off] = o;
    MAP[o->off] = o->type;
}

/*
 * Remove object from object list and maps.
 */
void
object_remove(struct object *o)
{
    list_remove(&game.cs.objects, &(o->node));
    OBJ[o->off] = NULL;
    MAP[o->off] = EMPTY;
}

