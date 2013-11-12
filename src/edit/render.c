/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013
 */

#include "media/media.h"
#include "edit_private.h"

/*
 * Draw objects.
 */
void
editor_render_level(void)
{
    struct coord c;
    SDL_Rect dst;
    SDL_Surface *surface;
    char lil_buf[4];

    // Draw all objects. Most objects have only a simple sprite with no 
    // alpha channel, but levitators have alpha transparency and a "warp"
    // effect, and the exit is a particle system.

    for (c.x = 0; c.x < SIZE; ++c.x) {
	for (c.z = 0; c.z < SIZE; ++c.z) {
	    for (c.y = 0; c.y < SIZE; ++c.y) {
		if (level.circuit_map[c.y][c.x][c.z].tree) {
		    dst = world_to_screen(SPS * c.x, SPS * c.y, SPS * c.z);
		    render_effect(1, 1, &dst);
		}

		switch (level.static_map[c.y][c.x][c.z]) {
		case GROUND ... SWITCH:
		    dst = world_to_screen(SPS * c.x, SPS * c.y, SPS * c.z);
		    render_object(level.static_map[c.y][c.x][c.z] - 1, 0,
				  &dst);
		    break;

		case TUBE:
		    dst = world_to_screen(SPS * c.x, SPS * c.y, SPS * c.z);
		    if (level.static_map[c.y + 1][c.x][c.z] != TUBE) {
			render_effect(1, 0, &dst);
		    }
		    render_effect(0, 0, &dst);
		    break;

		case WARP:
		    dst = world_to_screen(SPS * c.x, SPS * (c.y - 2), SPS * c.z);
		    render_particles(media.canvas, &dst);
		    break;

		default:
		    if (c.y == level.cursor.y) {
			dst = world_to_screen(SPS * c.x, SPS * c.y,
					    SPS * c.z);
			render_effect(3, 1, &dst);
		    }
		}

		if (coordcmp(level.cursor, c) == 0) {
		    dst = world_to_screen(SPS * c.x, SPS * c.y, SPS * c.z);
		    render_effect(0, 1, &dst);
		}
	    }

	    for (c.y = 0; c.y < SIZE; ++c.y) {
		if (level.static_map[c.y][c.x][c.z] == TUBE) {
		    dst = world_to_screen(SPS * c.x, SPS * c.y, SPS * c.z);
		    warp_surface(media.canvas, &dst);
		}
	    }

	    for (c.y = 0; c.y < SIZE; ++c.y) {
		if (level.circuit_map[c.y][c.x][c.z].tree) {
		    dst = world_to_screen(SPS * c.x, SPS * c.y, SPS * c.z);
		    render_effect(2, 1, &dst);
		}
	    }
	}
    }

    // Label all buttons and switches.

    for (c.x = 0; c.x < SIZE; ++c.x) {
	for (c.y = 0; c.y < SIZE; ++c.y) {
	    for (c.z = 0; c.z < SIZE; ++c.z) {
		switch (level.static_map[c.x][c.y][c.z]) {
		case BUTTON:
		case SWITCH:
		    sprintf(lil_buf, "%d%d%d", c.x, c.y, c.z);
		    if ((surface =
			 TTF_RenderUTF8_Blended(media.font_normal, lil_buf,
						color_white))) {
			dst = world_to_screen(SPS * c.x, SPS * c.y,
					    SPS * c.z);
			dst.x += 30 - surface->w / 2;
			dst.y += 10 - surface->h / 2;
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
editor_render_circuit(void)
{
    char buf[256];
    SDL_Surface *surface;
    SDL_Rect dst;

    switch (S_MAP[coord_to_idx(level.cursor)]) {
    case TUBE:
    case BELTLF:
    case BELTRT:
    case BELTBK:
    case BELTFT:
    case MOVING:
	memset(buf, 0, sizeof(buf));
	circuit_to_text(buf, &C_MAP[coord_to_idx(level.cursor)], 0, 0);

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
