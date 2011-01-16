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

#ifndef __rowid_symbol_codes_h__
#define __rowid_symbol_codes_h__

/* RowId symbol constants 
 * RowId: 18-byte string containing:
 *   A-Z, a-z, 0-9, /, +           */
enum
{
   ROW_ID_BIT_CODE_SIZE = 11,

   ROW_ID_NUM_OF_DIGITS = 10,
   ROW_ID_NUM_OF_SMALL_LETTERS = 26,
   ROW_ID_NUM_OF_BIG_LETTERS = 26,

   ROW_ID_SLASH = 0,

   ROW_ID_DIGITS = 1,

   ROW_ID_LOWER_CASE = (ROW_ID_DIGITS + ROW_ID_NUM_OF_DIGITS),

   ROW_ID_UPPER_CASE = (ROW_ID_LOWER_CASE + ROW_ID_NUM_OF_SMALL_LETTERS),

   ROW_ID_PLUS = 63,

   /* ROW_ID_NUM_OF_DIGITS = 10 +
      ROW_ID_NUM_OF_SMALL_LETTERS = 26 +
      ROW_ID_NUM_OF_BIG_LETTERS = 26 +
      '/' & '+' */
   ROW_ID_ALPHABET_SIZE = 64,

   ROW_ID_SIZE = 18
};

#endif /* __rowid_symbol_codes_h__ */

/* END OF 'ROWID_SYMBOL_CODES.H' FILE */
