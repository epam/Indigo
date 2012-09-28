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

#include "base_c/nano.h"

#include <sys/time.h>

qword nanoClock (void)
{
   // actually returns microseconds
   struct timeval t;
   struct timezone tz;
   
   gettimeofday(&t, &tz);
   return t.tv_usec + t.tv_sec * 1000000ULL;
}

float nanoHowManySeconds (qword val)
{
   return (float)((double)val / 1000000.);
}
