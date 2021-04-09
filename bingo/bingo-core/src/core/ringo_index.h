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

#ifndef __ringo_index__
#define __ringo_index__

#include "base_cpp/array.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"

#include "bingo_index.h"

using namespace indigo;

class BingoContext;

class RingoIndex : public BingoIndex
{
public:
    void prepare(Scanner& rxnfile, Output& fi_output, OsLock* lock_for_exclusive_access) override;

    const byte* getFingerprint();
    const Array<char>& getCrf();
    dword getHash();
    const char* getHashStr();

    void clear();

private:
    Array<byte> _fp;
    Array<char> _crf;
    dword _hash;
    Array<char> _hash_str;
};
#endif
