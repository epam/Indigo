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

#ifndef __rowid_saver_h__
#define __rowid_saver_h__

#include "base_cpp/bitoutworker.h"
#include "lzw/lzw_dictionary.h"
#include "lzw/lzw_encoder.h"

#include "oracle/rowid_symbol_codes.h"
#include "base_cpp/obj.h"

using namespace indigo;

namespace indigo
{
   class Output;
}

class RowIDSaver
{

public:

   DEF_ERROR("rowID saver");

   RowIDSaver( LzwDict &NewDict, Output &NewOut );

   void saveRowID( const char *RowID );

private:

   void _encodeSymbol( char Symbol );

   void _encode( int NextSymbol );

   Obj<LzwEncoder> _encoder_obj;

   LzwEncoder *_encoder;

   // no implicit copy
   RowIDSaver( const RowIDSaver & );

};

#endif /* __rowid_saver_h__ */

/* END OF 'ROWID_SAVER.H' FILE */
