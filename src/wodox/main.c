/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009 
 */

#include "../menu/menu.h"		// Menus.
#include "../play/play.h"		// Game engine.
#include "../edit/edit.h"		// Level editor.

#include <getopt.h>		// For getopt


static const char *str_version = 
    PACKAGE " " VERSION "\n"
    "compiled at " __DATE__ "\n"
    "datadir: " DATA_DIR "\n";

static const char *str_usage = 
    "Usage: wodox --help\n"
    "Usage: wodox --version\n"
    "Usage: wodox --edit <file>\n"
    "Usage: wodox [<options>] [<file>...]\n";

static const char *str_help = 
    "\n"
    "  -h, --help		Display this information and exit\n"
    "  -v, --version		Display version info and exit\n"
    "  -e, --edit <file>	Edit the specified level\n"
    "  -l, --lang <lang>	Specify language\n"
    "      --no-audio		Disable all audio\n"
    "      --no-music		Disable all music\n"
    "\n";

static struct option long_options[] = {
    { "no-audio", no_argument, &media.enable_audio, 0 },
    { "no-music", no_argument, &media.enable_music, 0 },
    { "help", no_argument, 0, 'h' },
    { "edit", no_argument, 0, 'e' },
    { "lang", required_argument, 0, 'l' },
    { "version", no_argument, 0, 'v' },
    { 0, 0, 0, 0 }
};

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

    while ((c = getopt_long (argc, argv, "hvel:", long_options, &option_index)) != -1) {
	switch (c) {
	case 0: // If this option set a flag do nothing.
            break;

        case 'h':
	    fputs(str_usage, stdout);
	    fputs(str_help, stdout);
	    exit(EXIT_SUCCESS);
    
        case 'e':
            mode = 1;
	    break;
    
        case 'l':
            lang = optarg;
	    break;
    
        case 'v':
	    fputs(str_version, stdout);
	    exit(EXIT_SUCCESS);
    
        default:
	    fputs(str_usage, stdout);
	    exit(EXIT_FAILURE);
        }
    }

    init_lang (lang);
    media_init ();

    if (optind < argc) {
	for (c = optind; c < argc; ++c) {
	    if (mode == 0){
              if (play (argv[c], argv[c]) == 0)
		  break; 
   	    } else {
	      edit (argv[c]);
            }
        }
    } else {
	menu ();
    }

    media_end ();
    end_lang ();
    return 0;
}




