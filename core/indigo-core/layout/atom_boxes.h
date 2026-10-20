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

#ifndef __atom_boxes_h__
#define __atom_boxes_h__

#include <vector>

#include "math/algebra.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{
    class BaseMolecule;

    // The box of every atom: its centre when it is drawn without a label, otherwise the box of its label in the
    // renderer's default font, the hydrogens on the side the renderer picks for them
    class DLLEXPORT AtomBoxes
    {
    public:
        static constexpr float LABEL_FONT = 0.4f; // in bond lengths

        AtomBoxes(BaseMolecule& molecule, float bond_length);

        Rect2f box(int atom_idx) const;

    private:
        std::vector<Rect2f> _boxes;
    };
} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
