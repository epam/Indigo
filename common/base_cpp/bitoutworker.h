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

#ifndef __bitoutworker_h__
#define __bitoutworker_h__

#include "base_c/defs.h"

namespace indigo {

class Output;

class BitOutWorker
{
public:

   BitOutWorker( int StartBits, Output &NewOut );

   bool writeBits( int Code );

   void close( void );

   ~BitOutWorker( void );

private:

   int _bits;                       /* Code size */

   int _bitBufferCount;             

   dword _bitBuffer;        

   Output &_output;

   BitOutWorker( const BitOutWorker & );

};

}

#endif /* __bitoutworker_h__ */

/* END OF 'BITOUTWORKER.H' FILE */
