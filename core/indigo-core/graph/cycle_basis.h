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

#ifndef _CYCLE_BASIS_H_
#define _CYCLE_BASIS_H_
#include "base_cpp/obj_array.h"
#include "base_cpp/red_black.h"

namespace indigo
{

    class Graph;

    class CycleBasis
    {
    public:
        CycleBasis()
        {
        }
        void create(const Graph& graph);

        int getCyclesCount() const
        {
            return _cycles.size();
        }
        const Array<int>& getCycle(int num) const
        {
            return _cycles[num];
        }

        bool containsVertex(int vertex) const;
        inline void clear()
        {
            _cycles.clear();
            _cycleVertices.clear();
        }

    private:
        CycleBasis(const CycleBasis&); // no implicit copy

        ObjArray<Array<int>> _cycles;

        RedBlackSet<int> _cycleVertices;
    };

} // namespace indigo
#endif /* _CYCLE_BASIS_H */
