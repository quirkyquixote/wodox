/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009 
 */

#include "draw.h"

/*
 * Configuration flags.
 */
int enable_audio = 1;
int enable_music = 1;

/*
 * SDL stuff allocated here.
 */
SDL_Surface *	canvas		= NULL;
SDL_Surface *	surface_frame	= NULL;
SDL_Surface *	surface_title	= NULL;
SDL_Surface * 	surface_hforhelp= NULL;
SDL_Surface * 	surface_circle	= NULL;

TTF_Font *	font_small	= NULL;
TTF_Font *	font_normal	= NULL;
TTF_Font *	font_large	= NULL;
TTF_Font *	font_input	= NULL;
TTF_Font *	font_equation	= NULL;
TTF_Font *	font_button	= NULL;

Mix_Chunk *	chunk_press	= NULL;
Mix_Chunk *	chunk_release	= NULL;
Mix_Chunk *	chunk_open	= NULL;
Mix_Chunk *	chunk_fail	= NULL;
Mix_Chunk *	chunk_wodox	= NULL;	
Mix_Chunk *	chunk_keystroke	= NULL;
Mix_Chunk *	chunk_marker	= NULL;

FPSmanager	fpsmanager;

SDL_Color 	color_white	= { 255, 255, 255 };
SDL_Color	color_black	= { 0, 0, 0 };
SDL_Color	color_red	= { 255, 0, 0 };
SDL_Color	color_marker	= { 40, 40, 24 };

/*
 * Bitmaps.
 */
static SDL_Surface * surface_background;
static SDL_Surface * surface_objects;	
static SDL_Surface * surface_effects;
static SDL_Surface * surface_manual;
static SDL_Surface * surface_properties;

/*
 * Source rects for all objects and effects.
 */
static SDL_Rect rects_55_63[][4] = {
  { {   0, 0, 55, 63 }, {   0, 63, 55, 63 }, {   0, 126, 55, 63 }, {   0, 189, 55, 63 } }, 
  { {  55, 0, 55, 63 }, {  55, 63, 55, 63 }, {  55, 126, 55, 63 }, {  55, 189, 55, 63 } }, 
  { { 110, 0, 55, 63 }, { 110, 63, 55, 63 }, { 110, 126, 55, 63 }, { 110, 189, 55, 63 } }, 
  { { 165, 0, 55, 63 }, { 165, 63, 55, 63 }, { 165, 126, 55, 63 }, { 165, 189, 55, 63 } }, 
  { { 220, 0, 55, 63 }, { 220, 63, 55, 63 }, { 220, 126, 55, 63 }, { 220, 189, 55, 63 } },
  { { 275, 0, 55, 63 }, { 275, 63, 55, 63 }, { 275, 126, 55, 63 }, { 275, 189, 55, 63 } },
  { { 330, 0, 55, 63 }, { 330, 63, 55, 63 }, { 330, 126, 55, 63 }, { 330, 189, 55, 63 } },
  { { 385, 0, 55, 63 }, { 385, 63, 55, 63 }, { 385, 126, 55, 63 }, { 385, 189, 55, 63 } },
  { { 440, 0, 55, 63 }, { 440, 63, 55, 63 }, { 440, 126, 55, 63 }, { 440, 189, 55, 63 } },
  { { 495, 0, 55, 63 }, { 495, 63, 55, 63 }, { 495, 126, 55, 63 }, { 495, 189, 55, 63 } },
  { { 550, 0, 55, 63 }, { 550, 63, 55, 63 }, { 550, 126, 55, 63 }, { 550, 189, 55, 63 } },
  { { 605, 0, 55, 63 }, { 605, 63, 55, 63 }, { 605, 126, 55, 63 }, { 605, 189, 55, 63 } },
  { { 660, 0, 55, 63 }, { 660, 63, 55, 63 }, { 660, 126, 55, 63 }, { 660, 189, 55, 63 } },
  { { 715, 0, 55, 63 }, { 715, 63, 55, 63 }, { 715, 126, 55, 63 }, { 715, 189, 55, 63 } },
  { { 770, 0, 55, 63 }, { 770, 63, 55, 63 }, { 770, 126, 55, 63 }, { 770, 189, 55, 63 } },
  { { 825, 0, 55, 63 }, { 825, 63, 55, 63 }, { 825, 126, 55, 63 }, { 825, 189, 55, 63 } },
  { { 880, 0, 55, 63 }, { 880, 63, 55, 63 }, { 880, 126, 55, 63 }, { 880, 189, 55, 63 } },
};

/*
 * Source rects for all particles.
 */
static SDL_Rect rects_particles[] = {
  { 165,  0, 32, 32 }, // particle (big, white)
  { 165, 32, 16, 16 }, // particle (medium, blue)
  { 165, 48,  8,  8 }, // particle (small, red)
};

/*
 * The 'wave' effect applied to some surfaces is controlled by this variable.
 */
static int current_frame = 0;

/*
 * Initialize SDL.
 */
int
init_graphics ()
{
  // Initialize SDL itself. Exit on failure.
 
  if (SDL_Init (SDL_INIT_VIDEO))
    {
      fprintf (stderr, "%s\n", SDL_GetError ());
      exit (0);
    }

  atexit (end_graphics);

  // Initialize window and load all graphics. Exit on failure (we need all 
  // graphics to play).

  if ((canvas = SDL_SetVideoMode (800, 600, 16, SDL_DOUBLEBUF)) == NULL)
    {
      fprintf (stderr, "%s\n", SDL_GetError ());
      exit (0);
    }

  surface_title = load_texture ("title.png");
  surface_frame = load_texture ("frame.png");
  surface_objects = load_texture ("objects.png");
  surface_effects = load_texture ("effects.png");
  surface_manual = load_texture ("manual.png");
  surface_properties = load_texture ("properties.png");
  surface_circle = load_texture ("circle.png");

  if ((surface_background = SDL_CreateRGBSurface (SDL_SWSURFACE, canvas->w,
  canvas->h + 4, canvas->format->BitsPerPixel, canvas->format->Rmask,
  canvas->format->Gmask, canvas->format->Bmask, canvas->format->Amask)) ==
  NULL)
    {
      fprintf (stderr, "%s\n", SDL_GetError ());
      exit (0);
    }

  SDL_Surface * tmp;
  SDL_Rect dst;

  tmp = load_texture ("background.png");

  for (dst.x = 0; dst.x < surface_background->w; dst.x += tmp->w)
    for (dst.y = 0; dst.y < surface_background->h; dst.y += tmp->h)
      {
        SDL_BlitSurface (tmp, NULL, surface_background, &dst);
      }

  SDL_FreeSurface (tmp);

  // Initialize and load true type fonts. Exit on failure.

  if (TTF_Init ()) 
    {
      fprintf (stderr, "%s\n", TTF_GetError());
      exit (0);
    }
/*
  font_small = load_font ("acoustic_bass.ttf", 12);
  font_normal = load_font ("acoustic_bass.ttf", 16);
  font_large = load_font ("acoustic_bass.ttf", 24);
  font_input = load_font ("acoustic_bass.ttf", 20);
  font_equation = load_font ("acoustic_bass.ttf", 20);
*/
  font_small = load_font ("underwood_champion.ttf", 16);
  font_normal = load_font ("underwood_champion.ttf", 18);
  font_large = load_font ("underwood_champion.ttf", 30);
  font_input = load_font ("underwood_champion.ttf", 26);
  font_equation = load_font ("underwood_champion.ttf", 20);
  font_button = load_font ("j_d_handcrafted.ttf", 22);

  surface_hforhelp = TTF_RenderUTF8_Blended (font_normal, str_hforhelp, color_white);

  // Initialize audio if wanted and load all samples. Disable audio on failure
  // (audio is not indispensable)

  if (enable_audio)
    {
      if (SDL_InitSubSystem (SDL_INIT_AUDIO))
        {
	  fprintf (stderr, "%s\n", SDL_GetError ());
	  enable_audio = 0;
	}
      else if (Mix_OpenAudio (22050, AUDIO_S16SYS, 2, 1024)) 
        {
          fprintf (stderr, "%s\n", Mix_GetError ());
	  enable_audio = 0;
        }
      else
        {
          chunk_press = load_sample ("press.wav");
          chunk_release = load_sample ("release.wav");
          chunk_open = load_sample ("open.wav");
          chunk_fail = load_sample ("fail.wav");
          chunk_wodox = load_sample ("wodox.wav");
          chunk_keystroke = load_sample ("typewriter.wav");
          chunk_marker = load_sample ("marker01.wav");
	}
    }

  // Give the window a name.

  SDL_WM_SetCaption ("Wodox", "wodox");

  // Initialize frame rate control.

  SDL_initFramerate (&fpsmanager);
  SDL_setFramerate (&fpsmanager, 30);

  // Most of the time keyrepeat is set to true.

  SDL_EnableKeyRepeat (SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  return 0;
}

/*
 * Terminate
 */
void
end_graphics ()
{
  if (enable_audio)
    {
      Mix_CloseAudio ();
    }

  SDL_Quit ();
}

/*
 * Call at the end of each iteration.
 */
void
sync ()
{
  SDL_framerateDelay (&fpsmanager);
  SDL_Flip (canvas);
}

/*
 * Pause
 */
void 
freeze ()
{
  SDL_Event event;

  SDL_BlitSurface (surface_frame, NULL, canvas, NULL);
  sepia_surface (canvas);

  for (;;)
    {
      if (SDL_PollEvent (&event))
	{
	  switch (event.type)
	    {
	    case SDL_KEYDOWN: return;
	    case SDL_QUIT: exit (0);
	    }
	} 

      sync ();
    }

  return;
}

/*
 * Display help.
 */
int
help (char ** text, char ** opts, char * shortcuts)
{
  int i;
  int ret = 0;
  int old_ret = 0;
  int keep_going = 1;

  SDL_Event event;

  SDL_Surface * bak;
  SDL_Surface * tmp;

  SDL_Rect dst;

  bak = SDL_ConvertSurface (canvas, canvas->format, canvas->flags);

  SDL_BlitSurface (surface_frame, NULL, canvas, NULL);
  sepia_surface (canvas);
  SDL_BlitSurface (surface_manual, NULL, canvas, NULL);

  if ((tmp = TTF_RenderUTF8_Blended (font_small, str_usermanual, color_red)))
    {
      dst.x = 740 - tmp->w;
      dst.y = 60;
      SDL_BlitSurface (tmp, NULL, canvas, &dst);
      SDL_FreeSurface (tmp);
    }

  for (i = 0; text[i]; ++i)
    {
      if ((tmp = TTF_RenderUTF8_Blended (font_normal, text[i], color_black)))
        {
	  dst.x = 100;
	  dst.y = 100 + 20 * i;
	  SDL_BlitSurface (tmp, NULL, canvas, &dst);
	  SDL_FreeSurface (tmp);
	}
    }

  for (i = 0; i < 5; ++i)
    {
      if (opts[i] && (tmp = TTF_RenderUTF8_Blended (font_button, opts[i], color_marker)))
        {
	  dst.x = 135 + 135 * i - tmp->w / 2;
	  dst.y = 530;
	  SDL_BlitSurface (tmp, NULL, canvas, &dst);
	  SDL_FreeSurface (tmp);
	}
    }

  tmp = SDL_ConvertSurface (canvas, canvas->format, canvas->flags);
 
  while (keep_going)
    {
      while (SDL_PollEvent (&event))
	{
	  switch (event.type)
	    {
	    case SDL_MOUSEMOTION:
              if (abs (event.motion.y - 540) < 50)
	        {
		  ret = 
		    opts[0] && abs (event.motion.x - 135) < 50 ? 0 : 
		    opts[1] && abs (event.motion.x - 270) < 50 ? 1 : 
		    opts[2] && abs (event.motion.x - 405) < 50 ? 2 :
		    opts[3] && abs (event.motion.x - 540) < 50 ? 3 :
		    opts[4] && abs (event.motion.x - 675) < 50 ? 4 :
		    ret;
		}
	      break;

	    case SDL_MOUSEBUTTONDOWN:
              if (abs (event.motion.y - 540) < 50)
	        {
		  i =
		    opts[0] && abs (event.motion.x - 135) < 50 ? 0 :
		    opts[1] && abs (event.motion.x - 270) < 50 ? 1 : 
		    opts[2] && abs (event.motion.x - 405) < 50 ? 2 : 
		    opts[3] && abs (event.motion.x - 540) < 50 ? 3 : 
		    opts[4] && abs (event.motion.x - 675) < 50 ? 4 : 
		    -1;

		  if (i >= 0)
		    {
		      keep_going = 0;
		      ret = i;
		    }
		}
	      break;

	    case SDL_KEYDOWN: 
	      switch (event.key.keysym.sym)
	        {
		case SDLK_LEFT:		if (--ret < 0) { ret = 4; } break;
		case SDLK_RIGHT:	if (++ret > 4) { ret = 0; } break;
		case SDLK_RETURN:	keep_going = 0; break;
		case SDLK_ESCAPE:	keep_going = 0; ret = 0; break;
		default: 		
		  if (event.key.keysym.sym == shortcuts[0]) { keep_going = 0; ret = 0; break; }
		  if (event.key.keysym.sym == shortcuts[1]) { keep_going = 0; ret = 1; break; }
		  if (event.key.keysym.sym == shortcuts[2]) { keep_going = 0; ret = 2; break; }
		  if (event.key.keysym.sym == shortcuts[3]) { keep_going = 0; ret = 3; break; }
		  if (event.key.keysym.sym == shortcuts[4]) { keep_going = 0; ret = 4; break; }
		  break;
		}
	      break;

	    case SDL_QUIT: exit (0);
	    }
	}

      SDL_BlitSurface (tmp, NULL, canvas, NULL); 
      dst.x = 135 + 135 * ret - surface_circle->w / 2;
      dst.y = 540 - surface_circle->h / 2;
      SDL_BlitSurface (surface_circle, NULL, canvas, &dst);

      if (old_ret != ret)
        {
          Mix_PlayChannel (2, chunk_marker, 0);
          old_ret = ret;
        }

      sync ();
    }

  SDL_BlitSurface (bak, NULL, canvas, NULL);
  SDL_FreeSurface (tmp);
  SDL_FreeSurface (bak);

  return ret;
}

#define PATH_LEN 1024

/*
 * Edit level properties.
 */
int
properties (char * path)
{
  int i;
  int ret = 1;
  int keep_going = 1;
  char * path_end = path + strlen (path);

  SDL_Surface *	tmp;
  SDL_Rect rect;
  SDL_Event event;

  SDL_EnableUNICODE (1);

  SDL_BlitSurface (surface_frame, NULL, canvas, NULL);
  sepia_surface (canvas);

  while (keep_going)
  {
    SDL_BlitSurface (surface_properties, NULL, canvas, NULL);

    if ((tmp = TTF_RenderUTF8_Blended (font_input, path, color_black)))
      {
        rect.x = 195;
        rect.y = 315;
        SDL_BlitSurface (tmp, NULL, canvas, &rect);
        SDL_FreeSurface (tmp);
      }

    if ((tmp = TTF_RenderUTF8_Blended (font_button, str_menusave[0], color_marker)))
      {
        rect.x = 220 - tmp->w / 2;
        rect.y = 540 - tmp->h / 2;
        SDL_BlitSurface (tmp, NULL, canvas, &rect);
        SDL_FreeSurface (tmp);
      }

    if ((tmp = TTF_RenderUTF8_Blended (font_button, str_menusave[1], color_marker)))
      {
        rect.x = 580 - tmp->w / 2;
        rect.y = 540 - tmp->h / 2;
        SDL_BlitSurface (tmp, NULL, canvas, &rect);
        SDL_FreeSurface (tmp);
      }

    rect.x = 580 - 360 * ret - surface_circle->w / 2;
    rect.y = 540 - surface_circle->h / 2;
    SDL_BlitSurface (surface_circle, NULL, canvas, &rect);
  
    while (SDL_PollEvent (&event) != 0)
      {
        switch (event.type)
          {
	    case SDL_MOUSEMOTION:
              if (abs (event.motion.y - 540) < 50)
	        {
		  ret = 
		    abs (event.motion.x - 220) < 50 ? 1 :
		    abs (event.motion.x - 580) < 50 ? 0 :
		    ret;
		}
	      break;

	    case SDL_MOUSEBUTTONDOWN:
              if (abs (event.motion.y - 540) < 50)
	        {
		  i = 
		    abs (event.motion.x - 220) < 50 ? 1 :
		    abs (event.motion.x - 580) < 50 ? 0 :
		    -1;

		  if (i >= 0)
		    {
		      keep_going = 0;
		      ret = i;
		    }
		}
	      break;

          case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
              {
              case SDLK_ESCAPE:        
	        ret = 0;
		keep_going = 0;
		break;

              case SDLK_RETURN:        
                if (ret == 0 || path_end != path)
                  {
    		    Mix_PlayChannel (2, chunk_keystroke, 0);
		    load_profile (path);
                    keep_going = 0;
                  }
                break;
  
              case SDLK_BACKSPACE:
                if (path_end != path)
                  {
    		    Mix_PlayChannel (2, chunk_keystroke, 0);
                    --path_end;
                    *path_end = 0;
                  }
                break;

	      case SDLK_LEFT:
	      case SDLK_RIGHT:
	        ret ^= 1;
		break;
  
              default:
                if (path_end != path + PATH_LEN && isprint (event.key.keysym.unicode))
                  {
    		    Mix_PlayChannel (2, chunk_keystroke, 0);
                    *path_end = event.key.keysym.unicode;
                    ++path_end;
		    *path_end = 0;
                  }
		  
                break;
              }
            break;
  
          case SDL_QUIT: exit (0);
  
          default: break;
          }
      }
  
    sync ();
  }

  SDL_EnableUNICODE (0);

  return ret;
}
  
/*
 * Start drawing.
 */
void
draw_background ()
{
  int bpp = canvas->format->BytesPerPixel;
  int pitch = canvas->pitch;
  Uint8 * dst = canvas->pixels;
  Uint8 * src = surface_background->pixels;
  int i;
  int j;

  static const Uint32 transf[] = {
    0, 0, 1, 1, 2, 2, 3, 3, 3, 3, 3, 2, 2, 2, 1, 1
  };   

  SDL_LockSurface (canvas);

  switch (bpp)
    {
    case 1:
      for (j = 0; j < canvas->h; ++j)
        {
	  int k = j + transf[(j + current_frame) & 0xf];
          for (i = 0; i < canvas->w; ++i)
	    {
	      *(dst + j * pitch + i) = *(src + k * pitch + i);
	    }
	}
      break;
      
    case 2:
      for (j = 0; j < canvas->h; ++j)
        {
	  int k = j + transf[(j + current_frame) & 0xf];
          for (i = 0; i < canvas->w; ++i)
	    {
	      *(Uint16 *)(dst + j * pitch + 2 * i) = *(Uint16 *)(src + k * pitch + 2 * i);
	    }
	}
      break;
      
    case 3:
      for (j = 0; j < canvas->h; ++j)
        {
	  int k = j + transf[(j + current_frame) & 0xf];
          for (i = 0; i < canvas->w; ++i)
	    {
	      *(dst + j * pitch + 3 * i + 0) = *(src + k * pitch + 3 * i + 0);
	      *(dst + j * pitch + 3 * i + 1) = *(src + k * pitch + 3 * i + 1);
	      *(dst + j * pitch + 3 * i + 2) = *(src + k * pitch + 3 * i + 2);
	    }
	}
      break;
      
    case 4:
      for (j = 0; j < canvas->h; ++j)
        {
	  int k = j + transf[(j + current_frame) & 0xf];
          for (i = 0; i < canvas->w; ++i)
	    {
	      *(Uint32 *)(dst + j * pitch + 4 * i) = *(Uint32 *)(src + k * pitch + 4 * i);
	    }
	}
      break;
      
    }

  SDL_UnlockSurface (canvas);
  
  ++current_frame;
}

/*
 * Blit object.
 */
void 
draw_object (Uint32 sprite, Uint32 frame, SDL_Rect * dst)
{
  SDL_BlitSurface (surface_objects, &rects_55_63[sprite][frame], canvas, dst);
}

/*
 * Blit effect.
 */
void 
draw_effect (Uint32 sprite, Uint32 frame, SDL_Rect * dst)
{
  SDL_BlitSurface (surface_effects, &rects_55_63[sprite][frame], canvas, dst);
}

/*
 * Blit text.
 */
void 
draw_text (TTF_Font * font, const char * text, Sint16 x, Sint16 y)
{
  static SDL_Color color = { 255, 255, 255 };

  SDL_Surface * tmp;
  SDL_Rect dst = { x, y, 0, 0 };
  
  if ((tmp = TTF_RenderUTF8_Blended (font, text, color)))
    {
      SDL_BlitSurface (tmp, NULL, canvas, &dst);
      SDL_FreeSurface (tmp);
    }
}

/*
 * Warp effect for levitators.
 */
void
warp_surface (SDL_Surface * s, SDL_Rect * rect)
{
  int bpp = s->format->BytesPerPixel;
  int pitch = s->pitch;
  Uint8 * pixels = s->pixels;
  int i;
  int j;

  static const Uint32 transf[] = {
    0, 0, 1, 1, 2, 2, 3, 3, 3, 3, 3, 2, 2, 2, 1, 1
  };   

  SDL_LockSurface (s);

  switch (bpp)
    {
    case 1:
      for (j = rect->y; j < rect->y + rect->h - 12; ++j)
        {
	  int k = j + transf[(j - current_frame) & 0xf];

          for (i = rect->x + 8; i < rect->x + rect->w - 8; ++i)
            {
              *(pixels + j * pitch + i) = *(pixels + k * pitch + i);
            }
	}
      break;

    case 2:
      for (j = rect->y; j < rect->y + rect->h - 12; ++j)
        {
	  int k = j + transf[(j - current_frame) & 0xf];

          for (i = rect->x + 8; i < rect->x + rect->w - 8; ++i)
            {
              *(Uint16 *)(pixels + j * pitch + 2 * i) = *(Uint16 *)(pixels + k * pitch + 2 * i);
            }
	}
      break;

    case 4:
      for (j = rect->y; j < rect->y + rect->h - 12; ++j)
        {
	  int k = j + transf[(j - current_frame) & 0xf];

          for (i = rect->x + 8; i < rect->x + rect->w - 8; ++i)
            {
              *(Uint32 *)(pixels + j * pitch + 4 * i) = *(Uint32 *)(pixels + k * pitch + 4 * i);
            }
	}
      break;
    }

  SDL_UnlockSurface (s);
}

/*
 * Wave effect for menus.
 */
void 
draw_spark (SDL_Surface * s, SDL_Rect * rect)
{
  int bpp = s->format->BytesPerPixel;
  int pitch = s->pitch;
  Uint8 * pixels = s->pixels;
  int i;
  int j;

  float h1;
  float h2;
  float m;

  SDL_LockSurface (s);
 
  h1 = rect->h * sin (.33f * current_frame);
  h2 = rect->h * cos (.33f * current_frame);

  for (i = 0; i < canvas->w; ++i)
    {
      if (i < rect->x)
        { m = 1.f / sqrtf (rect->x - i); }
      else if (i - rect->w <= rect->x)
        { m = 1.f; }
      else 
        { m = 1.f / sqrtf (i - rect->x - rect->w); }
      if (m < .2f)
        { m = .2f; }       

      switch (bpp)
        {
	case 2:
          j = rect->y + m * h1 * cosf (.5f * current_frame - .1f * i + rand () * .5f / RAND_MAX);
          *(Uint16 *)(pixels + j * pitch + 2 * i) = 0xffff;
          j = rect->y + m * h2 * cosf (.33f * current_frame - .1f * i + rand () * .5f / RAND_MAX);
          *(Uint16 *)(pixels + j * pitch + 2 * i) = 0xccff;
	  break;

	case 4:
          j = rect->y + m * h1 * cosf (.5f * current_frame - .1f * i + rand () * .5f / RAND_MAX);
          *(Uint32 *)(pixels + j * pitch + 4 * i) = 0xffffffff;
          j = rect->y + m * h2 * cosf (.33f * current_frame - .1f * i + rand () * .5f / RAND_MAX);
          *(Uint32 *)(pixels + j * pitch + 4 * i) = 0xccccffff;
	  break;
	}
    }

  SDL_UnlockSurface (s);
}

/*
 * The exit particle system
 */
void
draw_particles (SDL_Surface * s, SDL_Rect * dst)
{
  struct particle_t
  {
    Uint8 sprite;
    Sint16 x, y, v;
    Uint8 ttl;
  };
  
  typedef struct particle_t * particle;

  static struct particle_t particles[64] = { { 0 } };

  particle p;
  SDL_Rect pdst;

  for (p = particles; p < particles + 64; ++p)
    {
      if (p->ttl > 0)
        {
	  p->y += p->v;
	  --p->ttl;
	}
      else
        {
	  switch ((int)(5 * (rand () / (RAND_MAX + 1.))))
	    {
	    case 0 ... 1:
	      p->sprite = 0;
	      p->y = dst->h - 38;
	      p->x = 23 * (rand () / (RAND_MAX + 1.));
	      p->v = -4;
	      p->ttl = 16 * (rand () / (RAND_MAX + 1.));
	      break;

	    case 2 ... 3:
	      p->sprite = 1;
	      p->y = dst->h - 22;
	      p->x = 39 * (rand () / (RAND_MAX + 1.));
	      p->v = -6;
	      p->ttl = 16 * (rand () / (RAND_MAX + 1.));
	      break;

	    case 4:
	      p->sprite = 2;
	      p->y = dst->h - 14;
	      p->x = 47 * (rand () / (RAND_MAX + 1.));
	      p->v = -4;
	      p->ttl = 64 * (rand () / (RAND_MAX + 1.));
	      break;
	    }
	}
      pdst.x = dst->x + p->x;
      pdst.y = dst->y + p->y;
      SDL_BlitSurface (surface_effects, rects_particles + p->sprite, canvas, &pdst);
    }
}

void
sepia_surface (SDL_Surface * s)
{
  int bpp = s->format->BytesPerPixel;
  int pitch = s->pitch;
  Uint8 * pixels = s->pixels;
  int i;
  int j;

  Uint8 r1, g1, b1;
  Uint8 r2, g2, b2;

  SDL_LockSurface (s);

  switch (bpp)
    {
    case 1:
      for (j = 0; j < s->h; ++j)
        for (i = 0; i < s->w; ++i)
	  {
	    Uint8 * p = pixels + j * pitch + i;
	    SDL_GetRGB (*p, s->format, &r1, &g1, &b1);
	    r2 = (r1 * .3 + g1 * .6 + b1 * .1);
	    g2 = (r1 * .3 + g1 * .5 + b1 * .1);
	    b2 = (r1 * .2 + g1 * .5 + b1 * .1);
	    *p = SDL_MapRGB (s->format, r2, g2, b2);
	  }
      break;

    case 2:
      for (j = 0; j < s->h; ++j)
        for (i = 0; i < s->w; ++i)
	  {
	    Uint16 * p = (Uint16 *)(pixels + j * pitch + 2 * i);
	    SDL_GetRGB (*p, s->format, &r1, &g1, &b1);
	    r2 = (r1 * .3 + g1 * .6 + b1 * .1);
	    g2 = (r1 * .3 + g1 * .5 + b1 * .1);
	    b2 = (r1 * .2 + g1 * .5 + b1 * .1);
	    *p = SDL_MapRGB (s->format, r2, g2, b2);
	  }
      break;

    case 4:
      for (j = 0; j < s->h; ++j)
        for (i = 0; i < s->w; ++i)
	  {
	    Uint32 * p = (Uint32 *)(pixels + j * pitch + 4 * i);
	    SDL_GetRGB (*p, s->format, &r1, &g1, &b1);
	    r2 = (r1 * .3 + g1 * .6 + b1 * .1);
	    g2 = (r1 * .3 + g1 * .5 + b1 * .1);
	    b2 = (r1 * .2 + g1 * .5 + b1 * .1);
	    *p = SDL_MapRGB (s->format, r2, g2, b2);
	  }
      break;
    }

  SDL_UnlockSurface (s);
}

/*
 * Play music.
 */
void
play_music (int track)
{
  static const char * filenames[] = {
    "track0.ogg",
    "track1.ogg",
    "track2.ogg",
    "track3.ogg",
    "track4.ogg",
    "track5.ogg",
    "track6.ogg",
    "track7.ogg",
  };

  static int current = -1;
  static Mix_Chunk * chunk = NULL; 
  char *path;

  if (enable_audio && enable_music && track != current)
    {
      if (track == -1)
        {
          Mix_HaltChannel (0);
          Mix_FreeChunk (chunk);
	  current = -1;
	}
      else
        {
	  path = strjoin (PATH_SEPARATOR, DATA_DIR, "music", filenames[track], NULL);
          
	  if ((chunk = Mix_LoadWAV (path)) == NULL)
	    {
              fprintf (stderr, "%s: %s\n", path, Mix_GetError ());
	      current = -1;
	    }
	  else
	    {
	      Mix_PlayChannel (0, chunk, -1); 
	      current = track;
	    }

	  free (path);
	}
    }
}



/*
 * Load an image; on error exit the program with the appropiate error message.
 */
SDL_Surface *
load_texture (const char *filename)
{
  char *path;
  SDL_Surface *texture;

  path = strjoin (PATH_SEPARATOR, DATA_DIR, "textures", filename, NULL);

  if ((texture = IMG_Load (path)) == NULL)
    {
      fprintf (stderr, "%s: %s\n", path, SDL_GetError ());
      exit (EXIT_FAILURE);
    }

  free (path);
  return texture;
}

/*
 * Load a true type font; on error exit the program with the appropiate error message.
 */
TTF_Font *
load_font (const char *filename, int size)
{
  char *path;
  TTF_Font *font;

  path = strjoin (PATH_SEPARATOR, DATA_DIR, "fonts", filename, NULL);

  if ((font = TTF_OpenFont (path, size)) == NULL)
    {
      fprintf (stderr, "%s: %s\n", path, TTF_GetError ());
      exit (EXIT_FAILURE);
    }

  free (path);
  return font;
}

/*
 * Load a sample; on error output a message.
 */
Mix_Chunk *
load_sample (const char *filename)
{
  char *path;
  Mix_Chunk *sample;
  path = strjoin (PATH_SEPARATOR, DATA_DIR, "effects", filename, NULL);

  if ((sample = Mix_LoadWAV (path)) == NULL)
    {
      fprintf (stderr, "%s: %s\n", path, Mix_GetError ());
      exit (EXIT_FAILURE);
    }

  free (path);
  return sample;
}

