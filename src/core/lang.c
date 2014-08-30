/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "core.h"
#include "lang.h"

struct lang lang = {
    .entername = NULL,
    .enterlink = NULL,
    .hforhelp = NULL,
    .usermanual = NULL,

    .messages = NULL,

    .menus = NULL,
    .menumain = NULL,
    .menusandbox = NULL,
    .menuplay = NULL,
    .menuedit = NULL,
    .menulink = NULL,
    .menusave = NULL,

    .howtoplay = NULL,
    .howtoedit = NULL,
    .howtolink = NULL,
    .howtolink2 = NULL,
    .howtolink3 = NULL,
    .howtolink4 = NULL,
    .credits = NULL,
    .levelnames = NULL,
    .actnames = NULL,
};

static char ** read_messages(const char *dir, const char *filename);

int
lang_init(const char *file)
{
    char *dir;

    dir = strjoin(PATH_SEPARATOR, DATA_DIR, "lang", file, NULL);

    lang.messages = read_messages(dir, "messages.txt");

    lang.entername = lang.messages[0];
    lang.enterlink = lang.messages[1];
    lang.hforhelp = lang.messages[2];
    lang.usermanual = lang.messages[3];

    lang.menus = read_messages(dir, "menus.txt");

    lang.menumain = lang.menus;
    lang.menusandbox = lang.menus + 3;
    lang.menuplay = lang.menus + 4;
    lang.menuedit = lang.menus + 9;
    lang.menulink = lang.menus + 14;
    lang.menusave = lang.menus + 19;

    lang.howtoplay = read_messages(dir, "howtoplay.txt");
    lang.howtoedit = read_messages(dir, "howtoedit.txt");
    lang.howtolink = read_messages(dir, "howtolink.txt");
    lang.howtolink2 = read_messages(dir, "howtolink2.txt");
    lang.howtolink3 = read_messages(dir, "howtolink3.txt");
    lang.howtolink4 = read_messages(dir, "howtolink4.txt");
    lang.levelnames = read_messages(dir, "levelnames.txt");
    lang.actnames = read_messages(dir, "actnames.txt");
    lang.credits = read_messages(dir, "credits.txt");

    free(dir);

    return 0;
}

void
lang_end(void)
{
}

char **
read_messages(const char *dir, const char *filename)
{
    char *path;
    FILE *f;
    char buffer[1024];
    size_t i;
    char **str = NULL;

    path = strjoin(PATH_SEPARATOR, dir, filename, NULL);

    if ((f = fopen(path, "rt"))) {
        for (i = 0; fgets(buffer, sizeof(buffer), f); ++i) {
            if (i % 8 == 0) {
                str = (char **) realloc(str, sizeof(char *) * (i + 8));
                memset(str + i, 0, sizeof(char *) * 8);
            }

            str[i] = strdup(buffer);
            str[i][strlen(str[i]) - 1] = 0;

        }

        fclose(f);
    }

    free(path);
    return str;
}
