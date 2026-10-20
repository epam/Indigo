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

#include "layout/sgroup_texts.h"

#include <unordered_set>

#include "layout/sgroup_nesting.h"
#include "molecule/base_molecule.h"

using namespace indigo;

SGroupTexts::SGroupTexts(BaseMolecule& molecule, float bond_length) : _molecule(molecule), _bondLength(bond_length)
{
}

std::optional<Rect2f> SGroupTexts::box(const SGroup& sgroup) const
{
    if (!SGroupNesting::isBracketed(sgroup.sgroup_type) || sgroup.brackets.size() != 2 || !sgroup.hasBrackets())
        return std::nullopt;
    const Vec2f* closing = sgroup.brackets[1];
    const bool first_lower = closing[0].y < closing[1].y;
    const Vec2f& lower = first_lower ? closing[0] : closing[1];
    const Vec2f& upper = first_lower ? closing[1] : closing[0];

    Rect2f box = _labelBox(lower, _label(sgroup));
    const std::string pattern = _pattern(sgroup);
    if (!pattern.empty() && _crossed(sgroup))
        box = Rect2f(box, _patternBox(upper, pattern));
    return box;
}

std::string SGroupTexts::_label(const SGroup& sgroup)
{
    switch (sgroup.sgroup_type)
    {
    case SGroup::SG_TYPE_SRU:
        return sgroup.label.size() > 0 && sgroup.label[0] != 0 ? sgroup.label.ptr() : "n";
    case SGroup::SG_TYPE_COP:
        switch (sgroup.sgroup_subtype.value_or(0))
        {
        case SGroup::SG_SUBTYPE_ALT:
            return "alt";
        case SGroup::SG_SUBTYPE_RAN:
            return "ran";
        case SGroup::SG_SUBTYPE_BLO:
            return "blk";
        default:
            return "co";
        }
    case SGroup::SG_TYPE_COM:
        return sgroup.hasComponentOrder() ? "c" + std::to_string(sgroup.component_order) : "c";
    case SGroup::SG_TYPE_MIX:
        return "mix";
    case SGroup::SG_TYPE_FOR:
        return "f";
    default:
        return "";
    }
}

// An unset repeat pattern reads as either/unknown, as the Molfile format has it
std::string SGroupTexts::_pattern(const SGroup& sgroup)
{
    if (!SGroup::typeHasConnectivity(sgroup.sgroup_type) || sgroup.connectivity == SGroup::HEAD_TO_TAIL)
        return "";
    return sgroup.connectivity == SGroup::HEAD_TO_HEAD ? "hh" : "eu";
}

Rect2f SGroupTexts::_labelBox(const Vec2f& lower_end, const std::string& label) const
{
    return _textBox(lower_end.x + OFFSET * _bondLength, lower_end.y - DROP * FONT * _bondLength, label);
}

Rect2f SGroupTexts::_patternBox(const Vec2f& upper_end, const std::string& pattern) const
{
    return _textBox(upper_end.x + OFFSET * _bondLength, upper_end.y - (1 - DROP) * FONT * _bondLength, pattern);
}

Rect2f SGroupTexts::_textBox(float left, float bottom, const std::string& text) const
{
    const float width = CHARACTER_WIDTH * _bondLength * static_cast<float>(text.size());
    return Rect2f(Vec2f(left, bottom), Vec2f(left + width, bottom + FONT * _bondLength));
}

bool SGroupTexts::_crossed(const SGroup& sgroup) const
{
    const std::unordered_set<int> atoms(sgroup.atoms.begin(), sgroup.atoms.end());
    for (int atom : atoms)
    {
        const Vertex& vertex = _molecule.getVertex(atom);
        for (int i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
        {
            if (atoms.count(vertex.neiVertex(i)) > 0)
                continue;
            Vec2f inside, outside;
            Vec2f::projectZ(inside, _molecule.getAtomXyz(atom));
            Vec2f::projectZ(outside, _molecule.getAtomXyz(vertex.neiVertex(i)));
            for (int b = 0; b < sgroup.brackets.size(); ++b)
                if (Vec2f::segmentsIntersect(inside, outside, sgroup.brackets[b][0], sgroup.brackets[b][1]))
                    return true;
        }
    }
    return false;
}
