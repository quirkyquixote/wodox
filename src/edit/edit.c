/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009 
 */

#include "../edit/edit.h"
#include "../menu/menu.h"
#include "../play/play.h"

#include <errno.h>
#include "types.h"

struct level level;

/*
 * Edit links.
 */
static int edit_circuit(size_t offset);

/*
 * Draw objects.
 */
int draw();

/*
 * Load and save a level.
 */
static int load(const char *path);
static int save(const char *path);

/*
 * Shift the level in some direction.
 */
static void shift_bk();
static void shift_ft();
static void shift_lf();
static void shift_rt();
static void shift_up();
static void shift_dn();

/*
 * Shift a circuit in some direction.
 */
static struct circuit shift_circuit_bk(struct circuit c);
static struct circuit shift_circuit_ft(struct circuit c);
static struct circuit shift_circuit_lf(struct circuit c);
static struct circuit shift_circuit_rt(struct circuit c);
static struct circuit shift_circuit_up(struct circuit c);
static struct circuit shift_circuit_dn(struct circuit c);

/*
 * Rotate a level in some direction.
 */
static void rotate_lf();
static void rotate_rt();

/*
 * Rotate an object in some direction. This basically swaps one type of
 * conveyor belt for another.
 */
static uint8_t rotate_object_lf(uint8_t p);
static uint8_t rotate_object_rt(uint8_t p);

/*
 * Rotate a circuit in some direction.
 */
static struct circuit rotate_circuit_lf(struct circuit c);
static struct circuit rotate_circuit_rt(struct circuit c);

/*
 * To parse a circuit from text and format text from a circuit.
 */
static int circuit_to_text(char *buf, struct circuit *c, size_t node,
			   int is_right);
static int text_to_circuit(char *buf, struct circuit *c, size_t node);

static void tree_to_circuit(struct tree *t, struct circuit *c,
			    size_t node);

static struct tree *parse_expr_1(char **buf);
static struct tree *parse_expr_2(char **buf);
static struct tree *parse_expr_3(char **buf);
static struct tree *parse_expr_4(char **buf);
static struct tree *parse_expr_5(char **buf);

static void consume(char **buf);

static int token;

/*
 * The cursor.
 */
static uint16_t cursor;

/*
 * To edit a level.
 */
int
edit(char *path)
{
    // Variables to iterate.

    size_t i;
    size_t offset = 0;
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

    cursor = OFF(0, 0, 0);

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

	    S_MAP[cursor] = object;
	}
	// Start drawing.

	draw_background();

	// Draw level.

	draw();

	// Draw circuit under cursor.

	switch (S_MAP[cursor]) {
	case TUBE:
	case BELTLF:
	case BELTRT:
	case BELTBK:
	case BELTFT:
	case MOVING:
	    memset(buf, 0, sizeof(buf));
	    circuit_to_text(buf, C_MAP + cursor, 0, 0);

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
		    int y = Y(cursor);
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

		    cursor = OFF(x, y, z);
		}
		while (0);
		break;

	    case SDL_MOUSEBUTTONDOWN:
		switch (event.button.button) {
		case SDL_BUTTON_LEFT:
		    object = GROUND;
		    break;
		case SDL_BUTTON_WHEELDOWN:
		    if (Y(cursor) > MIN)
			cursor -= SIZE_2;
		    break;
		case SDL_BUTTON_WHEELUP:
		    if (Y(cursor) < MAX)
			cursor += SIZE_2;
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
		    if (Y(cursor) > MIN)
			cursor -= SIZE_2;
		    break;
		case SDLK_PAGEUP:
		    if (Y(cursor) < MAX)
			cursor += SIZE_2;
		    break;
		case SDLK_UP:
		    if (X(cursor) > MIN)
			cursor -= SIZE;
		    break;
		case SDLK_DOWN:
		    if (X(cursor) < MAX)
			cursor += SIZE;
		    break;
		case SDLK_RIGHT:
		    if (Z(cursor) > MIN)
			cursor -= 1;
		    break;
		case SDLK_LEFT:
		    if (Z(cursor) < MAX)
			cursor += 1;
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
		    edit_circuit(cursor);
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
edit_circuit(size_t offset)
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
    draw();
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
 * Draw objects.
 */
int
draw()
{
    size_t i, j, k;
    SDL_Rect dst;
    SDL_Surface *surface;
    char lil_buf[4];

    // Draw all objects. Most objects have only a simple sprite with no 
    // alpha channel, but levitators have alpha transparency and a "warp"
    // effect, and the exit is a particle system.

    for (i = 0; i < SIZE; ++i)
	for (k = 0; k < SIZE; ++k) {
	    for (j = 0; j < SIZE; ++j) {
		if (level.circuit_map[j][i][k].tree) {
		    dst.x = SCREENX(SPS * i, SPS * j, SPS * k);
		    dst.y = SCREENY(SPS * i, SPS * j, SPS * k);
		    draw_effect(1, 1, &dst);
		}
		switch (level.static_map[j][i][k]) {
		case GROUND ... SWITCH:
		    dst.x = SCREENX(SPS * i, SPS * j, SPS * k);
		    dst.y = SCREENY(SPS * i, SPS * j, SPS * k);
		    draw_object(level.static_map[j][i][k] - 1, 0, &dst);
		    break;

		case TUBE:
		    dst.x = SCREENX(SPS * i, SPS * j, SPS * k);
		    dst.y = SCREENY(SPS * i, SPS * j, SPS * k);
		    if (level.static_map[j + 1][i][k] != TUBE) {
			draw_effect(1, 0, &dst);
		    }
		    draw_effect(0, 0, &dst);
		    break;

		case WARP:
		    dst.x = SCREENX(SPS * i, SPS * j, SPS * k);
		    dst.y = SCREENY(SPS * i, SPS * j, SPS * k);
		    draw_particles(canvas, &dst);
		    break;

		default:
		    if (j == Y(cursor)) {
			dst.x = SCREENX(SPS * i, SPS * j, SPS * k);
			dst.y = SCREENY(SPS * i, SPS * j, SPS * k);
			draw_effect(3, 1, &dst);
		    }
		}
		if (cursor == OFF(i, j, k)) {
		    dst.x = SCREENX(SPS * i, SPS * j, SPS * k);
		    dst.y = SCREENY(SPS * i, SPS * j, SPS * k);
		    draw_effect(0, 1, &dst);
		}
	    }
	    for (j = 0; j < SIZE; ++j) {
		if (level.static_map[j][i][k] == TUBE) {
		    dst.x = SCREENX(SPS * i, SPS * j, SPS * k);
		    dst.y = SCREENY(SPS * i, SPS * j, SPS * k);
		    warp_surface(canvas, &dst);
		}
	    }
	    for (j = 0; j < SIZE; ++j) {
		if (level.circuit_map[j][i][k].tree) {
		    dst.x = SCREENX(SPS * i, SPS * j, SPS * k);
		    dst.y = SCREENY(SPS * i, SPS * j, SPS * k);
		    draw_effect(2, 1, &dst);
		}
	    }
	}

    // Label all buttons and switches.

    for (i = 0; i < SIZE; ++i)
	for (j = 0; j < SIZE; ++j)
	    for (k = 0; k < SIZE; ++k)
		switch (level.static_map[i][j][k]) {
		case BUTTON:
		case SWITCH:
		    sprintf(lil_buf, "%d%d%d", i, j, k);
		    if ((surface =
			 TTF_RenderUTF8_Blended(font_normal, lil_buf,
						color_white))) {
			dst.x =
			    SCREENX(SPS * j, SPS * i,
				    SPS * k) - surface->w / 2 + 30;
			dst.y =
			    SCREENY(SPS * j, SPS * i,
				    SPS * k) - surface->h / 2 + 10;
			SDL_BlitSurface(surface, NULL, canvas, &dst);
			SDL_FreeSurface(surface);
		    }

		default:
		    break;
		}

    // Draw the cursor
    /*
       dst.x = SCREENX (SPS * X (cursor), SPS * Y (cursor), SPS * Z (cursor));
       dst.y = SCREENY (SPS * X (cursor), SPS * Y (cursor), SPS * Z (cursor));
       draw_object (0, 1, &dst);
     */
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

/*
 * The following methods 'shift' the level in one direction.
 */
void
shift_bk()
{
    uint8_t i, j, k, tmp;
    struct circuit tmp2;
    for (i = 0; i < SIZE; ++i)
	for (j = 0; j < SIZE; ++j) {
	    tmp = level.static_map[j][i][MIN];
	    tmp2 = level.circuit_map[j][i][MIN];
	    for (k = MIN; k < MAX; ++k) {
		level.static_map[j][i][k] = level.static_map[j][i][k + 1];
		level.circuit_map[j][i][k] = shift_circuit_bk(level.circuit_map[j][i][k + 1]);
	    }
	    level.static_map[j][i][MAX] = tmp;
	    level.circuit_map[j][i][MAX] = shift_circuit_bk(tmp2);
	}
}

void
shift_ft()
{
    uint8_t i, j, k, tmp;
    struct circuit tmp2;
    for (i = 0; i < SIZE; ++i)
	for (j = 0; j < SIZE; ++j) {
	    tmp = level.static_map[j][i][MAX];
	    tmp2 = level.circuit_map[j][i][MAX];
	    for (k = MAX; k > MIN; --k) {
		level.static_map[j][i][k] = level.static_map[j][i][k - 1];
		level.circuit_map[j][i][k] = shift_circuit_ft(level.circuit_map[j][i][k - 1]);
	    }
	    level.static_map[j][i][MIN] = tmp;
	    level.circuit_map[j][i][MIN] = shift_circuit_ft(tmp2);
	}
}

void
shift_lf()
{
    uint8_t i, j, k, tmp;
    struct circuit tmp2;
    for (k = 0; k < SIZE; ++k)
	for (j = 0; j < SIZE; ++j) {
	    tmp = level.static_map[j][MIN][k];
	    tmp2 = level.circuit_map[j][MIN][k];
	    for (i = MIN; i < MAX; ++i) {
		level.static_map[j][i][k] = level.static_map[j][i + 1][k];
		level.circuit_map[j][i][k] = shift_circuit_lf(level.circuit_map[j][i + 1][k]);
	    }
	    level.static_map[j][MAX][k] = tmp;
	    level.circuit_map[j][MAX][k] = shift_circuit_lf(tmp2);
	}
}

void
shift_rt()
{
    uint8_t i, j, k, tmp;
    struct circuit tmp2;
    for (k = 0; k < SIZE; ++k)
	for (j = 0; j < SIZE; ++j) {
	    tmp = level.static_map[j][MAX][k];
	    tmp2 = level.circuit_map[j][MAX][k];
	    for (i = MAX; i > MIN; --i) {
		level.static_map[j][i][k] = level.static_map[j][i - 1][k];
		level.circuit_map[j][i][k] = shift_circuit_rt(level.circuit_map[j][i - 1][k]);
	    }
	    level.static_map[j][MIN][k] = tmp;
	    level.circuit_map[j][MIN][k] = shift_circuit_rt(tmp2);
	}
}

void
shift_dn()
{
    uint8_t i, j, k, tmp;
    struct circuit tmp2;
    for (i = 0; i < SIZE; ++i)
	for (k = 0; k < SIZE; ++k) {
	    tmp = level.static_map[MIN][i][k];
	    tmp2 = level.circuit_map[MIN][i][k];
	    for (j = MIN; j < MAX; ++j) {
		level.static_map[j][i][k] = level.static_map[j + 1][i][k];
		level.circuit_map[j][i][k] = shift_circuit_dn(level.circuit_map[j + 1][i][k]);
	    }
	    level.static_map[MAX][i][k] = tmp;
	    level.circuit_map[MAX][i][k] = shift_circuit_dn(tmp2);
	}
}

void
shift_up()
{
    uint8_t i, j, k, tmp;
    struct circuit tmp2;
    for (i = 0; i < SIZE; ++i)
	for (k = 0; k < SIZE; ++k) {
	    tmp = level.static_map[MAX][i][k];
	    tmp2 = level.circuit_map[MAX][i][k];
	    for (j = MAX; j > MIN; --j) {
		level.static_map[j][i][k] = level.static_map[j - 1][i][k];
		level.circuit_map[j][i][k] = shift_circuit_up(level.circuit_map[j - 1][i][k]);
	    }
	    level.static_map[MIN][i][k] = tmp;
	    level.circuit_map[MIN][i][k] = shift_circuit_up(tmp2);
	}
}

/*
 * The following methods 'shift' a circuit in one direction.
 */
struct circuit
shift_circuit_ft(struct circuit n)
{
    uint16_t *it;
    if (n.tree)
	for (it = n.tree; it < n.tree + n.size; ++it)
	    if (*it < NONE) {
		*it = SIZE * (*it / SIZE) + (((*it % SIZE) + 1) % SIZE);
	    }
    return n;
}

struct circuit
shift_circuit_bk(struct circuit n)
{
    uint16_t *it;
    if (n.tree)
	for (it = n.tree; it < n.tree + n.size; ++it)
	    if (*it < NONE) {
		*it =
		    SIZE * (*it / SIZE) +
		    (((*it % SIZE) + SIZE - 1) % SIZE);
	    }
    return n;
}

struct circuit
shift_circuit_up(struct circuit n)
{
    uint16_t *it;
    if (n.tree)
	for (it = n.tree; it < n.tree + n.size; ++it)
	    if (*it < NONE) {
		*it = (*it + SIZE_2) % SIZE_3;
	    }
    return n;
}

struct circuit
shift_circuit_dn(struct circuit n)
{
    uint16_t *it;
    if (n.tree)
	for (it = n.tree; it < n.tree + n.size; ++it)
	    if (*it < NONE) {
		*it = (*it + SIZE_3 - SIZE_2) % SIZE_3;
	    }
    return n;
}

struct circuit
shift_circuit_rt(struct circuit n)
{
    uint16_t *it;
    if (n.tree)
	for (it = n.tree; it < n.tree + n.size; ++it)
	    if (*it < NONE) {
		*it =
		    SIZE_2 * (*it / SIZE_2) +
		    (((*it % SIZE_2) + SIZE) % SIZE_2);
	    }
    return n;
}

struct circuit
shift_circuit_lf(struct circuit n)
{
    uint16_t *it;
    if (n.tree)
	for (it = n.tree; it < n.tree + n.size; ++it)
	    if (*it < NONE) {
		*it =
		    SIZE_2 * (*it / SIZE_2) +
		    (((*it % SIZE_2) + SIZE_2 - SIZE) % SIZE_2);
	    }
    return n;
}

/*
 * The following methods rotate the level.
 */
void
rotate_lf()
{
    uint8_t i, j, k, m, n, tmp;
    struct circuit tmp2;
    for (j = 0; j < SIZE; ++j) {
	for (i = MIN, m = MAX; i < m; ++i, --m)
	    for (k = MIN, n = MAX; k < n; ++k, --n) {
		tmp = level.static_map[j][i][k];
		level.static_map[j][i][k] = rotate_object_lf(level.static_map[j][n][i]);
		level.static_map[j][n][i] = rotate_object_lf(level.static_map[j][m][n]);
		level.static_map[j][m][n] = rotate_object_lf(level.static_map[j][k][m]);
		level.static_map[j][k][m] = rotate_object_lf(tmp);

		tmp2 = level.circuit_map[j][i][k];
		level.circuit_map[j][i][k] = rotate_circuit_lf(level.circuit_map[j][n][i]);
		level.circuit_map[j][n][i] = rotate_circuit_lf(level.circuit_map[j][m][n]);
		level.circuit_map[j][m][n] = rotate_circuit_lf(level.circuit_map[j][k][m]);
		level.circuit_map[j][k][m] = rotate_circuit_lf(tmp2);
	    }
    }
}

void
rotate_rt()
{
    uint8_t i, j, k, m, n, tmp;
    struct circuit tmp2;
    for (j = 0; j < SIZE; ++j) {
	for (i = MIN, m = MAX; i < m; ++i, --m)
	    for (k = MIN, n = MAX; k < n; ++k, --n) {
		tmp = level.static_map[j][i][k];
		level.static_map[j][i][k] = rotate_object_rt(level.static_map[j][k][m]);
		level.static_map[j][k][m] = rotate_object_rt(level.static_map[j][m][n]);
		level.static_map[j][m][n] = rotate_object_rt(level.static_map[j][n][i]);
		level.static_map[j][n][i] = rotate_object_rt(tmp);

		tmp2 = level.circuit_map[j][i][k];
		level.circuit_map[j][i][k] = rotate_circuit_rt(level.circuit_map[j][k][m]);
		level.circuit_map[j][k][m] = rotate_circuit_rt(level.circuit_map[j][m][n]);
		level.circuit_map[j][m][n] = rotate_circuit_rt(level.circuit_map[j][n][i]);
		level.circuit_map[j][n][i] = rotate_circuit_rt(tmp2);
	    }
    }
}

/*
 * These rotate objects whose orientation matters.
 */
uint8_t
rotate_object_lf(uint8_t p)
{
    switch (p) {
    case BELTLF:
	return BELTFT;
    case BELTBK:
	return BELTLF;
    case BELTRT:
	return BELTBK;
    case BELTFT:
	return BELTRT;
    }
    return p;
}

uint8_t
rotate_object_rt(uint8_t p)
{
    switch (p) {
    case BELTLF:
	return BELTBK;
    case BELTBK:
	return BELTRT;
    case BELTRT:
	return BELTFT;
    case BELTFT:
	return BELTLF;
    }
    return p;
}

/*
 * These rotate circuits.
 */
struct circuit
rotate_circuit_lf(struct circuit n)
{
    uint16_t *it;
    uint8_t i, j, k;
    if (n.tree)
	for (it = n.tree; it < n.tree + n.size; ++it)
	    if (*it < NONE) {
		i = X(*it);
		j = Y(*it);
		k = Z(*it);
		*it = OFF(k, j, MAX - i);
	    }
    return n;
}

struct circuit
rotate_circuit_rt(struct circuit n)
{
    uint16_t *it;
    uint8_t i, j, k;
    if (n.tree)
	for (it = n.tree; it < n.tree + n.size; ++it)
	    if (*it < NONE) {
		i = X(*it);
		j = Y(*it);
		k = Z(*it);
		*it = OFF(MAX - k, j, i);
	    }
    return n;
}


/*
 * To read and write circuits.
 */
int
circuit_to_text(char *buf, struct circuit *c, size_t node, int is_right)
{
    int ret = 0;

    if (c->tree)
	switch (c->tree[node]) {
	case AND:
	    if (is_right) {
		ret += sprintf(buf, "(");
	    }
	    ret += circuit_to_text(buf + ret, c, LCHILD(node), 0);
	    ret += sprintf(buf + ret, " and ");
	    ret += circuit_to_text(buf + ret, c, RCHILD(node), 1);
	    if (is_right) {
		ret += sprintf(buf + ret, ")");
	    }
	    break;

	case OR:
	    if (is_right) {
		ret += sprintf(buf, "(");
	    }
	    ret += circuit_to_text(buf + ret, c, LCHILD(node), 0);
	    ret += sprintf(buf + ret, " or ");
	    ret += circuit_to_text(buf + ret, c, RCHILD(node), 1);
	    if (is_right) {
		ret += sprintf(buf + ret, ")");
	    }
	    break;

	case XOR:
	    if (is_right) {
		ret += sprintf(buf, "(");
	    }
	    ret += circuit_to_text(buf + ret, c, LCHILD(node), 0);
	    ret += sprintf(buf + ret, " xor ");
	    ret += circuit_to_text(buf + ret, c, RCHILD(node), 1);
	    if (is_right) {
		ret += sprintf(buf + ret, ")");
	    }
	    break;

	case NOT:
	    ret += sprintf(buf, "not ");
	    ret += circuit_to_text(buf + ret, c, LCHILD(node), 1);
	    break;

	case NONE:
	    break;

	default:
	    ret += sprintf(buf, "%d", c->tree[node]);
	    break;
	}
    return ret;
}


/*
 * To transform from text to circuit we parse the string, make a tree of the
 * expression and walk the tree to generate a circuit.
 */
int
text_to_circuit(char *buf, struct circuit *c, size_t node)
{
    c->size = TREE_SIZE;
    c->tree = realloc(c->tree, sizeof(uint16_t) * c->size);
    consume(&buf);
    tree_to_circuit(parse_expr_1(&buf), c, node);
    return 0;
}

void
tree_to_circuit(struct tree *t, struct circuit *c, size_t node)
{
    if (t) {
	if (node > c->size) {
	    c->size *= 2;
	    c->tree = realloc(c->tree, sizeof(uint16_t) * c->size);
	}
	c->tree[node] = t->token;
	tree_to_circuit(t->l, c, LCHILD(node));
	tree_to_circuit(t->r, c, RCHILD(node));
	free(t);
    }
}

#define TOKEN_AND	1000
#define TOKEN_OR	1001
#define TOKEN_XOR	1002
#define TOKEN_NOT	1003
#define TOKEN_LPAREN	1004
#define TOKEN_RPAREN	1005
#define TOKEN_EOF	1006

struct tree *
parse_expr_1(char **buf)
{
    struct tree *t = parse_expr_2(buf);
    if (token == TOKEN_OR) {
	struct tree *p = (struct tree *) malloc(sizeof(struct tree));
	p->token = OR;
	p->l = t;
	consume(buf);
	p->r = parse_expr_1(buf);
	return p;
    }
    return t;
}

struct tree *
parse_expr_2(char **buf)
{
    struct tree *t = parse_expr_3(buf);
    if (token == TOKEN_AND) {
	struct tree *p = (struct tree *) malloc(sizeof(struct tree));
	p->token = AND;
	p->l = t;
	consume(buf);
	p->r = parse_expr_2(buf);
	return p;
    }
    return t;
}

struct tree *
parse_expr_3(char **buf)
{
    struct tree *t = parse_expr_4(buf);
    if (token == TOKEN_XOR) {
	struct tree *p = (struct tree *) malloc(sizeof(struct tree));
	p->token = XOR;
	p->l = t;
	consume(buf);
	p->r = parse_expr_3(buf);
	return p;
    }
    return t;
}

struct tree *
parse_expr_4(char **buf)
{
    if (token == TOKEN_NOT) {
	struct tree *t = (struct tree *) malloc(sizeof(struct tree));
	t->token = NOT;
	consume(buf);
	t->l = parse_expr_4(buf);
	t->r = NULL;
	return t;
    }
    return parse_expr_5(buf);
}

struct tree *
parse_expr_5(char **buf)
{
    if (token == TOKEN_EOF) {
	return NULL;
    }
    if (token == TOKEN_LPAREN) {
	consume(buf);
	struct tree *t = parse_expr_1(buf);
	if (token == TOKEN_RPAREN) {
	    consume(buf);
	}
	return t;
    }
    struct tree *t = (struct tree *) malloc(sizeof(struct tree));
    t->token = token;
    t->l = NULL;
    t->r = NULL;
    consume(buf);
    return t;
}

void
consume(char **buf)
{
    int i = 0;
    char tok[128];

    for (;;)
	switch (**buf) {
	case '(':
	    ++*buf;
	    token = TOKEN_LPAREN;
	    return;
	case ')':
	    ++*buf;
	    token = TOKEN_RPAREN;
	    return;
	case '0' ... '9':
	    token = 0;
	    do {
		token *= 10;
		token += **buf - '0';
		++*buf;
	    }
	    while (**buf >= '0' && **buf <= '9');
	    return;

	case ' ':
	    ++*buf;
	    break;
	case 'a' ... 'z':
	    i = 0;
	    do {
		tok[i] = **buf;
		++*buf;
		++i;
	    }
	    while (**buf >= 'a' && **buf <= 'z');
	    tok[i] = 0;
	    token =
		strcmp(tok, "or") == 0 ? TOKEN_OR : strcmp(tok,
							   "and") ==
		0 ? TOKEN_AND : strcmp(tok,
				       "xor") ==
		0 ? TOKEN_XOR : strcmp(tok,
				       "not") == 0 ? TOKEN_NOT : TOKEN_EOF;
	    return;

	case 0:
	    token = TOKEN_EOF;
	    return;
	}
}
