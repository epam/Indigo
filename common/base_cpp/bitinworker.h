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

#ifndef __bitinworker_h__
#define __bitinworker_h__

#include "base_c/defs.h"

namespace indigo
{

class Scanner;

class BitInWorker
{
public:

   BitInWorker( int StartBits, Scanner &NewIn ); 

   bool readBits( int &Code );

   bool isEOF( void );

   ~BitInWorker( void );

private:

   int _bits;                       /* Code size */

   int _bitBufferCount;             

   dword _bitBuffer;        

   Scanner &_scanner;

   BitInWorker( const BitInWorker & );

};

}

#endif /* __bitinworker_h__ */

/* END OF 'BITINWORKER.H' FILE */
