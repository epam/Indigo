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

#ifndef __bracket_spacing_h__
#define __bracket_spacing_h__

#include <optional>
#include <vector>

#include "base_c/defs.h"

namespace indigo
{
    class SGroupTexts;

    // Where the brackets in a gap stand. A gap is the space between a group's outermost atom box and the nearest atom
    // outside it on one side; a step is how far a bracket stands from what it encloses. When the brackets do not fit at
    // STEP, they share the gap
    class DLLEXPORT BracketSpacing
    {
    public:
        // In bond lengths
        static constexpr float STEP = 0.3f;
        static constexpr float MIN_STEP = 0.15f;
        static constexpr float TEXT_CLEARANCE = 0.05f;

        // The brackets between a group and the nearest atom outside it on a side, listed from the group outwards. The
        // length and the distances are in the coordinates of the molecule
        struct Gap
        {
            struct Bracket
            {
                int sgroup_idx;
                bool is_closing; // the right bracket of its group, the one with the texts
            };
            std::vector<Bracket> brackets;
            float length = 0;
            bool rightwards = false; // the atom outside lies to the right of the group
        };

        BracketSpacing(const SGroupTexts& texts, float bond_length);

        // Distances of the brackets from the start of the gap; an unset one stands at STEP from the box it encloses
        std::vector<std::optional<float>> distances(const Gap& gap) const;

    private:
        static constexpr int BISECTION_ROUNDS = 40;

        bool _fits(const Gap& gap) const;
        std::optional<std::vector<float>> _atReducedStep(const Gap& gap) const;
        std::vector<std::optional<float>> _beforeLoneAtom(const Gap& gap) const;
        std::vector<std::optional<float>> _betweenGroups(const Gap& gap) const;
        float _reducedStep(float length, const std::vector<float>& text_widths) const;
        std::vector<float> _textWidths(const Gap& gap) const;

        static size_t _nearCount(const Gap& gap);
        static std::vector<float> _centered(const std::vector<float>& intervals, float length);

        const SGroupTexts& _texts;
        float _bondLength;
    };
} // namespace indigo

#endif
