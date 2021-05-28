/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
