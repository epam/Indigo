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

#ifndef __lzw_dictionary_h__
#define __lzw_dictionary_h__

#include "base_cpp/array.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/tlscont.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class Output;
    class Scanner;

    class DLLEXPORT LzwDict
    {

    public:
        DECL_ERROR;

        /* Dictionary constants */
        enum
        {
            BITCODE_MIN = 8,
            BITCODE_MAX = 16,
            SIZE = 65536
        };

        LzwDict(void);

        LzwDict(int NewAlphabetSize, int NewBitCodeSize);

        void init(int NewAlphabetSize, int NewBitCodeSize);

        /* Prefix++Char hashing function */
        int hashFunction(const int SearchPrefix, const byte SearchChar) const;

        int getBitCodeSize(void);

        bool addElem(const int NewPrefix, const byte NewChar, int HashIndex);

        int dictSearch(const int SearchPrefix, const byte SearchChar, int HashIndex) const;

        int getAlphabetSize(void);

        int getCode(const int Index) const;

        int getPrefix(const int Index) const;

        byte getChar(const int Index) const;

        int getSize(void) const;

        bool isInitialized(void) const;

        void save(Output& _output);

        void saveFull(Output& _output);

        bool isModified(void);
        void resetModified(void);

        void load(Scanner& _scanner);

        ~LzwDict(void);

        void reset();

    private:
        /* Dictionary element representation type */
        struct _DictElement
        {
            int Prefix;      /* Coded string prefix */
            byte AppendChar; /* String appended char */

            _DictElement(int NewPrefix, byte NewChar) : Prefix(NewPrefix), AppendChar(NewChar)
            {
            }
        };

        int _hashingShift,                                              /* Hashing function shift */
            _bitcodeSize, _alphabetSize, _maxCode, _nextCode, _freePtr; /* Free hash table elements list pointer */

        bool _modified;

        CP_DECL;
        TL_CP_DECL(Array<_DictElement>, _storage); /* Dictionary */

        TL_CP_DECL(Array<int>, _nextPointers);

        TL_CP_DECL(Array<int>, _hashKeys);

        LzwDict(const LzwDict&);
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
