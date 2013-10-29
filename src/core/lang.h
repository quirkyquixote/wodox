/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2012
 */

#pragma once

struct lang {
    char *entername;
    char *enterlink;
    char *hforhelp;
    char *usermanual;

    char **messages;

    char **menus;
    char **menumain;
    char **menusandbox;
    char **menuplay;
    char **menuedit;
    char **menulink;
    char **menusave;

    char **howtoplay;
    char **howtoedit;
    char **howtolink;
    char **howtolink2;
    char **howtolink3;
    char **howtolink4;
    char **credits;
    char **levelnames;
    char **actnames;
};

extern struct lang lang;

int lang_init(const char *lang);
void lang_end(void);
