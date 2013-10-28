/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013
 */

#include <errno.h>

#include "../media/media.h"
#include "../play/play.h"

#include "types.h"
#include "circuit.h"
#include "shift.h"
#include "rotate.h"
#include "render.h"
#include "file.h"

struct level level;


static void handle_player_input(void);
/*
 * Edit links.
 */
static int edit_circuit(uint16_t offset);

/*
 * To edit a level.
 */
int
edit(char *path)
{
    uint16_t i;
    uint16_t offset = 0;
    SDL_Rect dst;

    media.surface_levelname = NULL;

    level.keep_going = 1;
    level.cursor = OFF(0, 0, 0);
    level.object = -1;

    edit_load(path);

    if (path == NULL) 
	path = strjoin(PATH_SEPARATOR, USER_DIR, "sandbox", NULL);

    while (level.keep_going) {
	render_background();
	render_level();
	render_circuit();
	handle_player_input();
	render_foreground();
	media_sync();
    }

    edit_free();
    return 0;
}

void
handle_player_input(void)
{
    SDL_Event event;
    int i;

    while (SDL_PollEvent(&event) != 0) {
	switch (event.type) {
	case SDL_MOUSEMOTION:
	    do {
		int y = Y(level.cursor);
		int x =
		    (WORLDX(event.motion.x, event.motion.y, SPS * y) -
		     SPS / 2) / SPS;
		int z =
		    (WORLDZ(event.motion.x, event.motion.y, SPS * y) +
		     SPS / 2) / SPS;

		if (x < 0) {
		    x = 0;
		} else if (x >= SIZE) {
		    x = SIZE - 1;
		}
		if (z < 0) {
		    z = 0;
		} else if (z >= SIZE) {
		    z = SIZE - 1;
		}

		level.cursor = OFF(x, y, z);
	    }
	    while (0);
	    break;

	case SDL_MOUSEBUTTONDOWN:
	    switch (event.button.button) {
	    case SDL_BUTTON_LEFT:
		level.object = GROUND;
		break;
	    case SDL_BUTTON_WHEELDOWN:
		if (Y(level.cursor) > MIN)
		    level.cursor -= SIZE_2;
		break;
	    case SDL_BUTTON_WHEELUP:
		if (Y(level.cursor) < MAX)
		    level.cursor += SIZE_2;
		break;
	    default:
		break;
	    }
	    break;

	case SDL_MOUSEBUTTONUP:
	    switch (event.button.button) {
	    case SDL_BUTTON_LEFT:
		level.object = -1;
		break;
	    default:
		break;
	    }
	    break;

	case SDL_KEYUP:
	    switch (event.key.keysym.sym) {
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
	    if (event.key.keysym.mod & KMOD_SHIFT) {
		switch (event.key.keysym.sym) {
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
	    if (event.key.keysym.mod & KMOD_CTRL) {
		switch (event.key.keysym.sym) {
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
	    switch (event.key.keysym.sym) {
	    case SDLK_PAGEDOWN:
		if (Y(level.cursor) > MIN)
		    level.cursor -= SIZE_2;
		break;
	    case SDLK_PAGEUP:
		if (Y(level.cursor) < MAX)
		    level.cursor += SIZE_2;
		break;
	    case SDLK_UP:
		if (X(level.cursor) > MIN)
		    level.cursor -= SIZE;
		break;
	    case SDLK_DOWN:
		if (X(level.cursor) < MAX)
		    level.cursor += SIZE;
		break;
	    case SDLK_RIGHT:
		if (Z(level.cursor) > MIN)
		    level.cursor -= 1;
		break;
	    case SDLK_LEFT:
		if (Z(level.cursor) < MAX)
		    level.cursor += 1;
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
		edit_circuit(level.cursor);
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

	S_MAP[level.cursor] = level.object;
    }
}

/*
 * Edit circuit.
 */
int
edit_circuit(uint16_t offset)
{
    // Only edit if what we want to link is a machine.

    switch (S_MAP[offset]) {
    case TUBE:
    case BELTLF:
    case BELTRT:
    case BELTBK:
    case BELTFT:
    case MOVING:
	Mix_PlayChannel(-1, media.chunk_press, 0);
	break;

    default:
	Mix_PlayChannel(-1, media.chunk_release, 0);
	return -1;
    }

    // SDL stuff.

    SDL_Rect dst;
    SDL_Event event;
    SDL_Surface *surface;
    SDL_Surface *bkgr;

    // Local data.

    char buf[128];
    char *buf_ptr;

    // Loop control.

    int keep_going = 1;
    int blink = 0;
    int page = 1;

    SDL_EnableUNICODE(1);

    memset(buf, 0, sizeof(buf));
    buf_ptr = buf + circuit_to_text(buf, C_MAP + offset, 0, 0);

    // Freeze the background.

    render_background();
    render_level();
    sepia_surface(media.canvas);
    bkgr = SDL_ConvertSurface(media.canvas, media.canvas->format, media.canvas->flags);

    // Main loop.

    while (keep_going) {
	SDL_BlitSurface(bkgr, NULL, media.canvas, NULL);

	// Draw the circuit being edited.

	if ((surface =
	     TTF_RenderUTF8_Blended(media.font_equation, buf, color_white))) {
	    dst.x = (media.canvas->w - surface->w) / 2;
	    dst.y = 0;
	    SDL_BlitSurface(surface, NULL, media.canvas, &dst);

	    if ((++blink / 4) % 2) {
		dst.x += surface->w;
		dst.w = 20;
		dst.h = surface->h - 5;
		SDL_FillRect(media.canvas, &dst, 0xffffffff);
	    }

	    SDL_FreeSurface(surface);
	} else if ((++blink / 4) % 2) {
	    dst.x = media.canvas->w / 2;
	    dst.y = 0;
	    dst.w = 20;
	    dst.h = 20;
	    SDL_FillRect(media.canvas, &dst, 0xffffffff);
	}
	// Handle player input.

	while (SDL_PollEvent(&event) != 0) {
	    switch (event.type) {
	    case SDL_KEYDOWN:
		switch (event.key.keysym.sym) {
		case SDLK_ESCAPE:
		    keep_going = 0;
		    break;

		case SDLK_BACKSPACE:
		    if (buf_ptr != buf) {
			--buf_ptr;
			*buf_ptr = 0;
		    } else {
			if (C_MAP[offset].tree) {
			    free(C_MAP[offset].tree);
			    C_MAP[offset].tree = NULL;
			    C_MAP[offset].size = 0;
			}
			keep_going = 0;
		    }

		    Mix_PlayChannel(2, media.chunk_keystroke, 0);
		    break;

		case SDLK_RETURN:
		    text_to_circuit(buf, C_MAP + offset, 0);
		    keep_going = 0;
		    Mix_PlayChannel(2, media.chunk_keystroke, 0);
		    break;

		case SDLK_F1:
		    page = 1;

		    while (page)
			switch (page) {
			case 1:
			    page =
				help(str_howtolink, str_menulink,
				     "\01234");
			    break;
			case 2:
			    page =
				help(str_howtolink2, str_menulink,
				     "\01234");
			    break;
			case 3:
			    page =
				help(str_howtolink3, str_menulink,
				     "\01234");
			    break;
			case 4:
			    page =
				help(str_howtolink4, str_menulink,
				     "\01234");
			    break;
			}
		    break;

		default:
		    if (isprint(event.key.keysym.unicode) &&
			buf_ptr != buf + sizeof(buf)) {
			*buf_ptr = event.key.keysym.unicode;
			++buf_ptr;
			Mix_PlayChannel(2, media.chunk_keystroke, 0);
		    }

		    break;
		}
		break;

	    case SDL_QUIT:
		exit(0);

	    default:
		break;
	    }
	}

	// Some decorations.

	dst.x = (media.canvas->w - media.surface_hforhelp->w) / 2;
	dst.y = media.canvas->h - media.surface_hforhelp->h;
	SDL_BlitSurface(media.surface_hforhelp, NULL, media.canvas, &dst);

	SDL_BlitSurface(media.surface_frame, NULL, media.canvas, NULL);

	// Update screen.

	media_sync();
    }

    SDL_FreeSurface(bkgr);

    SDL_EnableUNICODE(0);

    return 0;
}
