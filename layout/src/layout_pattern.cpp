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

#include "layout/layout_pattern.h"
#include "base_cpp/tlscont.h"
#include "graph/morgan_code.h"

using namespace indigo;

IMPL_ERROR(PatternLayout, "molecule");

PatternLayout::PatternLayout() : _morgan_code(0), _fixed(false)
{
}

PatternLayout::~PatternLayout()
{
}

int PatternLayout::addAtom(float x, float y)
{
    int idx = addVertex();

    _atoms.expand(idx + 1);

    _atoms[idx].pos.set(x, y);

    return idx;
}

int PatternLayout::addBond(int atom_beg, int atom_end, int type)
{
    int idx = addEdge(atom_beg, atom_end);

    _bonds.expand(idx + 1);
    _bonds[idx].type = type;

    return idx;
}

int PatternLayout::addOutlinePoint(float x, float y)
{
    Vec2f& p = _outline.push();

    p.set(x, y);

    return _outline.size() - 1;
}

const PatternAtom& PatternLayout::getAtom(int idx) const
{
    return _atoms[idx];
}

const PatternBond& PatternLayout::getBond(int idx) const
{
    return _bonds[idx];
}

void PatternLayout::calcMorganCode()
{
    MorganCode morgan(*this);
    QS_DEF(Array<long>, morgan_codes);

    morgan.calculate(morgan_codes, 3, 7);

    _morgan_code = 0;

    for (int i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
        _morgan_code += morgan_codes[i];
}
