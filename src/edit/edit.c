/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013
 */

#include <errno.h>

#include "../media/draw.h"
#include "../play/play.h"

#include "types.h"
#include "circuit.h"
#include "shift.h"
#include "rotate.h"
#include "render.h"

struct level level;

/*
 * Edit links.
 */
static int edit_circuit(uint16_t offset);

/*
 * Load and save a level.
 */
static int load(const char *path);
static int save(const char *path);

/*
 * To edit a level.
 */
int
edit(char *path)
{
    // Variables to iterate.

    uint16_t i;
    uint16_t offset = 0;
    int8_t object = -1;

    // SDL stuff

    SDL_Event event;
    SDL_Rect dst;

    // Circuits.

    char buf[256];
    SDL_Surface *surface;

    // Loop control

    int keep_going = 1;

    // Let's go.

    level.cursor = OFF(0, 0, 0);

    load(path);

    if (path == NULL) {
	path = strjoin(PATH_SEPARATOR, USER_DIR, "sandbox", NULL);
    }

    while (keep_going) {
	// Paint objects under the cursor.

	if (object >= 0) {
	    if (object == WODOX) {
		for (i = 0; i < SIZE_3; ++i) {
		    if (S_MAP[i] == WODOX) {
			S_MAP[i] = EMPTY;
		    }
		}
	    }

	    if (object == WARP) {
		for (i = 0; i < SIZE_3; ++i) {
		    if (S_MAP[i] == WARP) {
			S_MAP[i] = EMPTY;
		    }
		}
	    }

	    S_MAP[level.cursor] = object;
	}
	// Start drawing.

	draw_background();

	// Draw level.

	render();

	// Draw circuit under cursor.

	switch (S_MAP[level.cursor]) {
	case TUBE:
	case BELTLF:
	case BELTRT:
	case BELTBK:
	case BELTFT:
	case MOVING:
	    memset(buf, 0, sizeof(buf));
	    circuit_to_text(buf, C_MAP + level.cursor, 0, 0);

	    if ((surface =
		 TTF_RenderUTF8_Blended(font_equation, buf,
					color_white))) {
		dst.x = (canvas->w - surface->w) / 2;
		dst.y = 0;
		SDL_BlitSurface(surface, NULL, canvas, &dst);
		SDL_FreeSurface(surface);
	    }
	}

	// Handle player input.

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
		    object = GROUND;
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
		    object = -1;
		    break;
		default:
		    break;
		}
		break;

	    case SDL_KEYUP:
		switch (event.key.keysym.sym) {
		case SDLK_g:
		    if (object == GROUND)
			object = -1;
		    break;
		case SDLK_c:
		    if (object == CRATE)
			object = -1;
		    break;
		case SDLK_p:
		    if (object == WODOX)
			object = -1;
		    break;
		case SDLK_j:
		    if (object == SMALL)
			object = -1;
		    break;
		case SDLK_l:
		    if (object == TUBE)
			object = -1;
		    break;
		case SDLK_u:
		    if (object == BELTLF)
			object = -1;
		    break;
		case SDLK_o:
		    if (object == BELTBK)
			object = -1;
		    break;
		case SDLK_i:
		    if (object == BELTRT)
			object = -1;
		    break;
		case SDLK_y:
		    if (object == BELTFT)
			object = -1;
		    break;
		case SDLK_m:
		    if (object == MOVING)
			object = -1;
		    break;
		case SDLK_b:
		    if (object == BUTTON)
			object = -1;
		    break;
		case SDLK_s:
		    if (object == SWITCH)
			object = -1;
		    break;
		case SDLK_e:
		    if (object == WARP)
			object = -1;
		    break;
		case SDLK_x:
		    if (object == EMPTY)
			object = -1;
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
		    object = GROUND;
		    break;
		case SDLK_c:
		    object = CRATE;
		    break;
		case SDLK_p:
		    object = WODOX;
		    break;
		case SDLK_j:
		    object = SMALL;
		    break;
		case SDLK_l:
		    object = TUBE;
		    break;
		case SDLK_u:
		    object = BELTLF;
		    break;
		case SDLK_o:
		    object = BELTBK;
		    break;
		case SDLK_i:
		    object = BELTRT;
		    break;
		case SDLK_y:
		    object = BELTFT;
		    break;
		case SDLK_m:
		    object = MOVING;
		    break;
		case SDLK_b:
		    object = BUTTON;
		    break;
		case SDLK_s:
		    object = SWITCH;
		    break;
		case SDLK_e:
		    object = WARP;
		    break;
		case SDLK_x:
		    object = EMPTY;
		    break;
		case SDLK_w:
		    if (properties(1 + strrchr(path, '/'))) {
			save(path);
		    }
		    break;
		case SDLK_r:
		    load(path);
		    break;
		case SDLK_t:
		    save("level.tmp");
		    play("level.tmp", "Unnamed level");
		    break;
		case SDLK_ESCAPE:
		    keep_going = 0;
		    break;
		case SDLK_F1:
		    switch (help(str_howtoedit, str_menuedit, "\0twrq")) {
		    case 1:
			save("level.tmp");
			play("level.tmp", "Unnamed level");
			break;
		    case 2:
			if (properties(1 + strrchr(path, '/'))) {
			    save(path);
			}
			break;
		    case 3:
			load(path);
			break;
		    case 4:
			keep_going = 0;
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

	// Press 'h' for help.

	dst.x = (canvas->w - surface_hforhelp->w) / 2;
	dst.y = canvas->h - surface_hforhelp->h;
	SDL_BlitSurface(surface_hforhelp, NULL, canvas, &dst);

	SDL_BlitSurface(surface_frame, NULL, canvas, NULL);

	// Update timer and screen.

	sync();
    }

    for (offset = 0; offset < SIZE_3; ++offset)
	if (C_MAP[offset].tree) {
	    free(C_MAP[offset].tree);
	}

    return 0;
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
	Mix_PlayChannel(-1, chunk_press, 0);
	break;

    default:
	Mix_PlayChannel(-1, chunk_release, 0);
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

    draw_background();
    render();
    sepia_surface(canvas);
    bkgr = SDL_ConvertSurface(canvas, canvas->format, canvas->flags);

    // Main loop.

    while (keep_going) {
	SDL_BlitSurface(bkgr, NULL, canvas, NULL);

	// Draw the circuit being edited.

	if ((surface =
	     TTF_RenderUTF8_Blended(font_equation, buf, color_white))) {
	    dst.x = (canvas->w - surface->w) / 2;
	    dst.y = 0;
	    SDL_BlitSurface(surface, NULL, canvas, &dst);

	    if ((++blink / 4) % 2) {
		dst.x += surface->w;
		dst.w = 20;
		dst.h = surface->h - 5;
		SDL_FillRect(canvas, &dst, 0xffffffff);
	    }

	    SDL_FreeSurface(surface);
	} else if ((++blink / 4) % 2) {
	    dst.x = canvas->w / 2;
	    dst.y = 0;
	    dst.w = 20;
	    dst.h = 20;
	    SDL_FillRect(canvas, &dst, 0xffffffff);
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

		    Mix_PlayChannel(2, chunk_keystroke, 0);
		    break;

		case SDLK_RETURN:
		    text_to_circuit(buf, C_MAP + offset, 0);
		    keep_going = 0;
		    Mix_PlayChannel(2, chunk_keystroke, 0);
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
			Mix_PlayChannel(2, chunk_keystroke, 0);
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

	dst.x = (canvas->w - surface_hforhelp->w) / 2;
	dst.y = canvas->h - surface_hforhelp->h;
	SDL_BlitSurface(surface_hforhelp, NULL, canvas, &dst);

	SDL_BlitSurface(surface_frame, NULL, canvas, NULL);

	// Update screen.

	sync();
    }

    SDL_FreeSurface(bkgr);

    SDL_EnableUNICODE(0);

    return 0;
}


/*
 * Load level.
 */
int
load(const char *path)
{
    FILE *f;
    uint16_t n;
    memset(level.static_map, EMPTY, sizeof(uint8_t) * SIZE_3);
    memset(level.circuit_map, 0, sizeof(struct circuit) * SIZE_3);
    if ((f = fopen(path, "rb"))) {
	fread(level.static_map, sizeof(uint8_t), SIZE_3, f);
	while (fread(&n, sizeof(uint16_t), 1, f) == 1 && n < SIZE_3) {
	    fread(&C_MAP[n].size, sizeof(uint16_t), 1, f);
	    //C_MAP[n].size = TREE_SIZE;
	    C_MAP[n].tree =
		(uint16_t *) malloc(sizeof(uint16_t) * C_MAP[n].size);
	    fread(C_MAP[n].tree, sizeof(uint16_t), C_MAP[n].size, f);
	}
	fclose(f);
	return 1;
    }
    fprintf(stderr, "%s: %s\n", path, strerror(errno));
    return 0;
}

/*
 * Save level.
 */
int
save(const char *path)
{
    FILE *f;
    uint16_t n;
    if ((f = fopen(path, "wb"))) {
	fwrite(level.static_map, sizeof(uint8_t), SIZE_3, f);
	for (n = 0; n < SIZE_3; ++n)
	    if (C_MAP[n].tree) {
		fwrite(&n, sizeof(uint16_t), 1, f);
		fwrite(&C_MAP[n].size, sizeof(uint16_t), 1, f);
		fwrite(C_MAP[n].tree, sizeof(uint16_t), C_MAP[n].size, f);
	    }
	fclose(f);
	return 1;
    }
    fprintf(stderr, "%s: %s\n", path, strerror(errno));
    return 0;
}

