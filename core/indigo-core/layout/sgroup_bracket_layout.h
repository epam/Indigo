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

// Brackets of S-groups: ticket #3974.

#include <map>
#include <optional>
#include <vector>

#include "math/algebra.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{
    class BaseMolecule;
    class SGroup;

    // Vertical brackets around a group's atoms and the brackets nested in it. The renderer turns the hooks of a bracket
    // to its right and puts the label at the last bracket: the opening bracket runs up and comes first, the closing
    // one runs down.
    class DLLEXPORT SGroupBracketLayout
    {
    public:
        explicit SGroupBracketLayout(BaseMolecule& molecule);

        void placeAll();
        // A zero-length bracket counts as missing: loaders write such placeholders where a format has no bracket
        // coordinates.
        void placeMissing();

    private:
        static constexpr float GAP = 0.3f;
        static constexpr float MIN_LENGTH = 1.f;

        static bool isBracketed(int sgroup_type);

        void _place(bool only_missing);
        bool _hasCoordinates() const;
        int _findParent(int sgroup_idx) const;
        bool _contains(int sgroup_idx, const std::vector<int>& sorted_atoms) const;
        int _depth(int sgroup_idx) const;
        Rect2f _boundingBox(int sgroup_idx) const;
        void _setBrackets(SGroup& sgroup, const Rect2f& bbox) const;

        static float _averageBondLength(BaseMolecule& molecule);
        static bool _hasBrackets(const SGroup& sgroup);
        static std::optional<Rect2f> _bracketsBox(const SGroup& sgroup);

        BaseMolecule& _molecule;
        float _bondLength = 0;
        std::vector<int> _bracketed;
        std::map<int, int> _parents;
    };
} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
