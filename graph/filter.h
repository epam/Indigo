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

#ifndef __filter_h__
#define __filter_h__

#include "base_cpp/array.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class Graph;

    class DLLEXPORT Filter
    {
    public:
        enum
        {
            EQ = 1,
            NEQ = 2,
            LESS = 3,
            MORE = 4
        };

        Filter();
        Filter(const int* filter, int type, int value);

        void init(const int* filter, int type, int value);

        void initAll(int size);
        void initNone(int size);

        void hide(int idx);
        void unhide(int idx);

        bool valid(int idx) const;

        void collectGraphVertices(const Graph& graph, Array<int>& indices) const;
        void collectGraphEdges(const Graph& graph, Array<int>& indices) const;
        int count(const Graph& graph) const;
        inline void clear()
        {
            _filter = 0;
            _own.clear();
            _value = 0;
            _type = 0;
        }

        DECL_ERROR;

    protected:
        const int* _filter;

        Array<int> _own;

        int _value;
        int _type;

    private:
        Filter(const Filter&); // no implicit copy
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
