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

#ifndef __crc32_h__
#define __crc32_h__

namespace indigo {

class CRC32  
{
public:
   explicit CRC32 ();
   virtual ~CRC32 ();

   static unsigned get (const char *text);
   static unsigned get (const char *text, int len);
private:
   unsigned _table[256];
};

}
#endif

