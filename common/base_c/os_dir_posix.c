/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
 * 
 * This file is part of Indigo toolkit.
 * 
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/


#include <stdio.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fnmatch.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "base_c/os_dir.h"

int osDirExists (const char *dirname)
{
   if (access(dirname, 0) == 0)
   {
      struct stat status;

      stat(dirname, &status);

      if (status.st_mode & S_IFDIR)
         return OS_DIR_OK;
      else
      {
         errno = ENOTDIR;
         return OS_DIR_NOTDIR;
      }
   }
   else
      return OS_DIR_NOTFOUND;
}

int osDirCreate (const char *dirname)
{
   int rc = osDirExists(dirname);

   if (rc == OS_DIR_OK)
   {
      errno = EEXIST;
      return OS_DIR_EXISTS;
   }
   if (rc == OS_DIR_NOTDIR)
      return OS_DIR_NOTDIR;

   errno = 0;
   rc = mkdir(dirname, 0777);

   if (rc == 0)
      return OS_DIR_OK;

   if (errno == ENOENT)
      return OS_DIR_NOTFOUND;

   return OS_DIR_OTHER;
}

int osDirSearch (const char *dirname, const char *pattern, OsDirIter *iter)
{
   DIR *dirstream = opendir(dirname);

   if (dirstream == NULL)
   {
      if (errno == ENOENT)
         return OS_DIR_NOTFOUND;
      if (errno == ENOTDIR)
         return OS_DIR_NOTDIR;
      return OS_DIR_OTHER;
   }
   errno = 0;

   iter->dirname = dirname;
   iter->pattern = pattern;
   iter->dirstream = dirstream;
   
   return OS_DIR_OK;
}

int osDirNext (OsDirIter *iter)
{
   DIR *dirstream = (DIR *)iter->dirstream;
   const char *pattern = iter->pattern;

   while (1)
   {
      struct dirent *entry = readdir(dirstream);
      int n;

      if (entry == NULL)
      {
         closedir(dirstream);
         iter->dirstream = 0;
         if (errno == 0)
            return OS_DIR_END;
         return OS_DIR_OTHER;
      }

      const char *name = entry->d_name;

      // match the pattern
      if (pattern != 0)
      {
         if (fnmatch(pattern, name, FNM_PATHNAME) != 0)
            continue;
      }

      n = snprintf(iter->path, sizeof(iter->path), "%s/%s", iter->dirname, name);

      if (n >= sizeof(iter->path))
      {
         errno = ENAMETOOLONG;
         return OS_DIR_OTHER;
      }

      // check that it is a regular file
      if (access(iter->path, 0) == 0)
      {
         struct stat status;

         stat(iter->path, &status);

         if ((status.st_mode & S_IFMT) != S_IFREG)
            continue;
      }
      else
         continue;

      return OS_DIR_OK;
   }
}

void osDirClose (OsDirIter *iter)
{
   if (iter->dirstream != 0)
   {
      closedir((DIR *)iter->dirstream);
      iter->dirstream = 0;
   }
}

const char * osDirLastError (char *buf, int max_size)
{
   strncpy(buf, strerror(errno), max_size);
   return buf;
}

