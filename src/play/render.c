/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013 
 */

#include "types.h"
#include "render.h"


static void render_column(size_t i, size_t k);

static void transition0(SDL_Rect *dst, int k);
static void transition1(SDL_Rect *dst, int k);
static void transition2(SDL_Rect *dst, int k);

transition_func *TRANSITION_FUNC[] = {
    transition0,
    transition1,
    transition2
};


/*----------------------------------------------------------------------------
 * Draw all objects. Most objects have only a simple sprite with no alpha
 * channel, but levitators have alpha transparency and a "warp" effect, and the
 * exit is a particle system.
 *----------------------------------------------------------------------------*/
void
render_objects(void)
{
    size_t i, k;

    for (i = 0; i < SIZE; ++i)
	for (k = 0; k < SIZE; ++k) 
	    render_column(i, k);
}

/*----------------------------------------------------------------------------
 * Render a complete vertical column of objects.
 *
 * Apply a 'warp' effect where there are levitators. The entire column is done
 * at once to make the transitions smooth.
 *
 * FIXME: Note the horribly complicated way to draw moving objects. Order
 * matters a lot and yet, some objects appear in front of others that are
 * adjacent when they ought not to.
 *----------------------------------------------------------------------------*/
void
render_column(size_t i, size_t k)
{
    size_t j;
    struct object *o;
    SDL_Rect dst;

    for (j = 0; j < SIZE; ++j) {
	if (game.cs.forces_map[j][i][k] == DIR_UP) {
	    dst = world_to_screen (SPS * i, SPS * j, SPS * k);

	    if (game.cs.forces_map[j + 1][i][k] !=
		game.cs.forces_map[j][i][k]) {
		render_effect(1, 0, &dst);
	    }

	    render_effect(0, 0, &dst);
	}

	if (game.cs.forces_map[j][i][k] == WARP) {
	    dst = world_to_screen (SPS * i, SPS * j, SPS * k);
	    render_particles(media.canvas, &dst);
	}

	if (((o = game.cs.object_map[j][i][k]) &&
	     (o->dir == STILL || o->dir == DIR_UP || o->dir == DIR_RT ||
	      o->dir == DIR_FT))
	    || (j > 0 && (o = game.cs.object_map[j - 1][i][k]) &&
		o->dir == DIR_DN)
	    || (i > 0 && (o = game.cs.object_map[j][i - 1][k]) &&
		o->dir == DIR_LF)
	    || (k > 0 && (o = game.cs.object_map[j][i][k - 1]) &&
		o->dir == DIR_BK)) {
	    switch (o->dir) {
	    case DIR_LF:
	        dst = world_to_screen (i * SPS - o->dsp, j * SPS, k * SPS);
		break;
	    case DIR_RT:
	        dst = world_to_screen ((i - 1) * SPS + o->dsp, j * SPS, k * SPS);
		break;
	    case DIR_DN:
	        dst = world_to_screen (i * SPS, j * SPS - o->dsp, k * SPS);
		break;
	    case DIR_UP:
	        dst = world_to_screen (i * SPS, (j - 1) * SPS + o->dsp, k * SPS);
		break;
	    case DIR_BK:
	        dst = world_to_screen (i * SPS, j * SPS, k * SPS - o->dsp);
		break;
	    case DIR_FT:
	        dst = world_to_screen (i * SPS, j * SPS, (k - 1) * SPS + o->dsp);
		break;
	    default:
	        dst = world_to_screen (i * SPS, j * SPS, k * SPS);
		break;
	    }

	    if (o == game.po) {
		if (game.keep_going || game.warped) {
		    render_object(2, game.cs.cur_ang % 4, &dst);
		}
	    } else {
		render_object(o->type & SPRITE, 0, &dst);
	    }
	}

	else if (game.cs.static_map[j][i][k] & VISIBLE) {
	    switch (game.cs.static_map[j][i][k]) {
	    case BELT_LF_1:
	    case BELT_FT_1:
	    case BELT_RT_1:
	    case BELT_BK_1:
		dst = world_to_screen (SPS * i, SPS * j, SPS * k);
		render_object(game.cs.static_map[j][i][k] & SPRITE,
			    1 + game.cs.ticks % 3, &dst);
		break;

	    default:
		dst = world_to_screen (SPS * i, SPS * j, SPS * k);
		render_object(game.cs.static_map[j][i][k] & SPRITE, 0, &dst);
		break;
	    }
	}
    }

    for (j = 0; j < SIZE; ++j) {
	if (game.cs.forces_map[j][i][k] == DIR_UP) {
	    dst = world_to_screen (SPS * i, SPS * j, SPS * k);
	    warp_surface(media.canvas, &dst);
	}
    }
}


/*----------------------------------------------------------------------------
 * Some screen transition effects.
 *----------------------------------------------------------------------------*/
void
transition0(SDL_Rect *dst, int k)
{
    SDL_Rect rect;
    int state;
    int i;

    for (i = 0; i < media.canvas->w + 32; i += 32) {
	state = 2 * k + (i - media.canvas->w) / 32;

	if (state > 0) {
	    rect.x = i - state / 2;
	    rect.y = 0;
	    rect.w = state;
	    rect.h = media.canvas->h;
	    SDL_FillRect(media.canvas, &rect, 0);
	}
    }
}

/*----------------------------------------------------------------------------
 * Some screen transition effects.
 *----------------------------------------------------------------------------*/
void
transition1(SDL_Rect *dst, int k)
{
    SDL_Rect rect;
    int state;
    int j;

    for (j = 0; j < media.canvas->h + 32; j += 32) {
	state = 2 * k + (j - media.canvas->h) / 32;

	if (state > 0) {
	    rect.x = 0;
	    rect.y = j - state / 2;
	    rect.w = media.canvas->w;
	    rect.h = state;
	    SDL_FillRect(media.canvas, &rect, 0);
	}
    }
}

/*----------------------------------------------------------------------------
 * Some screen transition effects.
 *----------------------------------------------------------------------------*/
void
transition2(SDL_Rect *dst, int k)
{
    int32_t i, j;
    int32_t state;
    int16_t vx[4];
    int16_t vy[4];

    for (i = 0; i < media.canvas->w + 32; i += 32) {
	for (j = 0; j < media.canvas->h + 32; j += 32) {
	    state = k - 16 + hypot(i - dst->x, j - dst->y) / 32;

	    if (state > 0) {
		vx[0] = i;
		vx[1] = i + 2 * state;
		vx[2] = i;
		vx[3] = i - 2 * state;
		vy[0] = j + 2 * state;
		vy[1] = j;
		vy[2] = j - 2 * state;
		vy[3] = j;
		filledPolygonRGBA(media.canvas, vx, vy, 4, 0, 0, 0, 255);
	    }
	}
    }
}
