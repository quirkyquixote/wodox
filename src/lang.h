/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2012
 */

#pragma once

extern char * str_entername;
extern char * str_enterlink;
extern char * str_hforhelp;
extern char * str_usermanual;

extern char ** str_messages;

extern char ** str_menus;
extern char ** str_menumain;
extern char ** str_menusandbox;
extern char ** str_menuplay;
extern char ** str_menuedit;
extern char ** str_menulink;
extern char ** str_menusave;

extern char ** str_howtoplay;
extern char ** str_howtoedit;
extern char ** str_howtolink;
extern char ** str_howtolink2;
extern char ** str_howtolink3;
extern char ** str_howtolink4;
extern char ** str_credits;
extern char ** str_levelnames;
extern char ** str_actnames;

int init_lang (const char * lang);
void end_lang ();

