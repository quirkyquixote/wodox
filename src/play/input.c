/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013 
 */

#include "types.h"
#include "state.h"


/*----------------------------------------------------------------------------
 * Handle events and save states.
 *----------------------------------------------------------------------------*/
void
handle_player_input(void)
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

