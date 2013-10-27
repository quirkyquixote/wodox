/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013 
 */

#include "types.h"
#include "load.h"
#include "update.h"
#include "render.h"

struct game game;

/*
 * All static functions declared here.
 */
static void run_level();

static void handle_player_input();

static void save_state();
static void load_state();

static void transition0(Sint32 x, Sint32 y, int k);
static void transition1(Sint32 x, Sint32 y, int k);
static void transition2(Sint32 x, Sint32 y, int k);

/*
 * A surface for the level name.
 */
SDL_Surface *surface_levelname = NULL;

/*
 * Play a level.
 */
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
/*
  if (warped && load_level (path))
    {
       run_level (1, &warped);
       free_level ();
    }
*/
    SDL_FreeSurface(surface_levelname);
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,
			SDL_DEFAULT_REPEAT_INTERVAL);

    return game.warped;
}


/*
 * To run a level. This is almost your garden variety main game loop,
 * delegating most of the work in other methods but in order to correctly
 * handle resources and flow when undoing movements while aborting the level
 * the normal flow is broken by a GOTO.
 */
void
run_level()
{
    // SDL stuff.

    SDL_Event event;

    // Current game state.

    game.cs.pushing = 0;
    game.cs.dst_ang = 0;
    game.cs.cur_ang = 0;

    game.cs.outside = NULL;
    game.cs.inside = NULL;
    game.cs.unlocked = 1;

    game.cs.ticks = 0;
    game.cs.record_ptr = game.record_list;

    // If the state stack is empty, record the initial state.

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

	draw_background();	// Background texture.
	draw_objects();		// Draw all objects and features of the level.

	handle_player_input();	// Handle events and more.

	draw_foreground();	// Draw some GUI data. This is done
	// after the event handling because
	// these messages should not appear
	// when freezing the game.

	sync();			// Update screen.

	++game.cs.ticks;	// On to the next iteration.
    }

    if (enable_audio) {
	Mix_HaltChannel(CHANNEL_WODOX);
    }
    // Before terminating we perform a screen transition to close the level but
    // we must still check playr input to recognize some things. Pressing
    // BACKSPACE during transition returns the wodox to the last safe position.
    // Pushing ESCAPE will prevent the level from reloading and return to the
    // menu. Any other key will just stop the transition and restart the level.

    // Determine transition focus and function.

    Uint32 i, j, k;

    if (game.po) {
	i = SCREENX(SPS * X(game.po->off), SPS * Y(game.po->off),
		    SPS * Z(game.po->off)) + 28;
	j = SCREENY(SPS * X(game.po->off), SPS * Y(game.po->off),
		    SPS * Z(game.po->off)) + 32;
    } else {
	i = canvas->w / 2;
	j = canvas->h / 2;
    }

    void (*transition) (Sint32, Sint32, int) = NULL;

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

    // Return.

    return;
}


/*
 * Handle events and more.
 */
void
handle_player_input()
{
    SDL_Event event;

    // If we are replaying whe wodox movement is taken from the game.record_list.

    if (game.replay) {
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

	    case 0:
		//PICK_OR_RELEASE();
		break;

	    default:
		break;
	    }
	    ++game.cs.record_ptr;
	}
    }
    // Handle player input.

    while (SDL_PollEvent(&event)) {
	switch (event.type) {
	case SDL_KEYDOWN:
	    switch (event.key.keysym.sym) {
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

	    case SDLK_SPACE:
		//PICK_OR_RELEASE();
		//RECORD_MOVE(0);
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
		case 3:	/* load a solution */
		    break;
		case 4:
		    game.keep_going = 0;
		    game.keep_playing = 0;
		    break;
		}
		break;

	    case SDLK_p:
		freeze();
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

	    if (enable_audio &&
		(game.keyup || game.keydn || game.keylf || game.keyrt)) {
		Mix_Resume(CHANNEL_WODOX);
	    }

	    break;

	case SDL_KEYUP:
	    switch (event.key.keysym.sym) {
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

	    if (enable_audio &&
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

    // If the player pushed a button recently and wodox is aligned to the
    // grid and standing on the right kind of tile, save state.

    if (game.must_save && game.po->dsp == 0 && game.po->dir != DIR_DN &&
	FRC_AT(game.po) != DIR_UP) {
	switch (MAP_DN(game.po)) {
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
transition0(Sint32 x, Sint32 y, int k)
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
transition1(Sint32 x, Sint32 y, int k)
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
transition2(Sint32 x, Sint32 y, int k)
{
    Sint32 i, j;
    Sint32 state;
    Sint16 vx[4];
    Sint16 vy[4];

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
