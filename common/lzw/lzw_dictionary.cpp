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

#include "lzw/lzw_dictionary.h"

#include "base_cpp/bitoutworker.h"

#include "base_cpp/output.h"
#include "base_cpp/scanner.h"

#include "base_cpp/list.h"
#include "base_cpp/pool.h"

using namespace indigo;

IMPL_ERROR(LzwDict, "LZW dictionary");

CP_DEF(LzwDict);

LzwDict::LzwDict(void) : CP_INIT, TL_CP_GET(_storage), TL_CP_GET(_nextPointers), TL_CP_GET(_hashKeys)
{
    reset();
}

void LzwDict::reset()
{
    _alphabetSize = -1;
    _modified = false;
    _hashingShift = 8;
    _bitcodeSize = BITCODE_MAX;
    _maxCode = (1 << _bitcodeSize) - 1;
}

LzwDict::LzwDict(int NewAlphabetSize, int NewBitCodeSize) : _modified(false), CP_INIT, TL_CP_GET(_storage), TL_CP_GET(_nextPointers), TL_CP_GET(_hashKeys)
{
    init(NewAlphabetSize, NewBitCodeSize);
}

void LzwDict::init(int NewAlphabetSize, int NewBitCodeSize)
{
    if (NewBitCodeSize > BITCODE_MAX || NewBitCodeSize < BITCODE_MIN)
        throw Error("unexpected bit code size");

    _bitcodeSize = NewBitCodeSize;

    _hashingShift = 8;
    _alphabetSize = NewAlphabetSize;
    _nextCode = _alphabetSize + 1;
    _freePtr = 0;

    _maxCode = (1 << _bitcodeSize) - 1;

    _storage.clear();
    _hashKeys.resize(SIZE);
    _nextPointers.resize(SIZE);

    for (int i = 0; i < SIZE; i++)
    {
        _nextPointers[i] = -1;
        _hashKeys[i] = -1;
    }

    _modified = true;
}

/* Prefix++Char hashing function */
int LzwDict::hashFunction(const int Prefix, const byte Char) const
{
    int Code;

    Code = (Char << _hashingShift) ^ Prefix;

    return Code;
}

int LzwDict::getBitCodeSize(void)
{
    return _bitcodeSize;
}

/* Add dictionary element function */
bool LzwDict::addElem(const int NewPrefix, const byte NewChar, int HashIndex)
{
    int j;
    _DictElement D(NewPrefix, NewChar);

    if (_nextCode <= _maxCode)
    {
        if (_hashKeys[HashIndex] == -1)
            _hashKeys[HashIndex] = _freePtr;
        else
        {
            j = _hashKeys[HashIndex];
            while (true)
            {
                if (_nextPointers[j] == -1)
                {
                    _nextPointers[j] = _freePtr;
                    break;
                }

                j = _nextPointers[j];
            }
        }

        _storage.push(D);

        _freePtr++;

        _nextCode++;

        _modified = true;
        return true;
    }

    return false;
}

/* Dictionary search function */
int LzwDict::dictSearch(const int SearchPrefix, const byte SearchChar, int HashIndex) const
{
    int j;

    if (_hashKeys[HashIndex] == -1)
        return -1;

    j = _hashKeys[HashIndex];

    _DictElement D = _storage[j];

    while (true)
    {
        if (D.AppendChar == SearchChar && D.Prefix == SearchPrefix)
            return j + _alphabetSize + 1;

        if (_nextPointers[j] == -1)
            return -1;

        j = _nextPointers[j];
        D = _storage[j];
    }
    return 1;
}

int LzwDict::getAlphabetSize(void)
{
    return _alphabetSize;
}

/* Get indexed prefix function */
int LzwDict::getPrefix(const int Code) const
{
    if (!isInitialized())
        throw Error("getPrefix(): not initialized");

    return _storage[Code - _alphabetSize - 1].Prefix;
}

/* Get indexed appended char function */
byte LzwDict::getChar(const int Code) const
{
    if (!isInitialized())
        throw Error("getChar(): not initialized");

    return _storage[Code - _alphabetSize - 1].AppendChar;
}

/* Get dictionary size function */
int LzwDict::getSize(void) const
{
    return _storage.size();
}

bool LzwDict::isInitialized(void) const
{
    return _alphabetSize != -1 ? true : false;
}

/* Write lzw dictionary function */
void LzwDict::save(Output& _output)
{
    int n = _storage.size();

    _modified = false;

    _output.writeBinaryInt(_alphabetSize);
    _output.writeBinaryInt(_nextCode);
    _output.writeBinaryInt(n);
    _output.writeBinaryInt(_bitcodeSize);

    for (int i = 0; i < n; i++)
    {
        _output.writeBinaryInt(_storage[i].Prefix);
        _output.writeByte(_storage[i].AppendChar);
    }

    _output.writeBinaryInt(_freePtr);
}

/* Write full (with hash chains, for future modifying) *
 * lzw dictionary function */
void LzwDict::saveFull(Output& _output)
{
    int i, n = _storage.size(), code, HashCode;
    QS_DEF(Array<int>, MarkedHashCodes);

    MarkedHashCodes.resize(SIZE);

    for (i = 0; i < SIZE; i++)
        MarkedHashCodes[i] = 0;

    /* Save main dictionary part */
    save(_output);

    for (i = 0; i < n; i++)
    {
        code = hashFunction(_storage[i].Prefix, _storage[i].AppendChar);

        HashCode = code;

        if (!MarkedHashCodes[HashCode])
        {
            _output.writeBinaryInt(code);

            /* List head */
            code = _hashKeys[code];
            _output.writeBinaryInt(code);

            /* Other elements */
            while (_nextPointers[code] != -1)
            {
                code = _nextPointers[code];
                _output.writeBinaryInt(code);
            }

            _output.writeBinaryInt(-1);

            MarkedHashCodes[HashCode] = 1;
        }
    }
}

void LzwDict::resetModified(void)
{
    _modified = false;
}

bool LzwDict::isModified(void)
{
    return _modified;
}

/* Read full (with hash chains, for future modifying) *
 * lzw dictionary function */
void LzwDict::load(Scanner& _scanner)
{
    int i, j, HashCode, n, k;

    _modified = false;

    _alphabetSize = _scanner.readBinaryInt();
    _nextCode = _scanner.readBinaryInt();
    n = _scanner.readBinaryInt();
    _bitcodeSize = _scanner.readBinaryInt();

    _maxCode = (1 << _bitcodeSize) - 1;

    _storage.clear();
    _storage.resize(n);

    for (i = 0; i < n; i++)
    {
        k = _scanner.readBinaryDword();

        _storage[i].Prefix = k;
        _storage[i].AppendChar = _scanner.readByte();
    }

    _freePtr = _scanner.readBinaryInt();

    _hashKeys.clear_resize(SIZE);
    _nextPointers.clear_resize(SIZE);

    for (i = 0; i < SIZE; i++)
    {
        _nextPointers[i] = -1;
        _hashKeys[i] = -1;
    }

    while (!_scanner.isEOF())
    {
        HashCode = _scanner.readBinaryInt();
        j = _scanner.readBinaryInt();

        _hashKeys[HashCode] = j;

        i = j;
        j = _scanner.readBinaryInt();

        while (j != -1)
        {
            _nextPointers[i] = j;
            i = j;
            j = _scanner.readBinaryInt();
        }
    }
}

LzwDict::~LzwDict(void)
{
}
