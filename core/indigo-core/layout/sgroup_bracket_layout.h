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

#ifndef __sgroup_bracket_layout_h__
#define __sgroup_bracket_layout_h__

#include <optional>
#include <vector>

#include "layout/sgroup_nesting.h"
#include "math/algebra.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{
    class AtomBoxes;
    class BaseMolecule;
    class SGroup;
    class SGroupTexts;

    // Vertical brackets around a group's atom labels and the brackets and texts nested in it. The renderer turns the
    // hooks of a bracket to its right and puts the label at the last bracket: the opening bracket runs up and comes
    // first, the closing one runs down.
    class DLLEXPORT SGroupBracketLayout
    {
    public:
        explicit SGroupBracketLayout(BaseMolecule& molecule);

        // Places brackets for the groups that have none and for those whose atoms moved since construction. The
        // brackets of the other groups stay as they are, even when they no longer enclose those of their children.
        void update();

    private:
        // In bond lengths
        static constexpr float GAP = 0.3f;
        static constexpr float MIN_LENGTH = 1.f;
        static constexpr float TEXT_CLEARANCE = 0.05f;

        bool _hasMoved(const SGroup& sgroup) const;
        bool _hasCoordinates() const;
        Rect2f _boundingBox(int sgroup_idx, const AtomBoxes& atoms) const;
        Rect2f _bracketsBoxAround(int sgroup_idx, const Rect2f& bbox, const SGroupTexts& texts, float bond_length) const;

        static float _averageBondLength(BaseMolecule& molecule);
        static std::optional<Rect2f> _bracketsBox(const SGroup& sgroup);
        static void _setBrackets(SGroup& sgroup, const Rect2f& box);

        BaseMolecule& _molecule;
        SGroupNesting _nesting;
        std::vector<Vec2f> _initialPositions;
    };
} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
