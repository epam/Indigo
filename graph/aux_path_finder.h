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

#ifndef _AUX_PATH_FINDER_H__
#define _AUX_PATH_FINDER_H__

#include "base_cpp/array.h"
#include "base_cpp/queue.h"

namespace indigo
{

    class AuxiliaryGraph;
    class AuxPathFinder
    {
    public:
        AuxPathFinder(AuxiliaryGraph& graph, int max_size);

        bool find(Array<int>& vertices, Array<int>& edges, int u, int v);

    private:
        Queue<int> _queue;
        Array<int> _prev;
        AuxiliaryGraph& _graph;
        AuxPathFinder(const AuxPathFinder&);
    };

} // namespace indigo

#endif /* _AUX_PATH_FINDER_H */
