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

#include <windows.h>

#include "base_c/nano.h"

qword nanoClock (void)
{
   LARGE_INTEGER counter;
   
   if (!QueryPerformanceCounter(&counter))
      return 0;
   
   return (qword)(counter.QuadPart);
}

float nanoHowManySeconds (qword val)
{
   LARGE_INTEGER freq;
   double quot;
   
   if (!QueryPerformanceFrequency(&freq))
      return 0;
   
   quot = (double)val / freq.QuadPart;
      
   return (float)quot;
}
