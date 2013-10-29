/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009 
 */

#include <string.h>		// For memset, strerror
#include <errno.h>		// for errno

#include "../play/play.h"	// Game engine.
#include "../edit/edit.h"	// Level editor.
#include "../menu/menu.h"
#include "../media/media.h"

#include "types.h"

const SDL_Color black = { 0, 0, 0 };
const SDL_Color white = { 255, 255, 255 };

struct context ctx;

static int select_user();

static int render_main_menu();
static int run_main_menu();

static int render_sandbox_menu();
static int run_sandbox_menu();

static void shift_menu(int new_offset);
static void blit_menu();
static void play_from(int i);

static void load_background();

static void run_transition(int i);

/*
 * Menus.
 */
int
menu(void)
{
    FILE *f;
    char *path;
    char buf[1024];

    ctx.level_list = NULL;
    ctx.level_count = 0;
    ctx.offset = -400;

    path = strjoin(PATH_SEPARATOR, DATA_DIR, "levels", "level.lst", NULL);

    if ((f = fopen(path, "r"))) {
	while (fgets(buf, 1024, f)) {
	    if (ctx.level_count % 8 == 0) {
		ctx.level_list =
		    realloc(ctx.level_list,
			    sizeof(struct level) * (ctx.level_count + 8));
	    }

	    struct level *l = ctx.level_list + ctx.level_count;
	    sscanf(buf, "%s %d", l->file, &l->track);
	    ++ctx.level_count;
	}

	fclose(f);
    }

    free(path);

    for (;;) {
	load_background();
	select_user();
	run_main_menu();
	SDL_FreeSurface(ctx.surface_menu);
    }

    free(ctx.level_list);

    return 0;
}

/*
 * User selection screen.
 */
int
select_user()
{
    int keep_going = 1;
    char *user_name_end = user.name;

    SDL_Rect rect;
    SDL_Event event;
    SDL_Surface *tmp;

    SDL_EnableUNICODE(1);

    memset(&user, 0, sizeof(user));

    while (keep_going) {
	render_background();

	blit_menu();

	if ((tmp =
	     TTF_RenderUTF8_Blended(media.font_large, lang.entername, white))) {
	    rect.x = 300 - tmp->w / 2;
	    rect.y = 200 - tmp->h / 2;
	    SDL_BlitSurface(tmp, NULL, media.canvas, &rect);
	    SDL_FreeSurface(tmp);
	}

	while (SDL_PollEvent(&event) != 0) {
	    switch (event.type) {
	    case SDL_KEYDOWN:
		switch (event.key.keysym.sym) {
		case SDLK_ESCAPE:
		    exit(0);

		case SDLK_RETURN:
		    if (user_name_end != user.name) {
			Mix_PlayChannel(2, media.chunk_keystroke, 0);
			load_profile(user.name);
			keep_going = 0;
		    }
		    break;

		case SDLK_BACKSPACE:
		    if (user_name_end != user.name) {
			Mix_PlayChannel(2, media.chunk_keystroke, 0);
			--user_name_end;
			*user_name_end = 0;
		    }
		    break;

		default:
		    if (user_name_end != user.name + USER_NAME_LEN &&
			isprint(event.key.keysym.unicode)) {
			Mix_PlayChannel(2, media.chunk_keystroke, 0);
			*user_name_end = event.key.keysym.unicode;
			++user_name_end;
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

	media_sync();
    }

    SDL_EnableUNICODE(0);

    return 1;
}

#define left_absolute 220
#define left_relative 70

#define top_main 0
#define top_sandbox 2000

/*
 * Main menu.
 */
int
render_main_menu()
{
    SDL_Rect rect;
    SDL_Surface *tmp;
    int i;

    for (i = 0; i < 3; ++i)
	if ((tmp =
	     TTF_RenderUTF8_Blended(media.font_large, lang.menumain[i], white))) {
	    rect.x = left_relative;
	    rect.y = top_main + 200 + 80 * i;
	    SDL_BlitSurface(tmp, NULL, ctx.surface_menu, &rect);
	    SDL_FreeSurface(tmp);
	}

    if (user.max_level > 0) {
	for (i = 0; i <= user.max_level; ++i)
	    if ((tmp =
		 TTF_RenderUTF8_Blended(media.font_normal, lang.levelnames[i],
					white))) {
		rect.x = left_relative;
		rect.y = top_main + 450 + 24 * i;
		SDL_BlitSurface(tmp, NULL, ctx.surface_menu, &rect);
		SDL_FreeSurface(tmp);
	    }
	return user.max_level + 4;
    }

    return 3;
}

int
run_main_menu()
{
    // SDL stuff

    SDL_Event event;
    SDL_Rect rect;

    // Loop control

    int option = 1;
    int keep_going = 1;
    int w, h;
    int max_option;

    // Draw the menu in a surface.

    if (user.max_level > ctx.level_count - 1) {
	user.max_level = ctx.level_count - 1;
    }

    max_option = render_main_menu();
    shift_menu(top_main);
    play_music(0);

    // Go on.

    while (keep_going) {
	render_background();
	blit_menu();

	for (;;) {
	    switch (option) {
	    case 0:
		ctx.offset = 0;
		TTF_SizeText(media.font_input, user.name, &w, &h);
		rect.x = left_absolute - 20;
		rect.y = top_main + 130;
		rect.w = 200;
		rect.h = h - 10;
		break;

	    case 1 ... 3:
		TTF_SizeText(media.font_large, lang.menumain[option - 1], &w, &h);
		rect.x = left_absolute - 20;
		rect.y = top_main + 130 + 80 * option - ctx.offset;
		rect.w = 200;
		rect.h = h - 10;
		break;

	    default:
		rect.x = left_absolute - 20;
		rect.y = top_main + 458 + 24 * (option - 4) - ctx.offset;
		rect.w = 200;
		rect.h = 12;
		break;
	    }
	    if (rect.y - rect.h < 0) {
		ctx.offset -= 24;
	    } else if (rect.y + rect.h > media.canvas->h) {
		ctx.offset += 24;
	    } else
		break;
	}
	render_spark(media.canvas, &rect);

	// Handle player input.

	while (SDL_PollEvent(&event) != 0) {
	    switch (event.type) {
	    case SDL_KEYDOWN:
		switch (event.key.keysym.sym) {
		case SDLK_UP:
		    if (option > 0) {
			--option;
		    }
		    break;
		case SDLK_DOWN:
		    if (option < max_option) {
			++option;
		    }
		    break;

		case SDLK_PAGEUP:
		    option -= 3;
		    if (option < 0) {
			option = 0;
		    }
		    break;
		case SDLK_PAGEDOWN:
		    option += 3;
		    if (option > max_option) {
			option = max_option;
		    }
		    break;

		case SDLK_RETURN:
		    switch (option) {
		    case 0:
			keep_going = 0;
			break;
		    case 1:
			play_from(user.max_level);
			option = 1;
			ctx.offset = 0;
			break;
		    case 2:
			run_sandbox_menu();
			break;
		    case 3:
			exit(0);
		    default:
			play_from(option - 4);	/* option = 1; offset = 0; */
			break;
		    }
		    SDL_FreeSurface(ctx.surface_menu);
		    load_background();
		    max_option = render_main_menu();
		    break;

		case SDLK_ESCAPE:
		    keep_going = 0;
		    break;

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

	media_sync();
    }

    shift_menu(-400);
    play_music(-1);

    return 0;
}

/*
 * The sandbox menu.
 */
int
render_sandbox_menu()
{
    int i;
    SDL_Rect rect;
    SDL_Surface *tmp;

    for (i = 0; i < 1; ++i)
	if ((tmp =
	     TTF_RenderUTF8_Blended(media.font_large, lang.menusandbox[i],
				    white))) {
	    rect.x = left_relative;
	    rect.y = top_sandbox + 50 + 80 * i;
	    SDL_BlitSurface(tmp, NULL, ctx.surface_menu, &rect);
	    SDL_FreeSurface(tmp);
	}

    for (i = 0; i < ctx.sandbox_count; ++i)
	if ((tmp =
	     TTF_RenderUTF8_Blended(media.font_normal, ctx.sandbox_list[i],
				    white))) {
	    rect.x = left_relative;
	    rect.y = top_sandbox + 130 + 24 * i;
	    SDL_BlitSurface(tmp, NULL, ctx.surface_menu, &rect);
	    SDL_FreeSurface(tmp);
	}
    return 0;
}


int
run_sandbox_menu()
{
    char *dir;
    char *path;
    SDL_Rect rect;
    SDL_Event event;
    int keep_going = 1;
    int w, h;
    int option = 0;

    dir = strjoin(PATH_SEPARATOR, USER_DIR, "sandbox", NULL);

    ctx.sandbox_count = list_folder(dir, &ctx.sandbox_list);
    render_sandbox_menu();
    shift_menu(top_sandbox);

    while (keep_going) {
	render_background();
	blit_menu();

	for (;;) {
	    switch (option) {
	    case 0:
		TTF_SizeText(media.font_large, lang.menusandbox[option], &w, &h);
		rect.x = left_absolute - 20;
		rect.y = top_sandbox + 60 + 80 * option - ctx.offset;
		rect.w = 200;
		rect.h = h - 10;
		break;

	    default:
		TTF_SizeText(media.font_normal, ctx.sandbox_list[option - 1], &w,
			     &h);
		rect.x = left_absolute - 20;
		rect.y = top_sandbox + 138 + 24 * (option - 1) - ctx.offset;
		rect.w = 200;
		rect.h = 12;
		break;
	    }
	    if (rect.y - rect.h < 0) {
		ctx.offset -= 24;
	    } else if (rect.y + rect.h > media.canvas->h) {
		ctx.offset += 24;
	    } else
		break;
	}
	render_spark(media.canvas, &rect);

	while (SDL_PollEvent(&event) != 0) {
	    switch (event.type) {
	    case SDL_KEYDOWN:
		switch (event.key.keysym.sym) {
		case SDLK_UP:
		    if (option > 0) {
			--option;
		    }
		    break;
		case SDLK_DOWN:
		    if (option < ctx.sandbox_count) {
			++option;
		    }
		    break;

		case SDLK_PAGEUP:
		    option -= 3;
		    if (option < 0) {
			option = 0;
		    }
		    break;
		case SDLK_PAGEDOWN:
		    option += 3;
		    if (option > ctx.sandbox_count) {
			option = ctx.sandbox_count;
		    }
		    break;

		case SDLK_RETURN:
		    switch (option) {
		    case 0:
			path = "";
			edit(path);
			break;

		    default:
			path =
			    strjoin(PATH_SEPARATOR, dir,
				    ctx.sandbox_list[option - 1]);
			edit(path);
			free(path);
			break;
		    }

		    ctx.sandbox_count = list_folder(dir, &ctx.sandbox_list);
		    load_background();
		    render_main_menu();
		    render_sandbox_menu();
		    break;

		case SDLK_ESCAPE:
		    keep_going = 0;
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

	media_sync();
    }

    free(dir);
    shift_menu(top_main);
    return 0;
}



/*
 * Low level methods.
 */
void
shift_menu(int new_offset)
{
    int k;

    for (k = ctx.offset - new_offset; k != 0; k /= 2) {
	ctx.offset = new_offset + k;
	render_background();
	blit_menu();
	media_sync();
    }
}

void
blit_menu()
{
    SDL_Rect rect;
    SDL_Surface *tmp;

    SDL_BlitSurface(media.surface_title, NULL, media.canvas, NULL);

    rect.x = 150;
    rect.y = -ctx.offset;
    SDL_BlitSurface(ctx.surface_menu, NULL, media.canvas, &rect);

    if ((tmp = TTF_RenderUTF8_Blended(media.font_input, user.name, black))) {
	rect.x = 300 - tmp->w / 2;
	rect.y = 120 - ctx.offset;
	SDL_BlitSurface(tmp, NULL, media.canvas, &rect);
	SDL_FreeSurface(tmp);
    }
}

void
play_from(int i)
{
    char *path;
    int ret;

    for (; i < ctx.level_count; ++i) {
	if (i > user.max_level) {
	    user.max_level = i;
	    save_profile();
	}

	if (i % 10 == 0) {
	    run_transition(i / 10);
	}

	path =
	    strjoin(PATH_SEPARATOR, DATA_DIR, "levels", ctx.level_list[i].file,
		    NULL);
	play_music(ctx.level_list[i].track);
	ret = play(path, lang.levelnames[i]);
	free(path);

	if (ret == 0) {
	    break;
	}
    }

    play_music(0);
}

void
load_background()
{
    SDL_Surface *tmp = load_texture("menu.png");
    ctx.surface_menu = SDL_ConvertSurface(tmp, media.canvas->format, 0);
    SDL_SetColorKey(ctx.surface_menu, SDL_SRCCOLORKEY,
		    *(Uint16 *) ctx.surface_menu->pixels);
    SDL_FreeSurface(tmp);
}

void
run_transition(int i)
{
    SDL_Surface *tmp;
    SDL_Rect rect;
    SDL_Event event;
    int k;

    SDL_FillRect(media.canvas, NULL, 0);

    if ((tmp = TTF_RenderUTF8_Blended(media.font_large, lang.actnames[i], white))) {
	rect.x = (media.canvas->w - tmp->w) / 2;
	rect.y = (media.canvas->h - tmp->h) / 2;
	SDL_BlitSurface(tmp, NULL, media.canvas, &rect);
	SDL_FreeSurface(tmp);
    }

    for (k = 0; k < 200; ++k) {
	if (SDL_PollEvent(&event)) {
	    switch (event.type) {
	    case SDL_KEYDOWN:
		return;
	    case SDL_QUIT:
		exit(0);
	    }
	}
	media_sync();
    }
}
