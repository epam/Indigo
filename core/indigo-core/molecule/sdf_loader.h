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

#ifndef __sdf_loader__
#define __sdf_loader__

#include "base_cpp/properties_map.h"
#include "base_cpp/red_black.h"
#include "base_cpp/tlscont.h"

namespace indigo
{

    class Scanner;

    class SdfLoader
    {
        /*
         * Max data size is 10 Mb
         */
        enum
        {
            MAX_DATA_SIZE = 10485760
        };

    public:
        SdfLoader(Scanner& scanner);
        ~SdfLoader();

        bool isEOF();
        void readNext();

        long long tell();
        int currentNumber();
        int count();

        void readAt(int index);

        CP_DECL;
        TL_CP_DECL(Array<char>, data);
        TL_CP_DECL(PropertiesMap, properties);

        DECL_ERROR;

    protected:
        Scanner* _scanner;
        bool _own_scanner;
        TL_CP_DECL(Array<long long>, _offsets);
        TL_CP_DECL(Array<char>, _preread);
        int _current_number;
        long long _max_offset;
    };

} // namespace indigo

#endif
