/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013
 */

#include <errno.h>

#include "media/media.h"
#include "play/play.h"

#include "edit_private.h"

struct context {
    uint16_t idx;
    char buf[128];
    char *buf_ptr;
    int keep_going;
    int blink;
    SDL_Surface *bkgr;
};

static struct context ctx;

static void handle_event(SDL_Event * event);
static void capture_background(void);
static void render(void);

/*
 * Edit circuit.
 */
int
editor_edit_circuit(uint16_t idx)
{
    // Only edit if what we want to link is a machine.

    switch (S_MAP[idx]) {
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

    SDL_Event event;

    ctx.idx = idx;
    ctx.keep_going = 1;
    ctx.blink = 0;

    SDL_EnableUNICODE(1);

    memset(ctx.buf, 0, sizeof(ctx.buf));
    ctx.buf_ptr =
        ctx.buf + circuit_to_text(ctx.buf, &C_MAP[ctx.idx], 0, 0);

    capture_background();

    while (ctx.keep_going) {
        render();

        while (SDL_PollEvent(&event) != 0)
            handle_event(&event);

        render_foreground();
        media_sync();
    }

    SDL_FreeSurface(ctx.bkgr);

    SDL_EnableUNICODE(0);

    return 0;
}

void
handle_event(SDL_Event * event)
{
    int page;

    switch (event->type) {
    case SDL_KEYDOWN:
        switch (event->key.keysym.sym) {
        case SDLK_ESCAPE:
            ctx.keep_going = 0;
            break;

        case SDLK_BACKSPACE:
            if (ctx.buf_ptr != ctx.buf) {
                --ctx.buf_ptr;
                *ctx.buf_ptr = 0;
            } else {
                if (C_MAP[ctx.idx].tree) {
                    free(C_MAP[ctx.idx].tree);
                    C_MAP[ctx.idx].tree = NULL;
                    C_MAP[ctx.idx].size = 0;
                }
                ctx.keep_going = 0;
            }

            Mix_PlayChannel(2, media.chunk_keystroke, 0);
            break;

        case SDLK_RETURN:
            text_to_circuit(ctx.buf, &C_MAP[ctx.idx], 0);
            ctx.keep_going = 0;
            Mix_PlayChannel(2, media.chunk_keystroke, 0);
            break;

        case SDLK_F1:
            page = 1;

            while (page)
                switch (page) {
                case 1:
                    page = help(lang.howtolink, lang.menulink, "\01234");
                    break;
                case 2:
                    page = help(lang.howtolink2, lang.menulink, "\01234");
                    break;
                case 3:
                    page = help(lang.howtolink3, lang.menulink, "\01234");
                    break;
                case 4:
                    page = help(lang.howtolink4, lang.menulink, "\01234");
                    break;
                }
            break;

        default:
            if (isprint(event->key.keysym.unicode) &&
                ctx.buf_ptr != ctx.buf + sizeof(ctx.buf)) {
                *ctx.buf_ptr = event->key.keysym.unicode;
                ++ctx.buf_ptr;
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

void
capture_background(void)
{
    render_background();
    editor_render_level();
    sepia_surface(media.canvas);
    ctx.bkgr =
        SDL_ConvertSurface(media.canvas, media.canvas->format,
                           media.canvas->flags);
}

void
render(void)
{
    SDL_Surface *surface;
    SDL_Rect dst;

    SDL_BlitSurface(ctx.bkgr, NULL, media.canvas, NULL);

    if ((surface =
         TTF_RenderUTF8_Blended(media.font_equation, ctx.buf,
                                color_white))) {
        dst.x = (media.canvas->w - surface->w) / 2;
        dst.y = 0;
        SDL_BlitSurface(surface, NULL, media.canvas, &dst);

        if ((++ctx.blink / 4) % 2) {
            dst.x += surface->w;
            dst.w = 20;
            dst.h = surface->h - 5;
            SDL_FillRect(media.canvas, &dst, 0xffffffff);
        }

        SDL_FreeSurface(surface);
    } else if ((++ctx.blink / 4) % 2) {
        dst.x = media.canvas->w / 2;
        dst.y = 0;
        dst.w = 20;
        dst.h = 20;
        SDL_FillRect(media.canvas, &dst, 0xffffffff);
    }
}
