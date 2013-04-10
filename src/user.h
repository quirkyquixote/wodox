/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2012
 */

#pragma once


/*
 * User profile.
 */
#define USER_NAME_LEN 13

typedef struct user_profile_t user_profile;

struct user_profile_t
{
  char name[USER_NAME_LEN];
  int max_level;
};

extern user_profile user;

/*
 * User profile management.
 */
void load_profile (const char * username);
void save_profile ();

