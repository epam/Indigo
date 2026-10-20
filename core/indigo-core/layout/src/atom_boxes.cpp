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

#include "layout/atom_boxes.h"

#include "molecule/base_molecule.h"

using namespace indigo;

AtomBoxes::AtomBoxes(BaseMolecule& molecule, float bond_length) : _boxes(molecule.vertexEnd())
{
    const float font_size = LABEL_FONT * bond_length;
    for (int i = molecule.vertexBegin(); i != molecule.vertexEnd(); i = molecule.vertexNext(i))
    {
        Vec2f bottom_left, top_right;
        molecule.getAtomBoundingBox(i, font_size, LABEL_MODE_TERMINAL_HETERO, bottom_left, top_right, true);
        _boxes[i] = Rect2f(bottom_left, top_right);
    }
}

Rect2f AtomBoxes::box(int atom_idx) const
{
    return _boxes.at(atom_idx);
}
