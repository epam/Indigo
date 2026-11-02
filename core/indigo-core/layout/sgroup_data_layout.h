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

#ifndef __sgroup_data_layout_h__
#define __sgroup_data_layout_h__

#include <map>
#include <vector>

#include "math/algebra.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{
    class BaseMolecule;
    class DataSGroup;
    class SGroup;

    // Keeps the value of a data S-group where it stood against its atoms, or against its parent's atoms when it has
    // none: a rotation and a shift fitted to how those atoms moved, without a scale or a mirror
    class DLLEXPORT SGroupDataLayout
    {
    public:
        explicit SGroupDataLayout(BaseMolecule& molecule);

        // Moves the values whose reference atoms moved since construction; the others stay as they are
        void update();

    private:
        const SGroup* _referenceGroup(int sgroup_idx) const;
        bool _hasMoved(const SGroup& reference) const;
        void _place(DataSGroup& sgroup, const Vec2f& initial, const SGroup& reference) const;
        Vec2f _position(int atom) const;

        BaseMolecule& _molecule;
        std::vector<Vec2f> _initialAtomPositions;
        std::map<int, Vec2f> _initialValuePositions;
    };
} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
