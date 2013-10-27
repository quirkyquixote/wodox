/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "core.h"
#include "lang.h"

char *str_entername = NULL;
char *str_enterlink = NULL;
char *str_hforhelp = NULL;
char *str_usermanual = NULL;

char **str_messages = NULL;

char **str_menus = NULL;
char **str_menumain = NULL;
char **str_menusandbox = NULL;
char **str_menuplay = NULL;
char **str_menuedit = NULL;
char **str_menulink = NULL;
char **str_menusave = NULL;

char **str_howtoplay = NULL;
char **str_howtoedit = NULL;
char **str_howtolink = NULL;
char **str_howtolink2 = NULL;
char **str_howtolink3 = NULL;
char **str_howtolink4 = NULL;
char **str_credits = NULL;
char **str_levelnames = NULL;
char **str_actnames = NULL;


static inline char **
read_messages (const char *dir, const char *filename)
{
  char *path;
  FILE *f;
  char buffer[1024];
  size_t i;
  char **str = NULL;

  path = strjoin (PATH_SEPARATOR, dir, filename, NULL);

  if ((f = fopen (path, "rt")))
    {
      for (i = 0; fgets (buffer, sizeof (buffer), f); ++i)
	{
	  if (i % 8 == 0)
	    {
	      str = (char **) realloc (str, sizeof (char *) * (i + 8));
	      memset (str + i, 0, sizeof (char *) * 8);
	    }

	  str[i] = strdup (buffer);
	  str[i][strlen (str[i]) - 1] = 0;

	}

      fclose (f);
    }

  free (path);

  return str;
}


int
init_lang (const char *lang)
{
  char *dir;

  dir = strjoin (PATH_SEPARATOR, DATA_DIR, "lang", lang, NULL);

  str_messages = read_messages (dir, "messages.txt");

  str_entername = str_messages[0];
  str_enterlink = str_messages[1];
  str_hforhelp = str_messages[2];
  str_usermanual = str_messages[3];

  str_menus = read_messages (dir, "menus.txt");

  str_menumain = str_menus;
  str_menusandbox = str_menus + 3;
  str_menuplay = str_menus + 4;
  str_menuedit = str_menus + 9;
  str_menulink = str_menus + 14;
  str_menusave = str_menus + 19;

  str_howtoplay = read_messages (dir, "howtoplay.txt");
  str_howtoedit = read_messages (dir, "howtoedit.txt");
  str_howtolink = read_messages (dir, "howtolink.txt");
  str_howtolink2 = read_messages (dir, "howtolink2.txt");
  str_howtolink3 = read_messages (dir, "howtolink3.txt");
  str_howtolink4 = read_messages (dir, "howtolink4.txt");
  str_levelnames = read_messages (dir, "levelnames.txt");
  str_actnames = read_messages (dir, "actnames.txt");
  str_credits = read_messages (dir, "credits.txt");

  free (dir);

  return 0;
}

void
end_lang ()
{
}
