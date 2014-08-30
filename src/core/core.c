/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009 
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include "core.h"


/*
 * List and sort files in a folder. Used to find levels and music tracks.
 */
int
list_folder (char * path, char *** files)
{
  DIR *dp;
  struct dirent *ep;

  int max_files = 0;
  int file_count = 0;

  *files = NULL;

  if ((dp = opendir (path)))
    {
      while ((ep = readdir (dp)))
        {
          if (ep->d_name[0] != '.')
            {
              if (file_count == max_files)
                {
                  max_files += 8;
                  *files = (char **)realloc (*files, sizeof (char *) * max_files);
                }

              (*files)[file_count++] = strdup (ep->d_name);
            }
        }

      closedir (dp);
      qsort (*files, file_count, sizeof (const char *), cstring_cmp);
      return file_count;
    }

  fprintf (stderr, "%s: %s\n", path, strerror (errno));
  return 0;
}

int 
cstring_cmp (const void *a, const void *b)
{
  const char **ia = (const char **)a;
  const char **ib = (const char **)b;
  return strcmp(*ia, *ib);
}


/*
 * To hell with strcpy, strcat and all their relatives...
 */
char *
strjoin (char *separator, ...)
{
  va_list ap;
  char *dst;
  char *src;
  size_t len;
  size_t size;

  va_start (ap, separator);

  if ((src = va_arg (ap, char *)))
    {
      dst = strdup (src);
      len = strlen (dst);
      size = len + 1;

      while ((src = va_arg (ap, char *)))
        {
          if (separator)
            {
              size += strlen (separator);
              dst = realloc (dst, size);
              strcpy (dst + len, separator);
              len = size - 1;
            }

          size += strlen (src);
          dst = realloc (dst, size);
          strcpy (dst + len, src);
          len = size - 1;
        }
    }
  else
    {
      dst = NULL;
    }

  va_end (ap);
  return dst;
}
