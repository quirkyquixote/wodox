/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013 
 */

#include "../menu/menu.h"		// Menus.
#include "../play/play.h"		// Game engine.
#include "../edit/edit.h"		// Level editor.
#include "../media/media.h"		// Graphics and audio

#include <getopt.h>		// For getopt


static const char *str_version = 
    PACKAGE " " VERSION " compiled " __DATE__ "\n";

static const char *str_usage = 
    "Usage: wodox --help\n"
    "       wodox --version\n"
    "       wodox [<options>] --edit <file> ...\n"
    "       wodox [<options>] --play <file> ...\n"
    "       wodox [<options>]\n";

static const char *str_options = 
    "Options:\n"
    "  -e, --edit         skip the menu and edit files\n"
    "  -h, --help         display this help and exit\n"
    "  -l, --lang <lang>  specify language\n"
    "      --no-audio     disable all audio\n"
    "      --no-music     disable all music\n"
    "  -p, --play         skip the menu and play files\n"
    "  -v, --version      output version information and exit\n"
    "\n";

static struct option long_options[] = {
    { "no-audio", no_argument, &media.enable_audio, 0 },
    { "no-music", no_argument, &media.enable_music, 0 },
    { "help", no_argument, 0, 'h' },
    { "edit", no_argument, 0, 'e' },
    { "play", no_argument, 0, 'p' },
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
    char * lang = "en";
    int i;

    init_lang (lang);

    while ((c = getopt_long (argc, argv, "hvepl:", long_options, NULL)) != -1) {
	switch (c) {
	case 0: // If this option set a flag do nothing.
            break;

        case 'h':
	    fputs(str_usage, stdout);
	    fputs(str_options, stdout);
	    exit(EXIT_SUCCESS);
    
        case 'e':
	    media_init();
	    for (i = optind; i < argc; i++)
		edit(argv[i]);
	    media_end();
	    exit(EXIT_SUCCESS);
    
        case 'p':
	    media_init();
	    for (i = optind; i < argc; i++)
		play(argv[i], argv[i]);
	    media_end();
	    exit(EXIT_SUCCESS);
    
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

    if (optind != argc) {
	fputs (str_usage, stdout);
	exit(EXIT_FAILURE);
    }

    media_init();
    menu();
    media_end();

    end_lang ();
    exit(EXIT_SUCCESS);
}




