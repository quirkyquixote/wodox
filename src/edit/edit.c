/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013
 */

#include <errno.h>

#include "../media/media.h"
#include "../play/play.h"

#include "types.h"
#include "parse.h"
#include "shift.h"
#include "rotate.h"
#include "render.h"
#include "file.h"
#include "circuit.h"

struct level level;


static void check_singletons(void);
static void handle_event(SDL_Event * event);

/*
 * To edit a level.
 */
int
edit(char *path)
{
    uint16_t i;
    uint16_t offset = 0;
    SDL_Rect dst;
    SDL_Event event;

    media.surface_levelname = NULL;

    level.keep_going = 1;
    level.cursor = coord(0, 0, 0);
    level.object = -1;

    edit_load(path);

    if (path == NULL)
	path = strjoin(PATH_SEPARATOR, USER_DIR, "sandbox", NULL);

    while (level.keep_going) {
	check_singletons();
	render_background();
	render_level();
	render_circuit();

	while (SDL_PollEvent(&event) != 0)
	    handle_event(&event);

	render_foreground();
	media_sync();
    }

    edit_free();
    return 0;
}

void
handle_event(SDL_Event * event)
{
    switch (event->type) {
    case SDL_KEYUP:
	switch (event->key.keysym.sym) {
	case SDLK_g:
	    if (level.object == GROUND)
		level.object = -1;
	    break;
	case SDLK_c:
	    if (level.object == CRATE)
		level.object = -1;
	    break;
	case SDLK_p:
	    if (level.object == WODOX)
		level.object = -1;
	    break;
	case SDLK_j:
	    if (level.object == SMALL)
		level.object = -1;
	    break;
	case SDLK_l:
	    if (level.object == TUBE)
		level.object = -1;
	    break;
	case SDLK_u:
	    if (level.object == BELTLF)
		level.object = -1;
	    break;
	case SDLK_o:
	    if (level.object == BELTBK)
		level.object = -1;
	    break;
	case SDLK_i:
	    if (level.object == BELTRT)
		level.object = -1;
	    break;
	case SDLK_y:
	    if (level.object == BELTFT)
		level.object = -1;
	    break;
	case SDLK_m:
	    if (level.object == MOVING)
		level.object = -1;
	    break;
	case SDLK_b:
	    if (level.object == BUTTON)
		level.object = -1;
	    break;
	case SDLK_s:
	    if (level.object == SWITCH)
		level.object = -1;
	    break;
	case SDLK_e:
	    if (level.object == WARP)
		level.object = -1;
	    break;
	case SDLK_x:
	    if (level.object == EMPTY)
		level.object = -1;
	    break;
	default:
	    break;
	}
	break;

    case SDL_KEYDOWN:
	if (event->key.keysym.mod & KMOD_SHIFT) {
	    switch (event->key.keysym.sym) {
	    case SDLK_UP:
		shift_lf();
		break;
	    case SDLK_DOWN:
		shift_rt();
		break;
	    case SDLK_RIGHT:
		shift_bk();
		break;
	    case SDLK_LEFT:
		shift_ft();
		break;
	    case SDLK_PAGEDOWN:
		shift_dn();
		break;
	    case SDLK_PAGEUP:
		shift_up();
		break;
	    default:
		break;
	    }
	    break;
	}
	if (event->key.keysym.mod & KMOD_CTRL) {
	    switch (event->key.keysym.sym) {
	    case SDLK_RIGHT:
		rotate_rt();
		break;
	    case SDLK_LEFT:
		rotate_lf();
		break;
	    default:
		break;
	    }
	    break;
	}
	switch (event->key.keysym.sym) {
	case SDLK_PAGEDOWN:
	    if (level.cursor.y > MIN)
		level.cursor.y--; 
	    break;
	case SDLK_PAGEUP:
	    if (level.cursor.y < MAX)
		level.cursor.y++;
	    break;
	case SDLK_UP:
	    if (level.cursor.x > MIN)
		level.cursor.x--;
	    break;
	case SDLK_DOWN:
	    if (level.cursor.x < MAX)
		level.cursor.x++;
	    break;
	case SDLK_RIGHT:
	    if (level.cursor.z > MIN)
		level.cursor.z--;
	    break;
	case SDLK_LEFT:
	    if (level.cursor.z < MAX)
		level.cursor.z++;
	    break;
	case SDLK_g:
	    level.object = GROUND;
	    break;
	case SDLK_c:
	    level.object = CRATE;
	    break;
	case SDLK_p:
	    level.object = WODOX;
	    break;
	case SDLK_j:
	    level.object = SMALL;
	    break;
	case SDLK_l:
	    level.object = TUBE;
	    break;
	case SDLK_u:
	    level.object = BELTLF;
	    break;
	case SDLK_o:
	    level.object = BELTBK;
	    break;
	case SDLK_i:
	    level.object = BELTRT;
	    break;
	case SDLK_y:
	    level.object = BELTFT;
	    break;
	case SDLK_m:
	    level.object = MOVING;
	    break;
	case SDLK_b:
	    level.object = BUTTON;
	    break;
	case SDLK_s:
	    level.object = SWITCH;
	    break;
	case SDLK_e:
	    level.object = WARP;
	    break;
	case SDLK_x:
	    level.object = EMPTY;
	    break;
	case SDLK_w:
	    if (properties(1 + strrchr(level.path, '/'))) {
		edit_save(level.path);
	    }
	    break;
	case SDLK_r:
	    edit_load(level.path);
	    break;
	case SDLK_t:
	    edit_save("level.tmp");
	    play("level.tmp", "Unnamed level");
	    break;
	case SDLK_ESCAPE:
	    level.keep_going = 0;
	    break;
	case SDLK_F1:
	    switch (help(str_howtoedit, str_menuedit, "\0twrq")) {
	    case 1:
		edit_save("level.tmp");
		play("level.tmp", "Unnamed level");
		break;
	    case 2:
		if (properties(1 + strrchr(level.path, '/'))) {
		    edit_save(level.path);
		}
		break;
	    case 3:
		edit_load(level.path);
		break;
	    case 4:
		level.keep_going = 0;
	    }
	    break;
	case SDLK_RETURN:
	    edit_circuit(coord_to_idx(level.cursor));
	default:
	    break;
	}
	break;

    case SDL_QUIT:
	exit(0);

    default:
	break;
    }
}

void
check_singletons(void)
{
    int i;

    if (level.object >= 0) {
	if (level.object == WODOX) {
	    for (i = 0; i < SIZE_3; ++i) {
		if (S_MAP[i] == WODOX) {
		    S_MAP[i] = EMPTY;
		}
	    }
	}

	if (level.object == WARP) {
	    for (i = 0; i < SIZE_3; ++i) {
		if (S_MAP[i] == WARP) {
		    S_MAP[i] = EMPTY;
		}
	    }
	}

	S_MAP[coord_to_idx(level.cursor)] = level.object;
    }
}

