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

#ifndef __multiple_cml_loader__
#define __multiple_cml_loader__

#include "base_cpp/reusable_obj_array.h"
#include "base_cpp/tlscont.h"

namespace indigo
{

    class Scanner;

    class MultipleCmlLoader
    {
    public:
        DECL_ERROR;

        MultipleCmlLoader(Scanner& scanner);

        bool isEOF();
        void readNext();
        void readAt(int index);
        long long tell();
        int currentNumber();
        int count();

        bool isReaction();

        CP_DECL;
        TL_CP_DECL(Array<char>, data);

    protected:
        TL_CP_DECL(ReusableObjArray<Array<char>>, _tags);
        TL_CP_DECL(Array<long long>, _offsets);
        Scanner& _scanner;
        int _current_number;
        long long _max_offset;
        bool _reaction;
    };

} // namespace indigo
#endif
