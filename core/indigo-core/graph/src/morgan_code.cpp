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

#include "graph/morgan_code.h"
#include "base_cpp/tlscont.h"

using namespace indigo;

MorganCode::MorganCode(const Graph& g) : _g(g)
{
}

void MorganCode::calculate(Array<long>& codes, int coeff, int iteration_count)
{
    QS_DEF(Array<long>, next_codes);

    next_codes.clear_resize(_g.vertexEnd());
    codes.clear_resize(_g.vertexEnd());

    int i, j, k;

    for (i = _g.vertexBegin(); i < _g.vertexEnd(); i = _g.vertexNext(i))
        codes[i] = _g.getVertex(i).degree();

    for (j = 0; j < iteration_count; j++)
    {
        for (i = _g.vertexBegin(); i < _g.vertexEnd(); i = _g.vertexNext(i))
        {
            next_codes[i] = coeff * codes[i];

            const Vertex& vertex = _g.getVertex(i);

            for (k = vertex.neiBegin(); k < vertex.neiEnd(); k = vertex.neiNext(k))
                next_codes[i] += codes[vertex.neiVertex(k)];
        }

        memcpy(codes.ptr(), next_codes.ptr(), sizeof(long) * _g.vertexEnd());
    }
}
