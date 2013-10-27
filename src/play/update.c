/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013 
 */

#include "types.h"
#include "object.h"
#include "circuit.h"

static void update_static_circuits(void);
static void update_dynamic_circuits(void);
static void update_heavy_objects(void);
static void update_player_object(void);
static void update_objects(void);

static int levitate(struct object *o);

void
update(void)
{
    update_static_circuits();
    update_dynamic_circuits();
    update_heavy_objects();
    update_player_object();
    update_objects();

}


/*
 * Check the state for all static circuits. These may activate and deactivate
 * levitators and conveyor belts.
 *
 * Levitators are a special case because when they are off the only clue to
 * their presence is the fact that there is a static circuit pointing an empty
 * space; thus activating or deactivating levitators is done when there is no
 * other machine in place.
 */
void
update_static_circuits(void)
{
    struct static_circuit *s;

    for (s = game.static_circuits; s; s = s->next) {
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
}

/*
 * Check state for all dynamic circuits. These may activate and deactivate
 * moving blocks.
 */
void
update_dynamic_circuits(void)
{
    struct dynamic_circuit *d;

    for (d = game.dynamic_circuits; d; d = d->next) {
	if (calculate(d->tree, 0)) {
	    switch (d->obj->type) {
	    case MOVING_0:
		if (object_try_move(d->obj, DIR_DN)) {
		    MAP[idx_dn(d->obj->idx)] = MOVING_1;
		    d->obj->type = MOVING_1;
		    if (enable_audio) 
			Mix_PlayChannel(CHANNEL_OPEN, chunk_open, 0);
		}
		break;
	    }
	} else {
	    switch (d->obj->type) {
	    case MOVING_1:
		if (levitate(d->obj)) {
		    MAP[idx_up(d->obj->idx)] = MOVING_0;
		    d->obj->type = MOVING_0;
		    if (enable_audio) 
			Mix_PlayChannel(CHANNEL_OPEN, chunk_open, 0);
		}
		break;
	    }
	}
    }
}

/*
 * Heavy objects are subject to forces. Depending on the value of the space in
 * the forces map, they can go either up, down or nowhere, but if they ought to
 * go up and are unable to do so or the space has no force they might be
 * affected by a conveyor belt. For these checks we must iterate on the object
 * map because order matters: objects in higher layers have priority.
 */
void
update_heavy_objects(void)
{
    uint16_t idx;
    struct object *o;

    for (idx = SIZE_3 - 1; idx < SIZE_3; --idx) {
	if ((o = OBJ[idx]) && (o->type & HEAVY) && (o->dir == STILL)) {
	    switch (FRC[idx]) {
	    case DIR_DN:
		if (Y(o->idx) > MIN)
		    object_try_move(o, DIR_DN);
		break;
	    case DIR_UP:
		if (levitate(o))
		    break;	// Levitators are mightily strong.
	    }

	    if (o->dir == STILL) {
		switch (MAP[idx_dn(idx)]) {
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
	}
    }
}

/*
 * Control for the player object.
 */
void
update_player_object(void)
{
    // Handle wodox rotation. This is for purely decorative purposes.

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

    // Handle player movement; In addition to being a heavy object, the 
    // player object:
    //
    //  - Can be moved by the player if there are no external forces.
    //  - When moving it can push other heavy objects.
    //  - It is affected by the 'warp force'.
    //  - It is 'killed' if it reaches the lowermost layer of the level.
    //  - It interacts in complex ways with small objects.
    //
    // I hate small objects.

    if (game.po && game.po->dir == STILL) {
	if (Y(game.po->idx) == 0) {
	    game.keep_going = 0;
	}

	if (FRC[game.po->idx] == WARP) {
	    game.keep_going = 0;
	    game.keep_playing = 0;
	    game.warped = 1;
	}

	switch (game.keydn - game.keyup) {
	case -1:
	    game.cs.dst_ang = 0;
	    if (X(game.po->idx) > MIN)
		TRY_MOVE_PLAYER(DIR_LF);
	    break;
	case 1:
	    game.cs.dst_ang = 4;
	    if (X(game.po->idx) < MAX)
		TRY_MOVE_PLAYER(DIR_RT);
	    break;

	default:
	    switch (game.keylf - game.keyrt) {
	    case -1:
		game.cs.dst_ang = 2;
		if (Z(game.po->idx) > MIN)
		    TRY_MOVE_PLAYER(DIR_BK);
		break;
	    case 1:
		game.cs.dst_ang = 6;
		if (Z(game.po->idx) < MAX)
		    TRY_MOVE_PLAYER(DIR_FT);
		break;
	    default:
		game.cs.pushing = 0;
		break;
	    }
	}
    }
}

/*
 * Update every object. This time order doesn't matter and we can use the
 * object list.
 */
void
update_objects(void)
{
    int i;
    struct object *o;

    for (i = 0; i < game.object_count; ++i) {
	o = &game.cs.objects[i];

	if (o->dir != STILL && ++o->dsp >= SPS) {
	    if (MAP[o->idx] == GHOST) {
		MAP[o->idx] = EMPTY;
	    }

	    if (o != game.po) {
		release_buttons(o);
	    }

	    o->idx += offset[o->dir];

	    if (o != game.po) {
		press_buttons(o);
	    }

	    o->dir = STILL;
	    o->dsp = 0;
	}
    }
}


/*
 * Move an object up, pushing every other object on its way if possible.
 */
int
levitate(struct object *o)
{
    struct object *tmp;

    if (Y(o->idx) == MAX) {
	return 0;
    }

    if ((tmp = OBJ[idx_up(o->idx)]) && tmp->dir == STILL) {
	if (levitate(tmp)) {
	    object_move(o, DIR_UP);
	    return DIR_UP;
	}

	return STILL;
    }

    return object_try_move(o, DIR_UP);
}

/*
 * Save the state of the level in the state stack.
 */
void
save_state()
{
    memcpy(game.state_stack_top, &game.cs, sizeof(struct state));

    if (++game.state_stack_top == game.state_stack + STATE_STACK_LEN) {
	game.state_stack_top = game.state_stack;
    }

    if (game.state_stack_top == game.state_stack_bottom &&
	++game.state_stack_bottom == game.state_stack + STATE_STACK_LEN) {
	game.state_stack_bottom = game.state_stack;
    }
}

/*
 * Load the state of the level from the state stack.
 */
void
load_state()
{
    if (game.state_stack_top != game.state_stack_bottom) {
	if (game.state_stack_top == game.state_stack) {
	    game.state_stack_top = game.state_stack + STATE_STACK_LEN;
	}
	--game.state_stack_top;
    }

    memcpy(&game.cs, game.state_stack_top, sizeof(struct state));
}

/*
 * Some screen transition effects.
 */
void
transition0(int32_t x, int32_t y, int k)
{
    SDL_Rect rect;
    int state;
    int i;

    for (i = 0; i < canvas->w + 32; i += 32) {
	state = 2 * k + (i - canvas->w) / 32;

	if (state > 0) {
	    rect.x = i - state / 2;
	    rect.y = 0;
	    rect.w = state;
	    rect.h = canvas->h;
	    SDL_FillRect(canvas, &rect, 0);
	}
    }
}

void
transition1(int32_t x, int32_t y, int k)
{
    SDL_Rect rect;
    int state;
    int j;

    for (j = 0; j < canvas->h + 32; j += 32) {
	state = 2 * k + (j - canvas->h) / 32;

	if (state > 0) {
	    rect.x = 0;
	    rect.y = j - state / 2;
	    rect.w = canvas->w;
	    rect.h = state;
	    SDL_FillRect(canvas, &rect, 0);
	}
    }
}

void
transition2(int32_t x, int32_t y, int k)
{
    int32_t i, j;
    int32_t state;
    int16_t vx[4];
    int16_t vy[4];

    for (i = 0; i < canvas->w + 32; i += 32) {
	for (j = 0; j < canvas->h + 32; j += 32) {
	    state = k - 16 + hypot(i - x, j - y) / 32;

	    if (state > 0) {
		vx[0] = i;
		vx[1] = i + 2 * state;
		vx[2] = i;
		vx[3] = i - 2 * state;
		vy[0] = j + 2 * state;
		vy[1] = j;
		vy[2] = j - 2 * state;
		vy[3] = j;
		filledPolygonRGBA(canvas, vx, vy, 4, 0, 0, 0, 255);
	    }
	}
    }
}
