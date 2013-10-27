/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013 
 */

#include "types.h"
#include "load.h"
#include "update.h"
#include "render.h"
#include "input.h"
#include "state.h"

struct game game;

/*
 * All static functions declared here.
 */
static void run_level();

static void transition0(int32_t x, int32_t y, int k);
static void transition1(int32_t x, int32_t y, int k);
static void transition2(int32_t x, int32_t y, int k);

/*
 * A surface for the level name.
 */
SDL_Surface *surface_levelname = NULL;

/*----------------------------------------------------------------------------
 * Play a level.
 *----------------------------------------------------------------------------*/
int
play(const char *path, const char *name)
{
    SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
    surface_levelname =
	TTF_RenderUTF8_Blended(font_large, name, color_white);

    game.state_stack_top = game.state_stack;
    game.state_stack_bottom = game.state_stack;

    game.keep_playing = 1;

    while (game.keep_playing && load_level(path)) {
	run_level();
	free_level();
    }

    SDL_FreeSurface(surface_levelname);
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,
			SDL_DEFAULT_REPEAT_INTERVAL);

    return game.warped;
}


/*----------------------------------------------------------------------------
 * To run a level. This is almost your garden variety main game loop,
 * delegating most of the work in other methods but in order to correctly
 * handle resources and flow when undoing movements while aborting the level
 * the normal flow is broken by a GOTO.
 *----------------------------------------------------------------------------*/
void
run_level()
{
    SDL_Event event;

    game.cs.pushing = 0;
    game.cs.dst_ang = 0;
    game.cs.cur_ang = 0;

    game.cs.ticks = 0;
    game.cs.record_ptr = game.record_list;

    if (game.state_stack_top == game.state_stack_bottom) {
	save_state();
    }

    // And here's the aberration. It is said that Djikstra headbangs his coffin
    // once per each one of those. I guess that he will do mine around year 2500
    // or so.

  game_loop_begin:

    game.keyup = 0;
    game.keydn = 0;
    game.keylf = 0;
    game.keyrt = 0;

    game.keep_going = 1;
    game.must_save = 0;
    game.warped = 0;

    if (enable_audio) {
	Mix_PlayChannel(CHANNEL_WODOX, chunk_wodox, -1);
	Mix_Pause(CHANNEL_WODOX);
    }

    while (game.keep_going) {
	update();
	draw_background();
	render_objects();	
	handle_player_input();
	render_foreground();
	sync();	
	++game.cs.ticks;	
    }

    if (enable_audio) 
	Mix_HaltChannel(CHANNEL_WODOX);

    // Before terminating we perform a screen transition to close the level but
    // we must still check playr input to recognize some things. Pressing
    // BACKSPACE during transition returns the wodox to the last safe position.
    // Pushing ESCAPE will prevent the level from reloading and return to the
    // menu. Any other key will just stop the transition and restart the level.

    // Determine transition focus and function.

    uint32_t i, j, k;

    if (game.po) {
	i = SCREENX(SPS * X(game.po->idx), SPS * Y(game.po->idx),
		    SPS * Z(game.po->idx)) + 28;
	j = SCREENY(SPS * X(game.po->idx), SPS * Y(game.po->idx),
		    SPS * Z(game.po->idx)) + 32;
    } else {
	i = canvas->w / 2;
	j = canvas->h / 2;
    }

    void (*transition) (int32_t, int32_t, int) = NULL;

    switch ((int) (3 * (rand() / (RAND_MAX + 1.)))) {
    case 0:
	transition = &transition0;
	break;
    case 1:
	transition = &transition1;
	break;
    case 2:
	transition = &transition2;
	break;
    }

    // Perform a transition.

    for (k = 0; k < 32; ++k) {
	if (SDL_PollEvent(&event)) {
	    switch (event.type) {
	    case SDL_KEYDOWN:
		switch (event.key.keysym.sym) {
		case SDLK_BACKSPACE:
		    load_state();
		    goto game_loop_begin;
		case SDLK_ESCAPE:
		    game.keep_playing = 0;
		    return;
		default:
		    return;
		}
		break;
	    case SDL_QUIT:
		exit(0);
	    }
	}

	transition(i, j, k);
	sync();
    }

    return;
}


/*----------------------------------------------------------------------------
 * Some screen transition effects.
 *----------------------------------------------------------------------------*/
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

/*----------------------------------------------------------------------------
 * Some screen transition effects.
 *----------------------------------------------------------------------------*/
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

/*----------------------------------------------------------------------------
 * Some screen transition effects.
 *----------------------------------------------------------------------------*/
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
