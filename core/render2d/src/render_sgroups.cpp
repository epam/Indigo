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

#include "render_sgroups.h"

#include <algorithm>
#include <cmath>

#include "molecule/molecule_sgroups.h"
#include "render_context.h"
#include "render_internal.h"

using namespace indigo;

// ---- SGroupBrackets ------------------------------------------------------------------------------------------------

SGroupBrackets::SGroupBrackets(std::vector<BracketLine> brackets) : _brackets(std::move(brackets))
{
}

bool SGroupBrackets::empty() const
{
    return _brackets.empty();
}

const std::vector<BracketLine>& SGroupBrackets::all() const
{
    return _brackets;
}

const BracketLine& SGroupBrackets::closing() const
{
    const BracketLine* closing = &_brackets.front();
    for (const BracketLine& bracket : _brackets)
    {
        if (_closesLaterThan(bracket, *closing))
            closing = &bracket;
    }
    return *closing;
}

std::optional<float> SGroupBrackets::pairGap() const
{
    if (_brackets.size() != 2)
        return std::nullopt;
    const float gap = Vec2f::dist(_middle(_brackets[0]), _middle(_brackets[1]));
    return gap > EPSILON ? std::optional<float>(gap) : std::nullopt;
}

bool SGroupBrackets::_closesLaterThan(const BracketLine& bracket, const BracketLine& other)
{
    const Vec2f middle = _middle(bracket), other_middle = _middle(other);
    if (std::fabs(middle.x - other_middle.x) > EPSILON)
        return middle.x > other_middle.x;
    return middle.y > other_middle.y;
}

Vec2f SGroupBrackets::_middle(const BracketLine& bracket)
{
    Vec2f middle;
    middle.lineCombin2(bracket.from, 0.5f, bracket.to, 0.5f);
    return middle;
}

Vec2f SGroupBrackets::lowerEnd(const BracketLine& bracket)
{
    if (std::fabs(bracket.from.y - bracket.to.y) > EPSILON)
        return bracket.from.y > bracket.to.y ? bracket.from : bracket.to;
    return bracket.from.x < bracket.to.x ? bracket.from : bracket.to;
}

Vec2f SGroupBrackets::upperEnd(const BracketLine& bracket)
{
    if (std::fabs(bracket.from.y - bracket.to.y) > EPSILON)
        return bracket.from.y < bracket.to.y ? bracket.from : bracket.to;
    return bracket.from.x > bracket.to.x ? bracket.from : bracket.to;
}

Vec2f SGroupBrackets::hookDirection(const BracketLine& bracket, const Vec2f& towards)
{
    Vec2f normal;
    normal.diff(bracket.to, bracket.from);
    normal.normalize();
    normal.set(-normal.y, normal.x);
    Vec2f to_target;
    to_target.diff(towards, _middle(bracket));
    if (Vec2f::dot(normal, to_target) < 0)
        normal.negate();
    return normal;
}

// ---- SGroupCanvas --------------------------------------------------------------------------------------------------

SGroupCanvas::SGroupCanvas(MoleculeRenderInternal& render, Sgroup& items) : _render(render), _items(items)
{
}

Vec2f SGroupCanvas::atomPosition(int atom) const
{
    return _render._data.atoms[atom].pos;
}

Rect2f SGroupCanvas::atomBox(int atom) const
{
    const AtomDesc& desc = _render._data.atoms[atom];
    Vec2f min, max;
    min.sum(desc.pos, desc.boundBoxMin);
    max.sum(desc.pos, desc.boundBoxMax);
    return Rect2f(min, max);
}

Vec2f SGroupCanvas::atomsCentre(const Array<int>& atoms) const
{
    Vec2f centre;
    for (int i = 0; i < atoms.size(); ++i)
        centre.add(atomPosition(atoms[i]));
    if (atoms.size() > 0)
        centre.scale(1.0f / static_cast<float>(atoms.size()));
    return centre;
}

std::optional<Rect2f> SGroupCanvas::positionsBox(const Array<int>& atoms) const
{
    if (atoms.size() == 0)
        return std::nullopt;
    Rect2f box(atomPosition(atoms[0]), atomPosition(atoms[0]));
    for (int i = 1; i < atoms.size(); ++i)
        box.extend(Rect2f(atomPosition(atoms[i]), atomPosition(atoms[i])));
    return box;
}

Vec2f SGroupCanvas::toDrawing(const Vec2f& point) const
{
    Vec2f on_page;
    _render._objCoordTransform(on_page, point);
    return on_page;
}

Vec2f SGroupCanvas::toDrawingOffset(const Vec2f& offset) const
{
    Vec2f on_page;
    _render._objDistTransform(on_page, offset);
    return on_page;
}

float SGroupCanvas::lineWidth() const
{
    return _render._settings.unit;
}

void SGroupCanvas::addBracket(const BracketLine& line, const Vec2f& hook_direction, float hook_length)
{
    if (_items.bibegin < 0)
        _items.bibegin = _render._data.brackets.size();
    _items.bicount++;
    RenderItemBracket& bracket = _render._data.brackets.push();
    bracket.p0 = line.from;
    bracket.p1 = line.to;
    bracket.n = hook_direction;
    bracket.width = hook_length;
    bracket.q0.lineCombin(bracket.p0, bracket.n, bracket.width);
    bracket.q1.lineCombin(bracket.p1, bracket.n, bracket.width);
}

void SGroupCanvas::hideBrackets()
{
    _items.hide_brackets = true;
}

TextItem& SGroupCanvas::addText(const std::string& text, FONT_SIZE font_size, RenderItem::TYPE type)
{
    TextItem& item = _render._data.textitems[_render._pushTextItem(_items, type)];
    item.text.readString(text.c_str(), true);
    item.fontsize = font_size;
    _render._cw.setTextItemSize(item);
    return item;
}

// ---- SGroupRenderer ------------------------------------------------------------------------------------------------

std::unique_ptr<SGroupRenderer> SGroupRenderer::create(const SGroup& sgroup)
{
    switch (sgroup.sgroup_type)
    {
    case SGroup::SG_TYPE_SRU:
        return std::make_unique<RepeatingUnitRenderer>();
    case SGroup::SG_TYPE_COP:
        return std::make_unique<CopolymerRenderer>();
    case SGroup::SG_TYPE_COM:
        return std::make_unique<ComponentRenderer>();
    case SGroup::SG_TYPE_MIX:
        return std::make_unique<MixtureRenderer>();
    case SGroup::SG_TYPE_FOR:
        return std::make_unique<FormulationRenderer>();
    case SGroup::SG_TYPE_MUL:
        return std::make_unique<MultipleGroupRenderer>();
    case SGroup::SG_TYPE_GEN:
        return std::make_unique<GenericGroupRenderer>();
    case SGroup::SG_TYPE_SUP:
        return std::make_unique<SuperatomRenderer>();
    case SGroup::SG_TYPE_DAT:
        return std::make_unique<DataSGroupRenderer>();
    default:
        return nullptr;
    }
}

void SGroupRenderer::draw(const SGroup& sgroup, const std::optional<Rect2f>&, SGroupCanvas& canvas) const
{
    const SGroupBrackets brackets = _brackets(sgroup, canvas);
    if (brackets.empty())
        return;
    const Vec2f centre = canvas.atomsCentre(sgroup.atoms);
    const std::optional<float> pair_gap = brackets.pairGap();
    for (const BracketLine& bracket : brackets.all())
        canvas.addBracket(bracket, SGroupBrackets::hookDirection(bracket, centre), _hookLength(bracket, pair_gap));

    const BracketLine& closing = brackets.closing();
    const float offset = _labelOffset(canvas);
    const std::string label = _label(sgroup);
    if (!label.empty())
        _placeLabel(canvas.addText(label, FONT_SIZE_ATTR, RenderItem::RIT_SGROUP), SGroupBrackets::lowerEnd(closing), offset);
    const std::string pattern = _repeatPattern(sgroup);
    if (!pattern.empty())
    {
        const Vec2f upper = SGroupBrackets::upperEnd(closing);
        TextItem& item = canvas.addText(pattern, FONT_SIZE_ATTR, RenderItem::RIT_SGROUP);
        item.bbp.set(upper.x + offset, upper.y - item.bbsz.y);
    }
}

std::optional<Rect2f> SGroupRenderer::frameForChildren(const SGroup&, const std::optional<Rect2f>& frame, const SGroupCanvas&) const
{
    return frame;
}

SGroupBrackets SGroupRenderer::_brackets(const SGroup& sgroup, const SGroupCanvas& canvas) const
{
    return _storedBrackets(sgroup, canvas);
}

float SGroupRenderer::_hookLength(const BracketLine&, std::optional<float> pair_gap) const
{
    return pair_gap ? std::min(BRACKET_HOOK, *pair_gap * LONGEST_HOOK_SHARE_OF_GAP) : BRACKET_HOOK;
}

float SGroupRenderer::_labelOffset(const SGroupCanvas&) const
{
    return LABEL_OFFSET;
}

void SGroupRenderer::_placeLabel(TextItem& label, const Vec2f& lower_end, float offset) const
{
    label.bbp.set(lower_end.x + offset, lower_end.y);
}

std::string SGroupRenderer::_label(const SGroup&) const
{
    return "";
}

std::string SGroupRenderer::_repeatPattern(const SGroup&) const
{
    return "";
}

std::string SGroupRenderer::_connectivityText(const SGroup& sgroup)
{
    switch (sgroup.connectivity)
    {
    case SGroup::HEAD_TO_TAIL:
        return "";
    case SGroup::HEAD_TO_HEAD:
        return "hh";
    default:
        return "eu";
    }
}

float SGroupRenderer::_hookOfBracketLength(const BracketLine& bracket)
{
    Vec2f along;
    along.diff(bracket.to, bracket.from);
    return along.length() * HOOK_SHARE_OF_BRACKET;
}

SGroupBrackets SGroupRenderer::_storedBrackets(const SGroup& sgroup, const SGroupCanvas& canvas)
{
    std::vector<BracketLine> lines;
    for (int i = 0; i < sgroup.brackets.size(); ++i)
    {
        if (!sgroup.isZeroLengthBracket(i))
            lines.push_back({canvas.toDrawing(sgroup.brackets[i][0]), canvas.toDrawing(sgroup.brackets[i][1])});
    }
    return SGroupBrackets(std::move(lines));
}

SGroupBrackets SGroupRenderer::_pairAroundAtoms(const SGroup& sgroup, const SGroupCanvas& canvas)
{
    std::vector<BracketLine> lines;
    if (sgroup.atoms.size() > 0)
    {
        Rect2f box = canvas.atomBox(sgroup.atoms[0]);
        for (int i = 1; i < sgroup.atoms.size(); ++i)
            box.extend(canvas.atomBox(sgroup.atoms[i]));
        const float margin = canvas.lineWidth() * 3;
        const float x_min = box.left() - margin, x_max = box.right() + margin;
        const float y_min = box.bottom() - margin, y_max = box.top() + margin;
        lines.push_back({Vec2f(x_min, y_max), Vec2f(x_min, y_min)});
        lines.push_back({Vec2f(x_max, y_min), Vec2f(x_max, y_max)});
    }
    return SGroupBrackets(std::move(lines));
}

SGroupBrackets SGroupRenderer::_storedOrPairAroundAtoms(const SGroup& sgroup, const SGroupCanvas& canvas)
{
    SGroupBrackets stored = _storedBrackets(sgroup, canvas);
    return stored.all().size() >= 2 ? stored : _pairAroundAtoms(sgroup, canvas);
}

// ---- Bracketed groups ----------------------------------------------------------------------------------------------

std::optional<Rect2f> RepeatingUnitRenderer::frameForChildren(const SGroup& sgroup, const std::optional<Rect2f>&, const SGroupCanvas& canvas) const
{
    return canvas.positionsBox(sgroup.atoms);
}

std::string RepeatingUnitRenderer::_label(const SGroup& sgroup) const
{
    const std::string label = DataSGroup::textOf(sgroup.label);
    return label.empty() ? "n" : label;
}

std::string RepeatingUnitRenderer::_repeatPattern(const SGroup& sgroup) const
{
    return _connectivityText(sgroup);
}

std::string CopolymerRenderer::_label(const SGroup& sgroup) const
{
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
}

std::string CopolymerRenderer::_repeatPattern(const SGroup& sgroup) const
{
    return _connectivityText(sgroup);
}

std::string ComponentRenderer::_label(const SGroup& sgroup) const
{
    return sgroup.hasComponentOrder() ? "c" + std::to_string(sgroup.component_order) : "c";
}

std::string MixtureRenderer::_label(const SGroup&) const
{
    return "mix";
}

std::string FormulationRenderer::_label(const SGroup&) const
{
    return "f";
}

// ---- Multiple groups, generic groups, superatoms -------------------------------------------------------------------

std::optional<Rect2f> MultipleGroupRenderer::frameForChildren(const SGroup&, const std::optional<Rect2f>&, const SGroupCanvas&) const
{
    return std::nullopt;
}

SGroupBrackets MultipleGroupRenderer::_brackets(const SGroup& sgroup, const SGroupCanvas& canvas) const
{
    return _storedOrPairAroundAtoms(sgroup, canvas);
}

std::string MultipleGroupRenderer::_label(const SGroup& sgroup) const
{
    return std::to_string(static_cast<const MultipleGroup&>(sgroup).multiplier.value_or(0));
}

std::optional<Rect2f> GenericGroupRenderer::frameForChildren(const SGroup&, const std::optional<Rect2f>&, const SGroupCanvas&) const
{
    return std::nullopt;
}

SGroupBrackets GenericGroupRenderer::_brackets(const SGroup& sgroup, const SGroupCanvas& canvas) const
{
    return _storedOrPairAroundAtoms(sgroup, canvas);
}

float GenericGroupRenderer::_hookLength(const BracketLine& bracket, std::optional<float>) const
{
    return _hookOfBracketLength(bracket);
}

void SuperatomRenderer::draw(const SGroup& sgroup, const std::optional<Rect2f>& frame, SGroupCanvas& canvas) const
{
    if (!DataSGroup::hasText(sgroup.label))
        canvas.hideBrackets();
    SGroupRenderer::draw(sgroup, frame, canvas);
}

std::optional<Rect2f> SuperatomRenderer::frameForChildren(const SGroup&, const std::optional<Rect2f>&, const SGroupCanvas&) const
{
    return std::nullopt;
}

SGroupBrackets SuperatomRenderer::_brackets(const SGroup& sgroup, const SGroupCanvas& canvas) const
{
    return _pairAroundAtoms(sgroup, canvas);
}

float SuperatomRenderer::_hookLength(const BracketLine& bracket, std::optional<float>) const
{
    return _hookOfBracketLength(bracket);
}

float SuperatomRenderer::_labelOffset(const SGroupCanvas& canvas) const
{
    return canvas.lineWidth();
}

void SuperatomRenderer::_placeLabel(TextItem& label, const Vec2f& lower_end, float offset) const
{
    label.bbp.set(lower_end.x + offset, lower_end.y - label.bbsz.y);
}

std::string SuperatomRenderer::_label(const SGroup& sgroup) const
{
    return DataSGroup::textOf(sgroup.label);
}

// ---- Data S-groups -------------------------------------------------------------------------------------------------

void DataSGroupRenderer::draw(const SGroup& sgroup, const std::optional<Rect2f>& frame, SGroupCanvas& canvas) const
{
    const DataSGroup& group = static_cast<const DataSGroup&>(sgroup);
    const int origin_atom = group.originAtom();
    const bool relative_without_origin = group.detached && group.relative && origin_atom < 0 && !frame;
    if (relative_without_origin)
        return;

    std::string text;
    const char tag = group.tag.value_or(0);
    if (tag != 0 && tag != ' ')
    {
        text.push_back(tag);
        text += " = ";
    }
    if (group.data.size() > 0)
    {
        const int skip = group.data[0] == '\\' ? 1 : 0;
        text.append(group.data.ptr() + skip, static_cast<size_t>(group.data.size() - skip));
    }
    TextItem& item = canvas.addText(text, FONT_SIZE_DATA_SGROUP, RenderItem::RIT_DATASGROUP);

    if (!group.detached)
    {
        if (group.atoms.size() > 0)
        {
            const int atom = group.atoms[0];
            item.bbp.set(canvas.atomBox(atom).right() + canvas.lineWidth() * 2, canvas.atomPosition(atom).y - item.bbsz.y / 2);
        }
    }
    else if (group.relative)
    {
        if (group.display_pos.has_value())
            item.bbp = canvas.toDrawingOffset(group.display_pos.value());
        item.bbp.add(origin_atom >= 0 ? canvas.atomPosition(origin_atom) : frame->rightTop());
    }
    else if (group.display_pos.has_value())
    {
        item.bbp = canvas.toDrawing(group.display_pos.value());
    }
}

std::optional<Rect2f> DataSGroupRenderer::frameForChildren(const SGroup&, const std::optional<Rect2f>&, const SGroupCanvas&) const
{
    return std::nullopt;
}
