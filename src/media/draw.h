/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2012
 */

#pragma once

#include <SDL/SDL.h>		// For SDL
#include <SDL/SDL_image.h>		// For IMG_Load
#include <SDL/SDL_mixer.h>		// For Sound
#include <SDL/SDL_framerate.h>	// For FPSmanager
#include <SDL/SDL_gfxPrimitives.h>  // For lineColor
#include <SDL/SDL_ttf.h>		// For text

#include "../core/core.h"

/*
 * Configuration flags.
 */

extern int enable_audio;
extern int enable_music;

/* 
 * SDL stuff.
 */
extern SDL_Surface * canvas;		// The window.
extern SDL_Surface * surface_frame;	// The frame.
extern SDL_Surface * surface_title;	// The menu.
extern SDL_Surface * surface_hforhelp;	// Press H for help.
extern SDL_Surface * surface_levelname;

extern TTF_Font * font_small;		// Small sized font.
extern TTF_Font * font_normal;		// Normal sized font.
extern TTF_Font * font_large;		// Large sized font.
extern TTF_Font * font_input;		// Font for input boxes.
extern TTF_Font * font_equation;	// Font for equation.

extern Mix_Chunk * chunk_press;		// The waveform of a button being pressed.
extern Mix_Chunk * chunk_release;	// The waveform of a button being released.
extern Mix_Chunk * chunk_open;		// The waveform of a gate being openend.
extern Mix_Chunk * chunk_fail;   	// The waveform of a failure.
extern Mix_Chunk * chunk_wodox;		// The waveform of a wodox moving.
extern Mix_Chunk * chunk_keystroke;	// The waveform of a keystroke.
extern Mix_Chunk * chunk_marker;	// The waveform of a marker.

extern FPSmanager fpsmanager;		// To ensure a steady frame rate.

extern SDL_Color color_white;		// Some used and reused values.
extern SDL_Color color_black;		// Some used and reused values.

/*
 * Initialize and terminate SDL.
 */
int init_graphics ();
void end_graphics ();

/*
 * Call at the end of every iteration to draw to screen and handle framerate.
 */ 
void sync ();

/*
 * Pause the application
 */
void freeze ();

/*
 * Pause while showing a help text.
 */
int help (char ** text, char ** opts, char * shortcuts);

/*
 * Edit level properties
 */
int properties (char * path);

/*
 * To draw things.
 */
void draw_background ();
void render_foreground(void);
void draw_object (Uint32 sprite, Uint32 frame, SDL_Rect * dst);
void draw_effect (Uint32 sprite, Uint32 frame, SDL_Rect * dst);
void draw_text (TTF_Font * font, const char * text, Sint16 x, Sint16 y);

void warp_surface (SDL_Surface * s, SDL_Rect * rect);
void draw_spark (SDL_Surface * s, SDL_Rect * rect);
void draw_particles (SDL_Surface *, SDL_Rect * dst);

void sepia_surface (SDL_Surface * s);

/*
 * Play music.
 */
void play_music (int track);

/*
 * Load resources.
 */
SDL_Surface * load_texture (const char *filename);
TTF_Font * load_font (const char *filename, int size);
Mix_Chunk * load_sample (const char *filename);



