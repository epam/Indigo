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

#ifndef __shmem_h__
#define __shmem_h__

#include "base_cpp/ptr_array.h"

namespace indigo {
class SharedMemory
{
public:
   explicit SharedMemory (const char *name, int size, bool no_map_if_first);
   virtual ~SharedMemory ();

   inline       bool   wasFirst () {return _was_first; }
   inline const char * getID    () {return _id; }

   void * ptr () {return _pointer;}

   DECL_ERROR;
private:
   bool   _was_first;
   char   _id[1024];
   void * _pointer;
#ifdef _WIN32
   void * _map_object;
#else
   int    _shm_id;
   char   _filename[1024];
#endif
};

}

#endif
