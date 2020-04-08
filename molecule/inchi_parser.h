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

#ifndef __inchi_parser_h__
#define __inchi_parser_h__

#include "base_cpp/array.h"

namespace indigo
{
    DECL_EXCEPTION(InchiParserError);

    // Molecule InChI code constructor class
    class InChICodeParser
    {
    public:
        enum
        {
            STATIC = 1,
            MOBILE = 2
        };

        explicit InChICodeParser(const char* inchi_code);

        int staticHydrogenPositionBegin()
        {
            return _nextElement(STATIC, -1);
        }
        int staticHydrogenPositionNext(int index)
        {
            return _nextElement(STATIC, index);
        }
        int staticHydrogenPositionEnd()
        {
            return _hydrogens.size();
        }

        int mobileHydrogenPositionBegin()
        {
            return _nextElement(MOBILE, -1);
        }
        int mobileHydrogenPositionNext(int index)
        {
            return _nextElement(MOBILE, index);
        }
        int mobileHydrogenPositionEnd()
        {
            return _hydrogens.size();
        }

        int mobileHydrogenCount() const
        {
            return _mobileCount;
        }

        int getHydrogen(int index)
        {
            return _hydrogens.at(index);
        }

        DECL_ERROR;

    private:
        Array<int> _hydrogens;
        Array<int> _types;
        int _mobileCount;

        int _nextElement(int type, int index);
    };

} // namespace indigo

#endif // __inchi_parser_h__
