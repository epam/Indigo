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

#ifndef __sgroup_texts_h__
#define __sgroup_texts_h__

#include <optional>
#include <string>

#include "layout/atom_boxes.h"
#include "math/algebra.h"

namespace indigo
{
    class BaseMolecule;
    class SGroup;

    // The texts at the closing bracket of a group, where BIOVIA and RDKit put them: the label as a subscript at the lower
    // right and the repeat pattern as a superscript at the upper right, the pattern only when a crossing bond crosses a
    // bracket
    class DLLEXPORT SGroupTexts
    {
    public:
        SGroupTexts(BaseMolecule& molecule, float bond_length);

        std::optional<Rect2f> box(const SGroup& sgroup) const;

    private:
        static std::string _label(const SGroup& sgroup);
        static std::string _pattern(const SGroup& sgroup);

        // In bond lengths; DROP: the share of a text's height past the end of its bracket
        static constexpr float FONT = AtomBoxes::LABEL_FONT * 2 / 3;
        static constexpr float CHARACTER_WIDTH = 0.6f * FONT;
        static constexpr float OFFSET = 0.05f;
        static constexpr float DROP = 0.75f;

        Rect2f _labelBox(const Vec2f& lower_end, const std::string& label) const;
        Rect2f _patternBox(const Vec2f& upper_end, const std::string& pattern) const;
        Rect2f _textBox(float left, float bottom, const std::string& text) const;
        bool _crossed(const SGroup& sgroup) const;

        BaseMolecule& _molecule;
        float _bondLength;
    };
} // namespace indigo

#endif
