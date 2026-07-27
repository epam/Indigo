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

#include "base_cpp/array.h"
#include "base_cpp/output.h"
#include "base_cpp/tlscont.h"

#include "molecule/molecule.h"
#include "oracle/rowid_saver.h"
#include "oracle/rowid_symbol_codes.h"

using namespace indigo;

IMPL_ERROR(RowIDSaver, "rowID saver");

RowIDSaver::RowIDSaver(LzwDict& NewDict, Output& NewOut)
{
    if (!NewDict.isInitialized())
        NewDict.init(ROW_ID_ALPHABET_SIZE, ROW_ID_BIT_CODE_SIZE);

    _encoder_obj = std::make_unique<LzwEncoder>(NewDict, NewOut);
    _encoder = _encoder_obj.get();
};

void RowIDSaver::saveRowID(const char* RowId)
{
    _encoder->start();

    if (strlen(RowId) != ROW_ID_SIZE)
        throw Error("invalid RowID");

    for (int i = 0; i < ROW_ID_SIZE; i++)
        _encodeSymbol(RowId[i]);

    /* Write last symbol */
    _encoder->finish();
}

void RowIDSaver::_encode(int NextSymbol)
{
    _encoder->send(NextSymbol);
}

void RowIDSaver::_encodeSymbol(char Symbol)
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
