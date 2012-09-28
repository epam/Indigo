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

#ifndef __os_dir__
#define __os_dir__

#ifdef __cplusplus
extern "C" {
#endif

enum
{
   OS_DIR_OK,
   OS_DIR_NOTDIR,
   OS_DIR_EXISTS,
   OS_DIR_NOTFOUND,
   OS_DIR_END,
   OS_DIR_OTHER
};

int osDirExists (const char *dirname);
int osDirCreate (const char *dirname);

const char * osDirLastError (char *buf, int max_size);

typedef struct
{
   const char *dirname;
   char path[1024];
   void *dirstream;
#ifdef _WIN32
   char first[1024];
#else
   const char *pattern;
#endif
} OsDirIter;

int osDirSearch (const char *dirname, const char *pattern, OsDirIter *iter);
int osDirNext   (OsDirIter *iter);
void osDirClose  (OsDirIter *iter);

#ifdef __cplusplus
}
#endif

#endif
