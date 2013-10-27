/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013 
 */

#include "types.h"

/*
 * Release buttons under the chosen object.
 */
void
release_buttons(struct object *o)
{
    switch (MAP_DN(o)) {
    case BUTTON_1:
	MAP_DN(o) = BUTTON_0;
	if (enable_audio) {
	    Mix_PlayChannel(CHANNEL_RELEASE, chunk_release, 0);
	}
	break;
    }
}

/*
 * Press buttons under the chosen object.
 */
void
press_buttons(struct object *o)
{
    switch (MAP_DN(o)) {
    case BUTTON_0:
	MAP_DN(o) = BUTTON_1;
	if (enable_audio) {
	    Mix_PlayChannel(CHANNEL_PRESS, chunk_press, 0);
	}
	break;
    case SWITCH_0:
	MAP_DN(o) = SWITCH_1;
	if (enable_audio) {
	    Mix_PlayChannel(CHANNEL_PRESS, chunk_press, 0);
	}
	break;
    case SWITCH_1:
	MAP_DN(o) = SWITCH_0;
	if (enable_audio) {
	    Mix_PlayChannel(CHANNEL_PRESS, chunk_press, 0);
	}
	break;
    }
}

/*
 * Calculate the output of a circuit.
 */
int
calculate(Uint16 * tree, size_t off)
{
    switch (tree[off]) {
    case NOT:
	return !calculate(tree, 2 * off + 1);
    case AND:
	return calculate(tree, 2 * off + 1) & calculate(tree, 2 * off + 2);
    case OR:
	return calculate(tree, 2 * off + 1) | calculate(tree, 2 * off + 2);
    case XOR:
	return calculate(tree, 2 * off + 1) ^ calculate(tree, 2 * off + 2);
    case 0 ... SIZE_3 - 1:
	return MAP[tree[off]] & ACTIVE ? 1 : 0;
    }

    return 0;
}

