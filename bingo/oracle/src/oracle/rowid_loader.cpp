/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
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

#include "base_cpp/scanner.h"

#include "base_cpp/array.h"

#include "molecule/molecule.h"
#include "oracle/rowid_loader.h"
#include "oracle/rowid_symbol_codes.h"

IMPL_ERROR(RowIDLoader, "rowID loader");

RowIDLoader::RowIDLoader( LzwDict &NewDict, Scanner &NewIn ) : 
   _decoder(NewDict, NewIn)
{   
}

void RowIDLoader::loadRowID( Array<char> &RowId )
{
   RowId.clear();

   int CurCode;

   while (!_decoder.isEOF())
   {
      CurCode = _decoder.get();

      if (CurCode == ROW_ID_SLASH)
      {
         RowId.push('/');
         continue;
      }

      if (CurCode == ROW_ID_PLUS)
      {
         RowId.push('+');
         continue;
      }

      if (CurCode >= ROW_ID_DIGITS && 
         CurCode < ROW_ID_DIGITS + ROW_ID_NUM_OF_DIGITS) 
      {
         CurCode = CurCode + 48 - ROW_ID_DIGITS;
         RowId.push((char)CurCode);
         continue;
      }

      if (CurCode >= ROW_ID_LOWER_CASE && 
         CurCode < ROW_ID_LOWER_CASE + ROW_ID_NUM_OF_SMALL_LETTERS) 
      {
         CurCode = CurCode + 97 - ROW_ID_LOWER_CASE;
         RowId.push((char)CurCode);
         continue;
      }

      if (CurCode >= ROW_ID_UPPER_CASE && 
         CurCode < ROW_ID_UPPER_CASE + ROW_ID_NUM_OF_BIG_LETTERS) 
      {
         CurCode = CurCode + 65 - ROW_ID_UPPER_CASE;
         RowId.push((char)CurCode);
         continue;
      }

      throw Error("invalid rowID");
   }
}

/* END OF 'ROWID_LOADER.CPP' FILE */
