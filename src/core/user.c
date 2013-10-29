
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "core.h"
#include "user.h"

struct user_profile user = { "", 0 };


static int
access_or_create_dir(const char *path)
{
    if (access(path, F_OK)) {
#ifndef WIN32
	if (mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
#else
	if (mkdir(path))
#endif
	{
	    perror(path);
	    return -1;
	}
    }

    return 0;
}

/*
 * User profile management.
 */
void
load_profile(const char *username)
{
    char *dir;
    char *path;
    FILE *f;

    dir = strjoin(PATH_SEPARATOR, USER_DIR, NULL);

    if (access_or_create_dir(dir) == 0) {
	path = strjoin(PATH_SEPARATOR, dir, username, NULL);

	if ((f = fopen(path, "rb"))) {
	    fread(&user, sizeof(user), 1, f);
	    fclose(f);
	}

	free(path);
    }

    free(dir);
}


void
save_profile()
{
    char *dir;
    char *path;
    FILE *f;

    dir = strjoin(PATH_SEPARATOR, USER_DIR, NULL);

    if (access_or_create_dir(dir) == 0) {
	path = strjoin(PATH_SEPARATOR, dir, user.name, NULL);

	if ((f = fopen(path, "wb"))) {
	    fwrite(&user, sizeof(user), 1, f);
	    fclose(f);
	}

	free(path);
    }

    free(dir);
}
