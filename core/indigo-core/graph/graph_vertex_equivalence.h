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

#ifndef __graph_vertex_equivalence__
#define __graph_vertex_equivalence__

namespace indigo
{

    class Graph;
    class Output;
    class Scanner;

    // Find equivalence classes for vertices.
    // Used to check if vertices are equivalent during SSS
    class GraphVertexEquivalence
    {
    public:
        virtual ~GraphVertexEquivalence()
        {
        }

        virtual void construct(const Graph& g)
        {
        }

        virtual void save(Output& output)
        {
        }
        virtual void load(Scanner& input, const Graph& g)
        {
        }

        virtual void prepareForQueries()
        {
        }

        virtual int getVertexEquivalenceClassId(int vertex_idx)
        {
            return -1;
        }
        virtual void fixVertex(int vertex_idx)
        {
        }
        virtual void unfixVertex(int vertex_idx)
        {
        }
        virtual bool useHeuristicFurther()
        {
            return false;
        }

        virtual bool isVertexInTransversal(int vertex_idx)
        {
            return true;
        }

        // This method shouldn't be here...
        virtual void setNeighbourhoodRadius(int radius)
        {
        }
    };

} // namespace indigo

#endif // __graph_vertex_equivalence__
