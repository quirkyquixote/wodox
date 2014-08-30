/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013 
 */

#include "play_private.h"

/* Singleton to hold all internal state */
struct game game;

/* Main loop of the game */
static void game_main_loop();

/* Handles replaying */
static void replay(void);

/* Handles user input */
static void handle_event(SDL_Event * event);

/* In the correct circunstances, save state */
static void test_for_save_state(void);

/* To record a key pressing */
static void record(int k);

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

    while (game.keep_playing && game_load(path)) {
        game_main_loop();
        game_destroy();
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
game_main_loop()
{
    SDL_Event event;

    game.cs.pushing = 0;
    game.cs.dst_ang = 0;
    game.cs.cur_ang = 0;

    game.cs.ticks = 0;
    game.cs.record_ptr = game.record_list;

    if (game.state_stack_top == game.state_stack_bottom) {
        game_push_state();
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
        game_update();
        render_background();
        game_render();

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

    SDL_Rect dst;
    uint32_t k;

    if (game.po) {
        struct coord c = idx_to_coord(game.po->idx);
        dst = world_to_screen(c.x * SPS, c.y * SPS, c.z * SPS);
        dst.x += 28;
        dst.y += 32;
    } else {
        dst.x = media.canvas->w / 2;
        dst.y = media.canvas->h / 2;
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
                    game_pop_state();
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

        transition(&dst, k);
        media_sync();
    }

    return;
}

/*----------------------------------------------------------------------------
 * Record player input
 *----------------------------------------------------------------------------*/
void
record(int k)
{ 
    if (game.cs.record_ptr - game.record_list < RECORD_LIST_SIZE) {
        game.cs.record_ptr->time = game.cs.ticks; 
        game.cs.record_ptr->key = k; 
        ++game.cs.record_ptr; 
    } 
    game.replay = 0; 
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
            record(1);
            game.must_save = 1;
            break;
        case SDLK_DOWN:
            game.keydn = 1;
            record(2);
            game.must_save = 1;
            break;
        case SDLK_LEFT:
            game.keylf = 1;
            record(3);
            game.must_save = 1;
            break;
        case SDLK_RIGHT:
            game.keyrt = 1;
            record(4);
            game.must_save = 1;
            break;

        case SDLK_BACKSPACE:
            game_pop_state();
            record(game.keyup ? 1 : -1);
            record(game.keydn ? 2 : -2);
            record(game.keylf ? 3 : -3);
            record(game.keyrt ? 4 : -4);
            game.po->dir = STILL;
            break;

        case SDLK_F1:
            switch (help(lang.howtoplay, lang.menuplay, "\0rscq")) {
            case 1:
                game.keep_going = 0;
                break;
            case 2:
                game.keep_going = 0;
                game.keep_playing = 0;
                game.warped = 1;
                break;
            case 3:                /* load a solution */
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
            record(-1);
            break;
        case SDLK_DOWN:
            game.keydn = 0;
            record(-2);
            break;
        case SDLK_LEFT:
            game.keylf = 0;
            record(-3);
            break;
        case SDLK_RIGHT:
            game.keyrt = 0;
            record(-4);
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
            game_push_state();
            game.must_save = 0;
            break;
        }
    }
}
