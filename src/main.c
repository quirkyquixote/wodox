/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009 
 */

#include "menu.h"		// Menus.
#include "play.h"		// Game engine.
#include "edit.h"		// Level editor.

#include <getopt.h>		// For getopt


/*
 * Main.
 */
int
main (int argc, char * argv[])
{
  int c;
  int mode = 0;
  int option_index = 0;
  char * lang = "en";

  static struct option long_options[] = {
    { "no-audio", no_argument, &enable_audio, 0 },
    { "no-music", no_argument, &enable_music, 0 },
    { "help", no_argument, 0, 'h' },
    { "edit", no_argument, 0, 'e' },
    { "lang", required_argument, 0, 'l' },
    { "version", no_argument, 0, 'v' },
    { 0, 0, 0, 0 }
  };

  while ((c = getopt_long (argc, argv, "hel:v", long_options, &option_index)) != -1)
    switch (c)
      {
      case 0: // If this option set a flag do nothing.
        break;

      case 'h':
	printf ("Usage: %s [options] [file...]\n", argv[0]);
	puts ("Options:");
	puts ("  -h, --help            Display this information and exit.");
	puts ("  -e, --edit            Edit instead of playing.");
	puts ("  -l, --lang=LANG       Specify language.");
	puts ("      --no-audio        Disable all audio.");
	puts ("      --no-music        Disable music.");
	puts ("  -v, --version         Display version info and exit.");
	puts ("");
	exit (EXIT_SUCCESS);

      case 'e':
        mode = 1;
	break;

      case 'l':
        lang = optarg;
	break;

      case 'v':
	puts (PACKAGE " " VERSION);
	puts ("compiled at " __DATE__);
	puts ("datadir: " DATA_DIR);
	exit (EXIT_SUCCESS);
      }

  init_lang (lang);
  init_graphics ();

  if (optind < argc)
    {
      for (c = optind; c < argc; ++c)
        {
          if (mode == 0)
            {
              if (play (argv[c], argv[c]) == 0)
	        { 
		  break; 
		}
   	    }
          else
            {
	      edit (argv[c]);
            }
        }
    }
  else
    {
      menu ();
    }

  end_graphics ();
  end_lang ();

  return 0;
}




