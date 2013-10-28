/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013
 */

#pragma once

#include <SDL/SDL.h>		// For SDL
#include <SDL/SDL_image.h>	// For IMG_Load
#include <SDL/SDL_mixer.h>	// For Sound
#include <SDL/SDL_framerate.h>	// For FPSmanager
#include <SDL/SDL_gfxPrimitives.h>	// For lineColor
#include <SDL/SDL_ttf.h>	// For text

#include "../core/core.h"

/*----------------------------------------------------------------------------
 * All data needed for media
 *----------------------------------------------------------------------------*/
struct media {
    int enable_audio;
    int enable_music;

    SDL_Surface *canvas;	// The window.
    SDL_Surface *surface_frame;	// The frame.
    SDL_Surface *surface_title;	// The menu.
    SDL_Surface *surface_hforhelp;	// Press H for help.
    SDL_Surface *surface_levelname;

    SDL_Surface *surface_background;
    SDL_Surface *surface_objects;
    SDL_Surface *surface_effects;
    SDL_Surface *surface_manual;
    SDL_Surface *surface_properties;
    SDL_Surface *surface_circle;

    TTF_Font *font_small;	// Small sized font.
    TTF_Font *font_normal;	// Normal sized font.
    TTF_Font *font_large;	// Large sized font.
    TTF_Font *font_input;	// Font for input boxes.
    TTF_Font *font_equation;	// Font for equation.
    TTF_Font *font_button;	// Font for equation.

    Mix_Chunk *chunk_press;	// The waveform of a button being pressed.
    Mix_Chunk *chunk_release;	// The waveform of a button being released.
    Mix_Chunk *chunk_open;	// The waveform of a gate being openend.
    Mix_Chunk *chunk_fail;	// The waveform of a failure.
    Mix_Chunk *chunk_wodox;	// The waveform of a wodox moving.
    Mix_Chunk *chunk_keystroke;	// The waveform of a keystroke.
    Mix_Chunk *chunk_marker;	// The waveform of a marker.

    FPSmanager fpsmanager;	// To ensure a steady frame rate.

    int current_frame;		// Controls The 'wave' effect applied to some surfaces
};

extern struct media media;

/*----------------------------------------------------------------------------
 * Some constants
 *----------------------------------------------------------------------------*/
extern SDL_Color color_white;	// Some used and reused values.
extern SDL_Color color_black;	// Some used and reused values.
extern SDL_Color color_red;
extern SDL_Color color_marker;

/*----------------------------------------------------------------------------
 * Setup and setdown
 *----------------------------------------------------------------------------*/
int media_init(void);
void media_end(void);

/*----------------------------------------------------------------------------
 * Call at the end of every iteration to draw to screen and handle framerate.
 *----------------------------------------------------------------------------*/
void media_sync(void);

/*----------------------------------------------------------------------------
 * Pause the application
 *----------------------------------------------------------------------------*/
void media_freeze(void);

/*----------------------------------------------------------------------------
 * Pause while showing a help text.
 *----------------------------------------------------------------------------*/
int help(char **text, char **opts, char *shortcuts);

/*----------------------------------------------------------------------------
 * Edit level properties
 *----------------------------------------------------------------------------*/
int properties(char *path);

/*----------------------------------------------------------------------------
 * Specific rendering and post-rendering routines
 *----------------------------------------------------------------------------*/
void render_background(void);
void render_foreground(void);
void render_object(Uint32 sprite, Uint32 frame, SDL_Rect * dst);
void render_effect(Uint32 sprite, Uint32 frame, SDL_Rect * dst);
void render_text(TTF_Font * font, const char *text, Sint16 x, Sint16 y);

void warp_surface(SDL_Surface * s, SDL_Rect * rect);
void render_spark(SDL_Surface * s, SDL_Rect * rect);
void render_particles(SDL_Surface *, SDL_Rect * dst);

void sepia_surface(SDL_Surface * s);

/*----------------------------------------------------------------------------
 * Play music.
 *----------------------------------------------------------------------------*/
void play_music(int track);

/*----------------------------------------------------------------------------
 * Load resources.
 *----------------------------------------------------------------------------*/
SDL_Surface *load_texture(const char *filename);
TTF_Font *load_font(const char *filename, int size);
Mix_Chunk *load_sample(const char *filename);
