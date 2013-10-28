/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013 
 */

#include "types.h"
#include "load.h"
#include "update.h"
#include "render.h"
#include "state.h"

struct game game;

/*
 * All static functions declared here.
 */
static void run_level();
static void replay(void);
static void handle_event(SDL_Event * event);
static void test_for_save_state(void);

/*----------------------------------------------------------------------------
 * Play a level.
 *----------------------------------------------------------------------------*/
int
play(const char *path, const char *name)
{
    SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
    media.surface_levelname =
	TTF_RenderUTF8_Blended(media.font_large, name, color_white);

    game.state_stack_top = game.state_stack;
    game.state_stack_bottom = game.state_stack;

    game.keep_playing = 1;

    while (game.keep_playing && load_level(path)) {
	run_level();
	free_level();
    }

    SDL_FreeSurface(media.surface_levelname);
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

    if (media.enable_audio) {
	Mix_PlayChannel(CHANNEL_WODOX, media.chunk_wodox, -1);
	Mix_Pause(CHANNEL_WODOX);
    }

    while (game.keep_going) {
	update();
	render_background();
	render_objects();

	if (game.replay)
	    replay();

	while (SDL_PollEvent(&event))
	    handle_event(&event);

	test_for_save_state();
	render_foreground();
	media_sync();
	++game.cs.ticks;
    }

    if (media.enable_audio)
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
	i = media.canvas->w / 2;
	j = media.canvas->h / 2;
    }

    transition_func *transition =
	TRANSITION_FUNC[(int) (3 * (rand() / (RAND_MAX + 1.)))];

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
	media_sync();
    }

    return;
}


/*----------------------------------------------------------------------------
 * When replaying, the wodox moves automatically
 *----------------------------------------------------------------------------*/
void
replay(void)
{
    while (game.cs.record_ptr->time == game.cs.ticks) {
	switch (game.cs.record_ptr->key) {
	case 1:
	    game.keyup = 1;
	    game.must_save = 1;
	    break;
	case 2:
	    game.keydn = 1;
	    game.must_save = 1;
	    break;
	case 3:
	    game.keylf = 1;
	    game.must_save = 1;
	    break;
	case 4:
	    game.keyrt = 1;
	    game.must_save = 1;
	    break;

	case -1:
	    game.keyup = 0;
	    break;
	case -2:
	    game.keydn = 0;
	    break;
	case -3:
	    game.keylf = 0;
	    break;
	case -4:
	    game.keyrt = 0;
	    break;

	default:
	    break;
	}
	++game.cs.record_ptr;
    }
}

/*----------------------------------------------------------------------------
 * Handle all events
 *----------------------------------------------------------------------------*/
void
handle_event(SDL_Event * event)
{
    switch (event->type) {
    case SDL_KEYDOWN:
	switch (event->key.keysym.sym) {
	case SDLK_UP:
	    game.keyup = 1;
	    RECORD_MOVE(1);
	    game.must_save = 1;
	    break;
	case SDLK_DOWN:
	    game.keydn = 1;
	    RECORD_MOVE(2);
	    game.must_save = 1;
	    break;
	case SDLK_LEFT:
	    game.keylf = 1;
	    RECORD_MOVE(3);
	    game.must_save = 1;
	    break;
	case SDLK_RIGHT:
	    game.keyrt = 1;
	    RECORD_MOVE(4);
	    game.must_save = 1;
	    break;

	case SDLK_BACKSPACE:
	    load_state();
	    RECORD_MOVE(game.keyup ? 1 : -1);
	    RECORD_MOVE(game.keydn ? 2 : -2);
	    RECORD_MOVE(game.keylf ? 3 : -3);
	    RECORD_MOVE(game.keyrt ? 4 : -4);
	    game.po->dir = STILL;
	    break;

	case SDLK_F1:
	    switch (help(str_howtoplay, str_menuplay, "\0rscq")) {
	    case 1:
		game.keep_going = 0;
		break;
	    case 2:
		game.keep_going = 0;
		game.keep_playing = 0;
		game.warped = 1;
		break;
	    case 3:		/* load a solution */
		break;
	    case 4:
		game.keep_going = 0;
		game.keep_playing = 0;
		break;
	    }
	    break;

	case SDLK_p:
	    media_freeze();
	    break;
	case SDLK_r:
	    game.keep_going = 0;
	    break;
	case SDLK_ESCAPE:
	    game.keep_going = 0;
	    game.keep_playing = 0;
	    break;
	case SDLK_s:
	    game.keep_going = 0;
	    game.keep_playing = 0;
	    game.warped = 1;
	    break;
	default:
	    break;
	}

	if (media.enable_audio &&
	    (game.keyup || game.keydn || game.keylf || game.keyrt)) {
	    Mix_Resume(CHANNEL_WODOX);
	}

	break;

    case SDL_KEYUP:
	switch (event->key.keysym.sym) {
	case SDLK_UP:
	    game.keyup = 0;
	    RECORD_MOVE(-1);
	    break;
	case SDLK_DOWN:
	    game.keydn = 0;
	    RECORD_MOVE(-2);
	    break;
	case SDLK_LEFT:
	    game.keylf = 0;
	    RECORD_MOVE(-3);
	    break;
	case SDLK_RIGHT:
	    game.keyrt = 0;
	    RECORD_MOVE(-4);
	    break;
	default:
	    break;
	}

	if (media.enable_audio &&
	    (game.keyup | game.keydn | game.keylf | game.keyrt) == 0) {
	    Mix_Pause(CHANNEL_WODOX);
	}

	break;

    case SDL_QUIT:
	exit(0);

    default:
	break;
    }
}

/*----------------------------------------------------------------------------
 * If the player pushed a button recently and wodox is aligned to the grid and
 * standing on the right kind of tile, save state.
 *----------------------------------------------------------------------------*/
void
test_for_save_state(void)
{
    if (game.must_save && game.po->dsp == 0 && game.po->dir != DIR_DN &&
	FRC[game.po->idx] != DIR_UP) {
	switch (MAP[idx_dn(game.po->idx)]) {
	case GROUND:
	case CRATE:
	case BELT_LF_0:
	case BELT_RT_0:
	case BELT_FT_0:
	case BELT_BK_0:
	case BUTTON_0:
	case BUTTON_1:
	case SWITCH_0:
	case SWITCH_1:
	    save_state();
	    game.must_save = 0;
	    break;
	}
    }
}
