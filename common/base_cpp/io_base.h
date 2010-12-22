/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
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

#ifndef __io_base_h__
#define __io_base_h__

#include <stdio.h>

namespace indigo
{
   enum Encoding
   {
      ENCODING_ASCII = 1,
      ENCODING_UTF8 = 2
   };

   FILE *openFile( Encoding filename_encoding, const char *filename, const char *mode);
};

#endif
