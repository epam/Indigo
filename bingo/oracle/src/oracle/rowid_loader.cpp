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

#include "base_cpp/scanner.h"

#include "base_cpp/array.h"

#include "molecule/molecule.h"
#include "oracle/rowid_loader.h"
#include "oracle/rowid_symbol_codes.h"

IMPL_ERROR(RowIDLoader, "rowID loader");

RowIDLoader::RowIDLoader(LzwDict& NewDict, Scanner& NewIn) : _decoder(NewDict, NewIn)
{
}

void RowIDLoader::loadRowID(std::string& RowId)
{
    RowId.clear();

    int CurCode;

    while (!_decoder.isEOF())
    {
        CurCode = _decoder.get();

        if (CurCode == ROW_ID_SLASH)
        {
            RowId +='/';
            continue;
        }

        if (CurCode == ROW_ID_PLUS)
        {
            RowId += '+';
            continue;
        }

        if (CurCode >= ROW_ID_DIGITS && CurCode < ROW_ID_DIGITS + ROW_ID_NUM_OF_DIGITS)
        {
            CurCode = CurCode + 48 - ROW_ID_DIGITS;
            RowId += ((char)CurCode);
            continue;
        }

        if (CurCode >= ROW_ID_LOWER_CASE && CurCode < ROW_ID_LOWER_CASE + ROW_ID_NUM_OF_SMALL_LETTERS)
        {
            CurCode = CurCode + 97 - ROW_ID_LOWER_CASE;
            RowId += ((char)CurCode);
            continue;
        }

        if (CurCode >= ROW_ID_UPPER_CASE && CurCode < ROW_ID_UPPER_CASE + ROW_ID_NUM_OF_BIG_LETTERS)
        {
            CurCode = CurCode + 65 - ROW_ID_UPPER_CASE;
            RowId += ((char)CurCode);
            continue;
        }

        throw Error("invalid rowID");
    }
}

/* END OF 'ROWID_LOADER.CPP' FILE */
