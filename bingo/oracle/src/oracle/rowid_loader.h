/****************************************************************************
 * Copyright (C) 2009-2011 GGA Software Services LLC
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

#ifndef __rowid_loader_h__
#define __rowid_loader_h__

#include "base_cpp/bitinworker.h"
#include "lzw/lzw_dictionary.h"
#include "lzw/lzw_decoder.h"

using namespace indigo;

namespace indigo
{
   class Molecule;
   class Scanner;
}

class RowIDLoader
{
public:

   DECL_ERROR;

   RowIDLoader( LzwDict &NewDict, Scanner &NewIn );

   void loadRowID( Array<char> &RowId );

private:

   int _getNextCode( void );

   LzwDecoder _decoder;

   // no implicit copy
   RowIDLoader( const RowIDLoader & );

};

#endif /* __rowid_loader_h__ */

/* END OF 'ROWID_LOADER.H' FILE */
