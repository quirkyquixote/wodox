/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013
 */

#include "../media/media.h"
#include "types.h"
#include "parse.h"

/*
 * Draw objects.
 */
void
render_level(void)
{
    size_t i, j, k;
    SDL_Rect dst;
    SDL_Surface *surface;
    char lil_buf[4];

    // Draw all objects. Most objects have only a simple sprite with no 
    // alpha channel, but levitators have alpha transparency and a "warp"
    // effect, and the exit is a particle system.

    for (i = 0; i < SIZE; ++i) {
	for (k = 0; k < SIZE; ++k) {
	    for (j = 0; j < SIZE; ++j) {
		if (level.circuit_map[j][i][k].tree) {
		    dst.x = SCREENX(SPS * i, SPS * j, SPS * k);
		    dst.y = SCREENY(SPS * i, SPS * j, SPS * k);
		    render_effect(1, 1, &dst);
		}
		switch (level.static_map[j][i][k]) {
		case GROUND ... SWITCH:
		    dst.x = SCREENX(SPS * i, SPS * j, SPS * k);
		    dst.y = SCREENY(SPS * i, SPS * j, SPS * k);
		    render_object(level.static_map[j][i][k] - 1, 0, &dst);
		    break;

		case TUBE:
		    dst.x = SCREENX(SPS * i, SPS * j, SPS * k);
		    dst.y = SCREENY(SPS * i, SPS * j, SPS * k);
		    if (level.static_map[j + 1][i][k] != TUBE) {
			render_effect(1, 0, &dst);
		    }
		    render_effect(0, 0, &dst);
		    break;

		case WARP:
		    dst.x = SCREENX(SPS * i, SPS * j, SPS * k);
		    dst.y = SCREENY(SPS * i, SPS * j, SPS * k);
		    render_particles(media.canvas, &dst);
		    break;

		default:
		    if (j == Y(level.cursor)) {
			dst.x = SCREENX(SPS * i, SPS * j, SPS * k);
			dst.y = SCREENY(SPS * i, SPS * j, SPS * k);
			render_effect(3, 1, &dst);
		    }
		}
		if (level.cursor == OFF(i, j, k)) {
		    dst.x = SCREENX(SPS * i, SPS * j, SPS * k);
		    dst.y = SCREENY(SPS * i, SPS * j, SPS * k);
		    render_effect(0, 1, &dst);
		}
	    }
	    for (j = 0; j < SIZE; ++j) {
		if (level.static_map[j][i][k] == TUBE) {
		    dst.x = SCREENX(SPS * i, SPS * j, SPS * k);
		    dst.y = SCREENY(SPS * i, SPS * j, SPS * k);
		    warp_surface(media.canvas, &dst);
		}
	    }
	    for (j = 0; j < SIZE; ++j) {
		if (level.circuit_map[j][i][k].tree) {
		    dst.x = SCREENX(SPS * i, SPS * j, SPS * k);
		    dst.y = SCREENY(SPS * i, SPS * j, SPS * k);
		    render_effect(2, 1, &dst);
		}
	    }
	}
    }

    // Label all buttons and switches.

    for (i = 0; i < SIZE; ++i) {
	for (j = 0; j < SIZE; ++j) {
	    for (k = 0; k < SIZE; ++k) {
		switch (level.static_map[i][j][k]) {
		case BUTTON:
		case SWITCH:
		    sprintf(lil_buf, "%d%d%d", i, j, k);
		    if ((surface =
			 TTF_RenderUTF8_Blended(media.font_normal, lil_buf,
						color_white))) {
			dst.x =
			    SCREENX(SPS * j, SPS * i,
				    SPS * k) - surface->w / 2 + 30;
			dst.y =
			    SCREENY(SPS * j, SPS * i,
				    SPS * k) - surface->h / 2 + 10;
			SDL_BlitSurface(surface, NULL, media.canvas, &dst);
			SDL_FreeSurface(surface);
		    }

		default:
		    break;
		}
	    }
	}
    }

    // Draw the cursor
    /*
       dst.x = SCREENX (SPS * X (level.cursor), SPS * Y (level.cursor), SPS * Z (level.cursor));
       dst.y = SCREENY (SPS * X (level.cursor), SPS * Y (level.cursor), SPS * Z (level.cursor));
       render_object (0, 1, &dst);
     */
}

void
render_circuit(void)
{
    char buf[256];
    SDL_Surface *surface;
    SDL_Rect dst;

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
		 TTF_RenderUTF8_Blended(media.font_equation, buf,
					color_white))) {
		dst.x = (media.canvas->w - surface->w) / 2;
		dst.y = 0;
		SDL_BlitSurface(surface, NULL, media.canvas, &dst);
		SDL_FreeSurface(surface);
	    }
	}
}
