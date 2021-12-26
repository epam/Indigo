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

#pragma once

#include "base_cpp/obj_array.h"
#include "layout_pattern.h"

namespace indigo
{
    class LayoutPatternHolder
    {
    public:
        LayoutPatternHolder();

        ObjArray<PatternLayout>& getPatterns();

    private:
        void _initPatterns();
        static int _pattern_cmp(PatternLayout& p1, PatternLayout& p2, void* context);

        ObjArray<PatternLayout> _patterns;

        DECL_ERROR;
    };
}
