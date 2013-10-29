/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2012
 */

#pragma once


/*
 * User profile.
 */
#define USER_NAME_LEN 13

struct user_profile {
    char name[USER_NAME_LEN];
    int max_level;
};

extern struct user_profile user;

/*
 * User profile management.
 */
void load_profile(const char *username);
void save_profile();
