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

#ifndef __lzw_decoder_h__
#define __lzw_decoder_h__

#include "base_cpp/bitinworker.h"
#include "lzw/lzw_dictionary.h"

namespace indigo {

class Scanner;

class LzwDecoder
{
public:

   DEF_ERROR("LZW decoder");

   LzwDecoder( LzwDict &NewDict, Scanner &NewIn );

   bool isEOF( void );

   int get( void );

private:

   LzwDict &_dict;

   BitInWorker _bitin;

   TL_CP_DECL(Array<byte>, _symbolsBuf);

   // no implicit copy
   LzwDecoder( const LzwDecoder & );

};

}

#endif
