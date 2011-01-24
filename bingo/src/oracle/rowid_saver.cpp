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

#include "base_cpp/array.h"
#include "base_cpp/tlscont.h"
#include "base_cpp/output.h"

#include "molecule/molecule.h"
#include "oracle/rowid_saver.h"
#include "oracle/rowid_symbol_codes.h"

RowIDSaver::RowIDSaver( LzwDict &NewDict, Output &NewOut ) 
{
   if (!NewDict.isInitialized())
      NewDict.init(ROW_ID_ALPHABET_SIZE, ROW_ID_BIT_CODE_SIZE);

   _encoder_obj.create(NewDict, NewOut);
   _encoder = _encoder_obj.get();
};

void RowIDSaver::saveRowID( const char *RowId )
{
   _encoder->start();

   if (strlen(RowId) != ROW_ID_SIZE)
      throw Error("invalid RowID");

   for (int i = 0; i < ROW_ID_SIZE; i++)
      _encodeSymbol(RowId[i]);
      
   /* Write last symbol */
   _encoder->finish();
}

void RowIDSaver::_encode( int NextSymbol ) 
{
   _encoder->send(NextSymbol);
}

void RowIDSaver::_encodeSymbol( char Symbol )
{
   int RowIDCode = -1;

   if (Symbol == '/')
      RowIDCode = ROW_ID_SLASH;
      
   if (Symbol >= '0' && Symbol <= '9')
      RowIDCode = ROW_ID_DIGITS + Symbol - 48;

   if (Symbol >= 'a' && Symbol <= 'z')
      RowIDCode = ROW_ID_LOWER_CASE + Symbol - 97;

   if (Symbol >= 'A' && Symbol <= 'Z')
      RowIDCode = ROW_ID_UPPER_CASE + Symbol - 65;

   if (Symbol == '+')
      RowIDCode = ROW_ID_PLUS;

   if (RowIDCode == -1)
      throw Error("invalid rowID symbol");

   _encode(RowIDCode);
}

/* END OF 'ROWID_SAVER.CPP' FILE */
