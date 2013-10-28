/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013 
 */

#include "types.h"
#include "object.h"
#include "circuit.h"

static void update_static_circuit(struct static_circuit *s);
static void update_dynamic_circuit(struct dynamic_circuit *d);
static void update_heavy_object(struct object *o);
static void update_player_object(void);
static void update_object(struct object *o);

static void try_move_player(uint8_t dir);
static int levitate(struct object *o);

/*----------------------------------------------------------------------------
 * Update all
 *----------------------------------------------------------------------------*/
void
update(void)
{
    struct static_circuit *s;
    struct dynamic_circuit *d;
    struct object *o;
    int i;

    for (s = game.static_circuits; s; s = s->next)
	update_static_circuit(s);

    for (d = game.dynamic_circuits; d; d = d->next)
	update_dynamic_circuit(d);

    for (i = SIZE_3 - 1; i >= 0; --i)
	if ((o = OBJ[i]) && (o->type & HEAVY))
	    update_heavy_object(o);

    update_player_object();

    for (i = 0; i < game.object_count; ++i)
	update_object(&game.cs.objects[i]);
}

/*----------------------------------------------------------------------------
 * Check the state for all static circuits. These may activate and deactivate
 * levitators and conveyor belts.
 *
 * Levitators are a special case because when they are off the only clue to
 * their presence is the fact that there is a static circuit pointing an empty
 * space; thus activating or deactivating levitators is done when there is no
 * other machine in place.
 *----------------------------------------------------------------------------*/
void
update_static_circuit(struct static_circuit *s)
{
    if (calculate(s->tree, 0)) {
	switch (MAP[s->idx]) {
	case BELT_LF_0:
	    MAP[s->idx] = BELT_LF_1;
	    break;
	case BELT_RT_0:
	    MAP[s->idx] = BELT_RT_1;
	    break;
	case BELT_BK_0:
	    MAP[s->idx] = BELT_BK_1;
	    break;
	case BELT_FT_0:
	    MAP[s->idx] = BELT_FT_1;
	    break;

	case BELT_LF_1:
	case BELT_RT_1:
	case BELT_BK_1:
	case BELT_FT_1:
	    break;

	default:
	    FRC[s->idx] = DIR_UP;
	    if (FRC[idx_up(s->idx)] == DIR_DN)
		FRC[idx_up(s->idx)] = STILL;
	    break;
	}
    } else {
	switch (MAP[s->idx]) {
	case BELT_LF_1:
	    MAP[s->idx] = BELT_LF_0;
	    break;
	case BELT_RT_1:
	    MAP[s->idx] = BELT_RT_0;
	    break;
	case BELT_BK_1:
	    MAP[s->idx] = BELT_BK_0;
	    break;
	case BELT_FT_1:
	    MAP[s->idx] = BELT_FT_0;
	    break;

	case BELT_LF_0:
	case BELT_RT_0:
	case BELT_BK_0:
	case BELT_FT_0:
	    break;

	default:
	    if (FRC[idx_dn(s->idx)] == DIR_UP)
		FRC[s->idx] = STILL;
	    else
		FRC[s->idx] = DIR_DN;
	    if (FRC[idx_up(s->idx)] == STILL)
		FRC[idx_up(s->idx)] = DIR_DN;
	    break;
	}
    }
}

/*----------------------------------------------------------------------------
 * Check state for all dynamic circuits. These may activate and deactivate
 * moving blocks.
 *----------------------------------------------------------------------------*/
void
update_dynamic_circuit(struct dynamic_circuit *d)
{
    if (calculate(d->tree, 0)) {
	switch (d->obj->type) {
	case MOVING_0:
	    if (object_try_move(d->obj, DIR_DN)) {
		MAP[idx_dn(d->obj->idx)] = MOVING_1;
		d->obj->type = MOVING_1;
		if (media.enable_audio)
		    Mix_PlayChannel(CHANNEL_OPEN, media.chunk_open, 0);
	    }
	    break;
	}
    } else {
	switch (d->obj->type) {
	case MOVING_1:
	    if (levitate(d->obj)) {
		MAP[idx_up(d->obj->idx)] = MOVING_0;
		d->obj->type = MOVING_0;
		if (media.enable_audio)
		    Mix_PlayChannel(CHANNEL_OPEN, media.chunk_open, 0);
	    }
	    break;
	}
    }
}

/*----------------------------------------------------------------------------
 * Heavy objects are subject to forces. Depending on the value of the space in
 * the forces map, they can go either up, down or nowhere, but if they ought to
 * go up and are unable to do so or the space has no force they might be
 * affected by a conveyor belt. For these checks we must iterate on the object
 * map because order matters: objects in higher layers have priority.
 *----------------------------------------------------------------------------*/
void
update_heavy_object(struct object *o)
{
    if (o->dir != STILL)
	return;

    switch (FRC[o->idx]) {
    case DIR_DN:
	if (Y(o->idx) > MIN && object_try_move(o, DIR_DN))
	    return;
	break;
    case DIR_UP:
	if (levitate(o))
	    return;
	break;
    }

    switch (MAP[idx_dn(o->idx)]) {
    case BELT_LF_1:
	if (X(o->idx) > MIN)
	    object_try_move(o, DIR_LF);
	break;
    case BELT_RT_1:
	if (X(o->idx) < MAX)
	    object_try_move(o, DIR_RT);
	break;
    case BELT_BK_1:
	if (Z(o->idx) > MIN)
	    object_try_move(o, DIR_BK);
	break;
    case BELT_FT_1:
	if (Z(o->idx) < MAX)
	    object_try_move(o, DIR_FT);
	break;
    }
}

/*----------------------------------------------------------------------------
 * Handle player movement; In addition to being a heavy object, it...
 *  - Can be moved by the player if there are no external forces.
 *  - When moving it can push other heavy objects.
 *  - It is affected by the 'warp force'.
 *  - It is 'killed' if it reaches the lowermost layer of the level.
 *  - It interacts in complex ways with small objects.
 *
 * Also handle wodox rotation. This is for purely decorative purposes.
 *----------------------------------------------------------------------------*/
void
update_player_object(void)
{
    switch (game.cs.cur_ang - game.cs.dst_ang) {
    case -8 ... -5:
	if (--game.cs.cur_ang < 0)
	    game.cs.cur_ang = 7;
	break;
    case -4 ... -1:
	++game.cs.cur_ang;
	break;
    case 1 ... 4:
	--game.cs.cur_ang;
	break;
    case 5 ... 8:
	if (++game.cs.cur_ang > 7)
	    game.cs.cur_ang = 0;
	break;
    default:
	game.cs.cur_ang = game.cs.dst_ang;
	break;
    }

    if (game.po == NULL || game.po->dir != STILL)
	return;

    if (Y(game.po->idx) == 0)
	game.keep_going = 0;

    if (FRC[game.po->idx] == WARP) {
	game.keep_going = 0;
	game.keep_playing = 0;
	game.warped = 1;
    }

    switch (game.keydn - game.keyup) {
    case -1:
	game.cs.dst_ang = 0;
	if (X(game.po->idx) > MIN)
	    try_move_player(DIR_LF);
	return;

    case 1:
	game.cs.dst_ang = 4;
	if (X(game.po->idx) < MAX)
	    try_move_player(DIR_RT);
	return;
    }

    switch (game.keylf - game.keyrt) {
    case -1:
	game.cs.dst_ang = 2;
	if (Z(game.po->idx) > MIN)
	    try_move_player(DIR_BK);
	return;
    case 1:
	game.cs.dst_ang = 6;
	if (Z(game.po->idx) < MAX)
	    try_move_player(DIR_FT);
	return;
    }

    game.cs.pushing = 0;
}

/*----------------------------------------------------------------------------
 * Try to move the player object in direction dir
 *----------------------------------------------------------------------------*/
void
try_move_player(uint8_t dir)
{
    struct object *o;

    if ((o = OBJ[game.po->idx + offset[dir]]) && o->dir == STILL) {
	if ((o->type & HEAVY) &&
	    (o->idx / bounds[dir] ==
	     (o->idx + offset[dir]) / bounds[dir])) {
	    if (game.cs.pushing++ > PUSH_DELAY && object_try_move(o, dir)) {
		object_move(game.po, dir);
		game.cs.pushing = 0;
	    }
	    return;
	}
    }

    object_try_move(game.po, dir);
    game.cs.pushing = 0;
}

/*----------------------------------------------------------------------------
 * Update every object. This time order doesn't matter and we can use the
 * object list.
 *----------------------------------------------------------------------------*/
void
update_object(struct object *o)
{
    if (o->dir == STILL || ++o->dsp < SPS)
	return;

    if (MAP[o->idx] == GHOST)
	MAP[o->idx] = EMPTY;

    if (o != game.po)
	release_buttons(o);

    o->idx += offset[o->dir];

    if (o != game.po)
	press_buttons(o);

    o->dir = STILL;
    o->dsp = 0;
}


/*----------------------------------------------------------------------------
 * Move an object up, pushing every other object on its way if possible.
 *----------------------------------------------------------------------------*/
int
levitate(struct object *o)
{
    struct object *tmp;

    if (Y(o->idx) == MAX)
	return 0;

    if ((tmp = OBJ[idx_up(o->idx)]) && tmp->dir == STILL) {
	if (levitate(tmp)) {
	    object_move(o, DIR_UP);
	    return DIR_UP;
	}

	return STILL;
    }

    return object_try_move(o, DIR_UP);
}

