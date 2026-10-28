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

#include "layout/metalayout.h"
#include "layout/sgroup_nesting.h"
#include "math/algebra.h"
#include "molecule/elements.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{
    class BaseMolecule;
    class SGroup;

    // Two vertical brackets around a group: a margin outside the boxes of its atom labels, a space outside the brackets
    // of the groups nested in it, whichever reaches further
    class DLLEXPORT SGroupBracketLayout
    {
    public:
        // In bond lengths
        static constexpr float HORIZONTAL_MARGIN = 0.1f;
        static constexpr float VERTICAL_MARGIN = 0.3f;
        static constexpr float HORIZONTAL_SPACE = 0.3f;
        static constexpr float VERTICAL_SPACE = 0.3f;
        static constexpr float DEFAULT_FONT_SIZE =
            LayoutOptions::DEFAULT_FONT_SIZE_PX / LayoutOptions::DEFAULT_BOND_LENGTH_PX * LayoutOptions::DEFAULT_BOND_LENGTH;

        explicit SGroupBracketLayout(BaseMolecule& molecule, float font_size = DEFAULT_FONT_SIZE, LABEL_MODE label_mode = LABEL_MODE_TERMINAL_HETERO);
        static float labelFontSizeInAngstroms(const LayoutOptions& options);

        // Places brackets for the groups that have none and for those whose atoms moved since construction. The
        // brackets of the other groups stay as they are, even when they no longer enclose those of their children.
        void update();

    private:
        bool _hasMoved(const SGroup& sgroup) const;
        bool _allAtomsAtOnePoint() const;
        Rect2f _bracketsBox(int sgroup_idx) const;
        Rect2f _atomsBox(const SGroup& sgroup) const;

        static std::optional<Rect2f> _spannedByBrackets(const SGroup& sgroup);
        static Rect2f _enlarged(const Rect2f& box, float horizontal, float vertical);
        static void _setBrackets(SGroup& sgroup, const Rect2f& box);

        BaseMolecule& _molecule;
        SGroupNesting _nesting;
        std::vector<Vec2f> _initialPositions;
        float _fontSize;
        LABEL_MODE _labelMode;
    };
} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
