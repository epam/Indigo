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

#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "base_cpp/tree.h"
#include "molecule/molecule.h"
#include "molecule/molecule_attachment_groups.h"
#include "molecule/molecule_haptic_bonds.h"
#include "molecule/molecule_sgroups.h"
#include "molecule/query_molecule.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction.h"
#include "render_context.h"
#include "render_internal.h"
#include <algorithm>
#include <limits>
#include <regex>

#ifdef _WIN32
#pragma warning(push, 4)
#endif

using namespace indigo;

namespace
{
    // The size of an atom's charge and radical; the size factor of their glyphs.
    constexpr float ATOM_LABEL_SIZE_FACTOR = 1;

    // The digits of the charge of an attachment group are a third of the font of an atom label; its sign and
    // radical shrink in the same proportion as the digits.
    constexpr float ATTACHMENT_GROUP_LABEL_TO_ATOM_LABEL = 1.0f / 3;

    // Clockwise quarter turns from "up" to the quadrant a direction falls in, y pointing up. Up is [135, 45) degrees,
    // right [45, 315), down [315, 225), left [225, 135): a boundary belongs to the quadrant clockwise of it. A zero
    // direction counts as up.
    int quarterTurnsToQuadrant(float dx, float dy)
    {
        if (dx > 0 && -dx < dy && dy <= dx)
            return 1;
        if (dy < 0 && dy < dx && dx <= -dy)
            return 2;
        if (dx < 0 && dx <= dy && dy < -dx)
            return 3;
        return 0;
    }

    // How far a label of a group may move out of its corner, as a share of the distance from the group centre to
    // its nearest atom: within it the label stays nearer the centre than any atom and reads as the ligand's, not
    // as an atom's charge.
    constexpr float ATTACHMENT_GROUP_LABEL_REACH = 0.5f;

    // With the bond pointing up, the charge of a group stands at the top right of its centre and the radical at the
    // top left (y pointing up); the arrangement turns with the bond.
    const Vec2f CHARGE_CORNER(1, 1);
    const Vec2f RADICAL_CORNER(-1, 1);

    // `corner` turned `quarter_turns` times clockwise, with y flipped to point down as on the canvas.
    Vec2f canvasCorner(Vec2f corner, int quarter_turns)
    {
        for (int turn = 0; turn < quarter_turns; ++turn)
            corner.rotate(-1, 0);
        return Vec2f(corner.x, -corner.y);
    }

    // `rect` as a convex polygon, `margin` wider on every side.
    std::vector<Vec2f> outlineOf(const Rect2f& rect, float margin)
    {
        return {Vec2f(rect.left() - margin, rect.bottom() - margin), Vec2f(rect.right() + margin, rect.bottom() - margin),
                Vec2f(rect.right() + margin, rect.top() + margin), Vec2f(rect.left() - margin, rect.top() + margin)};
    }

    // The band a stroke from `from` to `to` covers, `half_width` to either side of its axis, as a convex polygon
    // `margin` wider all round.
    std::vector<Vec2f> outlineOfStroke(const Vec2f& from, const Vec2f& to, float half_width, float margin)
    {
        Vec2f along;
        along.diff(to, from);
        if (!along.normalize())
            along.set(1, 0);
        Vec2f across(along);
        across.rotate(1, 0);
        across.scale(half_width + margin);
        along.scale(margin);

        Vec2f start, end;
        start.diff(from, along);
        end.sum(to, along);
        return {start - across, end - across, end + across, start + across};
    }

    // The least shift t >= 0 along `direction` at which `label` intersects none of `obstacles`; a shift at which it
    // only touches the margin of one counts as free.
    float leastFreeShift(const std::vector<Vec2f>& label, const Vec2f& direction, const std::vector<std::vector<Vec2f>>& obstacles)
    {
        std::vector<std::pair<float, float>> blocked;
        for (const std::vector<Vec2f>& obstacle : obstacles)
        {
            float low, high;
            if (convexPolygonsIntersectAlong(label, obstacle, direction, low, high))
                blocked.emplace_back(low, high);
        }
        std::sort(blocked.begin(), blocked.end());

        float shift = 0;
        for (const auto& interval : blocked)
        {
            if (interval.first >= shift)
                break;
            shift = std::max(shift, interval.second);
        }
        return shift;
    }

    // A variable attachment shares the container of haptic bonds and is drawn its own way: drawn as a haptic bond it
    // would become one on the picture, and its group would get a label.
    bool isDrawnAsHapticBond(const HapticBond& bond)
    {
        return bond.type() == _BOND_HAPTIC;
    }

    bool hasChargeOrRadical(const AttachmentGroup& group)
    {
        return group.charge() != 0 || group.radical() != 0;
    }

    // Where the labels of one attachment group may stand on the canvas, y pointing down: `gap` from `centre` on both
    // axes, at most `reach` further out of a corner, clear of `obstacles`. Each placed label joins `obstacles`,
    // `clearance` wider.
    struct LabelPlacement
    {
        Vec2f centre;
        Vec2f toward_metal;
        float gap;
        float reach;
        float clearance;
        std::vector<std::vector<Vec2f>>& obstacles;
    };

    // The free place nearest to the centre in the corner `corner` points to: `gap` from the centre on both axes, then,
    // if anything drawn is in the way, out along the diagonal of the corner or along one of its sides, whichever frees
    // the label soonest. Returns how far the label had to move.
    float freePlaceIn(const LabelPlacement& placement, const Vec2f& size, const Vec2f& corner, Vec2f& top_left)
    {
        const Vec2f& centre = placement.centre;
        const float gap = placement.gap;
        top_left.set(corner.x > 0 ? centre.x + gap : centre.x - gap - size.x, corner.y > 0 ? centre.y + gap : centre.y - gap - size.y);
        Vec2f bottom_right;
        bottom_right.sum(top_left, size);
        const std::vector<Vec2f> box = outlineOf(Rect2f(top_left, bottom_right), 0);

        Vec2f diagonal(corner);
        diagonal.normalize();
        const Vec2f directions[3] = {diagonal, Vec2f(corner.x, 0), Vec2f(0, corner.y)};
        Vec2f best_direction = directions[0];
        float best_shift = leastFreeShift(box, directions[0], placement.obstacles);
        for (int k = 1; k < 3; ++k)
        {
            const float shift = leastFreeShift(box, directions[k], placement.obstacles);
            if (shift < best_shift)
            {
                best_shift = shift;
                best_direction = directions[k];
            }
        }
        top_left.addScaled(best_direction, best_shift);
        return best_shift;
    }

    // The top left of a label of `size` meant for `corner`: there when that corner has room within reach of the
    // centre; otherwise in the neighbouring corner on the side of the metal, then in the other neighbouring corner -
    // the label stays by the ligand rather than end up beside an atom. When no corner has room within reach, its own
    // corner, however far.
    Vec2f placeLabel(LabelPlacement& placement, const Vec2f& size, const Vec2f& corner)
    {
        Vec2f clockwise(corner), anticlockwise(corner);
        clockwise.rotate(1, 0);
        anticlockwise.rotate(-1, 0);
        const bool clockwise_first = Vec2f::dot(clockwise, placement.toward_metal) >= Vec2f::dot(anticlockwise, placement.toward_metal);
        const Vec2f corners[3] = {corner, clockwise_first ? clockwise : anticlockwise, clockwise_first ? anticlockwise : clockwise};

        Vec2f top_left;
        bool within_reach = false;
        for (const Vec2f& candidate : corners)
            if (freePlaceIn(placement, size, candidate, top_left) <= placement.reach)
            {
                within_reach = true;
                break;
            }
        if (!within_reach)
            freePlaceIn(placement, size, corner, top_left);

        Vec2f bottom_right;
        bottom_right.sum(top_left, size);
        placement.obstacles.push_back(outlineOf(Rect2f(top_left, bottom_right), placement.clearance));
        return top_left;
    }
}

void MoleculeRenderInternal::_renderBondIds()
{
    // show bond ids
    if (_opt.showBondIds)
    {
        for (int i = _mol->edgeBegin(); i < _mol->edgeEnd(); i = _mol->edgeNext(i))
        {
            TextItem ti;
            ti.fontsize = FONT_SIZE_INDICES;
            ti.color = CWC_DARKGREEN;
            int base = _opt.atomBondIdsFromOne ? 1 : 0;
            bprintf(ti.text, "%i", i + base);
            Vec2f v;
            v.sum(_be(_bd(i).be1).p, _be(_bd(i).be2).p);
            v.scale(0.5);
            _cw.setTextItemSize(ti, v);
            _extendRenderItem(ti, _settings.boundExtent);
            _cw.drawItemBackground(ti);
            _cw.drawTextItemText(ti, _idle);
        }
    }

    // show bond end ids
    if (_opt.showBondEndIds)
    {
        for (int i = 0; i < _data.bondends.size(); ++i)
        {
            TextItem ti;
            ti.fontsize = FONT_SIZE_INDICES;
            ti.color = CWC_RED;
            bprintf(ti.text, "%i", i);
            Vec2f v;
            v.lineCombin2(_be(i).p, 0.75, _be(_getOpposite(i)).p, 0.25);
            _cw.setTextItemSize(ti, v);
            _extendRenderItem(ti, _settings.boundExtent);
            _cw.drawItemBackground(ti);
            _cw.drawTextItemText(ti, _idle);
        }
    }
    if (_opt.showNeighborArcs)
    {
        for (int i = 0; i < _data.bondends.size(); ++i)
        {
            BondEnd& be = _be(i);
            BondEnd& be1 = _be(be.lnei);
            BondEnd& be2 = _be(be.rnei);
            float a0 = atan2(be.dir.y, be.dir.x) - 0.1f;
            float a1 = atan2(be1.dir.y, be1.dir.x) + 0.1f;
            _cw.setSingleSource(CWC_RED);
            _cw.drawArc(_ad(be.aid).pos, _settings.bondSpace * 3, a1, a0);

            float a2 = atan2(be2.dir.y, be2.dir.x) - 0.1f;
            float a3 = atan2(be.dir.y, be.dir.x) + 0.1f;
            _cw.setSingleSource(CWC_DARKGREEN);
            _cw.drawArc(_ad(be.aid).pos, _settings.bondSpace * 3 + _settings.unit, a3, a2);
        }
    }
}

void MoleculeRenderInternal::_renderAtomIds()
{
    if (_opt.showAtomIds)
    {
        for (int i = _mol->vertexBegin(); i < _mol->vertexEnd(); i = _mol->vertexNext(i))
        {
            const AtomDesc& desc = _ad(i);
            for (int j = 0; j < desc.ticount; ++j)
            {
                const TextItem& ti = _data.textitems[j + desc.tibegin];
                if (ti.ritype == RenderItem::RIT_ATOMID)
                {
                    _cw.drawItemBackground(ti);
                    _cw.drawTextItemText(ti, _idle);
                }
            }
        }
    }
}

void MoleculeRenderInternal::_renderEmptyRFragment()
{
    if (!isRFragment || _data.atoms.size() > 0)
        return;
    int attachmentPointBegin = _data.attachmentPoints.size(); // always 0
    int attachmentPointCount = 2;

    Vec2f pos, dir1(1, 0), dir2(-1, 0);
    float offset = 0.4f;
    {
        RenderItemAttachmentPoint& attachmentPoint = _data.attachmentPoints.push();

        attachmentPoint.dir.copy(dir1);
        attachmentPoint.p0.set(0, 0);
        attachmentPoint.p1.lineCombin(pos, dir1, offset);
        attachmentPoint.color = CWC_BASE;
        attachmentPoint.highlighted = false;
        attachmentPoint.number = 1;
    }
    {
        RenderItemAttachmentPoint& attachmentPoint = _data.attachmentPoints.push();

        attachmentPoint.dir.copy(dir2);
        attachmentPoint.p0.set(0, 0);
        attachmentPoint.p1.lineCombin(pos, dir2, offset);
        attachmentPoint.color = CWC_BASE;
        attachmentPoint.highlighted = false;
        attachmentPoint.number = 2;
    }
    _cw.setSingleSource(CWC_BASE);
    for (int i = 0; i < attachmentPointCount; ++i)
        _cw.drawAttachmentPoint(_data.attachmentPoints[attachmentPointBegin + i], _idle);
}

void MoleculeRenderInternal::_renderLabels()
{
    for (int i = _mol->vertexBegin(); i < _mol->vertexEnd(); i = _mol->vertexNext(i))
        _drawAtom(_ad(i));
}

void MoleculeRenderInternal::_renderRings()
{
    for (int i = 0; i < _data.rings.size(); ++i)
    {
        const Ring& ring = _data.rings[i];
        if (ring.aromatic)
        {
            float r = 0.75f * ring.radius;
            for (int k = 0; k < ring.bondEnds.size(); ++k)
            {
                BondEnd& be = _be(ring.bondEnds[k]);
                if (_edgeIsHighlighted(be.bid))
                    _cw.setHighlight();
                float a0 = ring.angles[k], a1 = ring.angles[(k + 1) % ring.bondEnds.size()];
                if (fabs(a1 - a0) > M_PI)
                    _cw.drawArc(ring.center, r, std::max(a0, a1), std::min(a0, a1));
                else
                    _cw.drawArc(ring.center, r, std::min(a0, a1), std::max(a0, a1));
                if (_edgeIsHighlighted(be.bid))
                    _cw.resetHighlight();
            }
        }
    }
}

void MoleculeRenderInternal::_renderBonds()
{
    for (int i = _mol->edgeBegin(); i < _mol->edgeEnd(); i = _mol->edgeNext(i))
        _drawBond(i);
}

bool MoleculeRenderInternal::_hapticEndpointPos(const HapticBond::Endpoint& endpoint, Vec2f& pos, int& label_atom)
{
    label_atom = -1;

    if (!endpoint.isGroup())
    {
        if (!_mol->hasVertex(endpoint.index()))
            return false;
        pos = _ad(endpoint.index()).pos;
        label_atom = endpoint.index();
        return true;
    }

    const MoleculeAttachmentGroups& groups = _mol->attachment_groups;
    if (!groups.hasGroup(endpoint.index()))
        return false;

    // A group end is the centroid of its members: the point the star of a V3000
    // record stands for. Canvas positions, not the molecule's, hence the
    // point-taking form of the rule.
    std::vector<Vec2f> positions;
    for (int atom : groups.group(endpoint.index()).atoms())
    {
        if (!_mol->hasVertex(atom))
            return false;
        positions.push_back(_ad(atom).pos);
    }

    if (positions.empty())
        return false;

    pos = AttachmentGroup::centreOf(positions);
    return true;
}

void MoleculeRenderInternal::_clipHapticEndToLabel(int label_atom, Vec2f& pos, const Vec2f& other)
{
    if (label_atom < 0)
        return;

    Vec2f dir;
    dir.diff(other, pos);
    if (!dir.normalize())
        return;

    // -1 is how _getBondOffset says "this atom draws no label": a value to discard,
    // not to move by - unclamped it drags the end a unit the other way. So do the
    // other two callers.
    const float offset = _getBondOffset(label_atom, pos, dir, _settings.bondLineWidth);
    if (offset > 0)
        pos.addScaled(dir, offset);
}

void MoleculeRenderInternal::_renderHapticBonds()
{
    // No edge of the graph, so no part of the bond pipeline draws it. A plain line:
    // no arrowhead - this is not a dative bond - and no marker at the centre.
    const MoleculeHapticBonds& bonds = _mol->haptic_bonds;
    if (bonds.isEmpty())
        return;

    _cw.setLineWidth(_settings.bondLineWidth);
    _cw.setSingleSource(CWC_BASE);

    for (int i = bonds.begin(); i != bonds.end(); i = bonds.next(i))
    {
        Vec2f begin, end;
        if (_hapticLine(bonds.at(i), begin, end))
            _cw.drawLine(begin, end);
    }
}

bool MoleculeRenderInternal::_hapticLine(const HapticBond& bond, Vec2f& begin, Vec2f& end)
{
    if (!isDrawnAsHapticBond(bond))
        return false;

    int begin_label = -1, end_label = -1;
    if (!_hapticEndpointPos(bond.begin(), begin, begin_label) || !_hapticEndpointPos(bond.end(), end, end_label))
        return false;

    if (Vec2f::dist(begin, end) < EPSILON)
        return false;

    _clipHapticEndToLabel(begin_label, begin, end);
    _clipHapticEndToLabel(end_label, end, begin);
    return true;
}

void MoleculeRenderInternal::_prepareAttachmentGroupLabels()
{
    const MoleculeHapticBonds& bonds = _mol->haptic_bonds;
    const MoleculeAttachmentGroups& groups = _mol->attachment_groups;
    bool any_label = false;
    for (int i = groups.begin(); i != groups.end() && !any_label; i = groups.next(i))
        any_label = hasChargeOrRadical(groups.group(i));
    if (bonds.isEmpty() || !any_label)
        return;

    // Everything already drawn that a label must not touch, kept as far as a bond is kept from an atom label.
    const float clearance = _settings.boundExtent;
    std::vector<std::vector<Vec2f>> obstacles;
    for (int i = _mol->edgeBegin(); i < _mol->edgeEnd(); i = _mol->edgeNext(i))
    {
        const BondDescr& bd = _bd(i);
        obstacles.push_back(outlineOfStroke(_be(bd.be1).p, _be(bd.be2).p, std::max(bd.extP, bd.extN), clearance));
    }
    // Every label item drawn so far - atom labels and their charges, indices, S-group labels. Their boxes already
    // carry that margin (_extendRenderItems); the box an atom keeps of them goes stale when a label is reversed.
    const auto addItemObstacle = [&obstacles](const RenderItem& item) {
        Vec2f max;
        max.sum(item.bbp, item.bbsz);
        obstacles.push_back(outlineOf(Rect2f(item.bbp, max), 0));
    };
    for (int i = 0; i < _data.textitems.size(); ++i)
        addItemObstacle(_data.textitems[i]);
    for (int i = 0; i < _data.graphitems.size(); ++i)
        addItemObstacle(_data.graphitems[i]);
    for (int i = bonds.begin(); i != bonds.end(); i = bonds.next(i))
    {
        Vec2f begin, end;
        if (_hapticLine(bonds.at(i), begin, end))
            obstacles.push_back(outlineOfStroke(begin, end, _settings.bondLineWidth / 2, clearance));
    }

    // Bonds in index order, the order the file lists them: a group's first haptic bond places its label.
    std::vector<bool> labelled(groups.end(), false);
    for (int i = bonds.begin(); i != bonds.end(); i = bonds.next(i))
    {
        const HapticBond& bond = bonds.at(i);
        if (!isDrawnAsHapticBond(bond))
            continue;

        const HapticBond::Endpoint& group_end = bond.begin().isGroup() ? bond.begin() : bond.end();
        if (!group_end.isGroup() || !groups.hasGroup(group_end.index()) || labelled[group_end.index()])
            continue;

        labelled[group_end.index()] = true;
        _prepareAttachmentGroupLabel(bond, obstacles);
    }
}

void MoleculeRenderInternal::_prepareAttachmentGroupLabel(const HapticBond& bond, std::vector<std::vector<Vec2f>>& obstacles)
{
    const bool group_begins = bond.begin().isGroup();
    const HapticBond::Endpoint& group_end = group_begins ? bond.begin() : bond.end();
    const int atom = (group_begins ? bond.end() : bond.begin()).index();

    const AttachmentGroup& group = _mol->attachment_groups.group(group_end.index());
    if (!hasChargeOrRadical(group))
        return;

    Vec2f centre;
    int no_label_atom = -1;
    if (!_hapticEndpointPos(group_end, centre, no_label_atom) || !_mol->hasVertex(atom))
        return;

    // The quadrant comes from the coordinates of the molecule, where y points up as on the picture: the canvas only
    // shifts and scales them, and its rounding must not decide a bond lying on a boundary.
    const Vec3f group_centre = _mol->attachmentGroupCentre(group_end.index());
    const Vec3f& atom_position = _mol->getAtomXyz(atom);
    const int quarter_turns = quarterTurnsToQuadrant(atom_position.x - group_centre.x, atom_position.y - group_centre.y);

    AttachmentGroupLabel& label = _data.attachmentGroupLabels.push();
    // The size of an atom charge times this gives the share of the atom label above.
    const float size_factor = static_cast<float>(ATTACHMENT_GROUP_LABEL_TO_ATOM_LABEL * _cw.fontGetSize(FONT_SIZE_LABEL) / _cw.fontGetSize(FONT_SIZE_ATTR));

    Vec2f toward_metal;
    toward_metal.diff(_ad(atom).pos, centre);

    float nearest_atom = std::numeric_limits<float>::infinity();
    for (int member : group.atoms())
        nearest_atom = std::min(nearest_atom, Vec2f::dist(_ad(member).pos, centre));

    const float reach = ATTACHMENT_GROUP_LABEL_REACH * nearest_atom;
    LabelPlacement placement{centre, toward_metal, _settings.labelInternalOffset, reach, _settings.boundExtent, obstacles};
    const auto place = [&placement](const std::vector<RenderItem*>& parts, const Vec2f& size, const Vec2f& corner) {
        const Vec2f top_left = placeLabel(placement, size, corner);
        for (RenderItem* part : parts)
            part->bbp.add(top_left);
    };

    if (group.charge() != 0)
    {
        const ChargeItems items = _pushChargeItems(label, group.charge(), size_factor, CWC_BASE, false);
        std::vector<RenderItem*> parts;
        Vec2f size;
        if (items.value >= 0)
        {
            TextItem& value = _data.textitems[items.value];
            parts.push_back(&value);
            size.x += value.bbsz.x;
            size.y = std::max(size.y, value.bbsz.y);
        }
        GraphItem& sign = _data.graphitems[items.sign];
        sign.bbp.set(size.x, 0);
        parts.push_back(&sign);
        size.x += sign.bbsz.x;
        size.y = std::max(size.y, sign.bbsz.y);
        place(parts, size, canvasCorner(CHARGE_CORNER, quarter_turns));
    }

    if (group.radical() != 0)
    {
        const RadicalItems items = _pushRadicalItems(label, group.radical(), size_factor, CWC_BASE, false);
        std::vector<RenderItem*> parts;
        GraphItem& first = _data.graphitems[items.first];
        parts.push_back(&first);
        Vec2f size(first.bbsz);
        if (items.second >= 0)
        {
            GraphItem& second = _data.graphitems[items.second];
            second.bbp.set(size.x + items.spacing, 0);
            parts.push_back(&second);
            size.x += items.spacing + second.bbsz.x;
        }
        place(parts, size, canvasCorner(RADICAL_CORNER, quarter_turns));
    }
}

void MoleculeRenderInternal::_renderAttachmentGroupLabels()
{
    for (int i = 0; i < _data.attachmentGroupLabels.size(); ++i)
    {
        const AttachmentGroupLabel& label = _data.attachmentGroupLabels[i];
        for (int j = 0; j < label.ticount; ++j)
            _cw.drawTextItemText(_data.textitems[label.tibegin + j], _idle);
        for (int j = 0; j < label.gicount; ++j)
            _cw.drawGraphItem(_data.graphitems[label.gibegin + j]);
    }
}

void MoleculeRenderInternal::_renderSGroups()
{
    for (int i = 0; i < _data.sgroups.size(); ++i)
    {
        const Sgroup& sg = _data.sgroups[i];
        for (int j = 0; j < sg.ticount; ++j)
            _cw.drawTextItemText(_data.textitems[j + sg.tibegin], _idle);
        for (int j = 0; j < sg.gicount; ++j)
            _cw.drawGraphItem(_data.graphitems[j + sg.gibegin]);
        for (int j = 0; j < sg.bicount; ++j)
            if (!sg.hide_brackets)
                _cw.drawBracket(_data.brackets[j + sg.bibegin]);
    }

    /*
     * Additional fix for #44
     * To avoid overlapping of sgroups brackets with atom labels,
     * we draw a rectangle filled with background color at every
     * atom label position
     */
    if (_data.sgroups.size() > 0)
    {
        for (int i = _mol->vertexBegin(); i < _mol->vertexEnd(); i = _mol->vertexNext(i))
        {
            const AtomDesc& desc = _ad(i);
            for (int j = 0; j < desc.ticount; ++j)
            {
                const TextItem& ti = _data.textitems[j + desc.tibegin];
                _cw.drawItemBackground(ti);
            }
        }
    }
}

void MoleculeRenderInternal::_applyBondOffset()
{
    for (int i = 0; i < _data.bondends.size(); ++i)
    {
        BondEnd& be = _be(i);
        be.p.addScaled(be.dir, be.offset);
    }
}

void MoleculeRenderInternal::_setBondCenter()
{
    // find bond center
    for (int i = _mol->edgeBegin(); i < _mol->edgeEnd(); i = _mol->edgeNext(i))
    {
        BondDescr& bd = _bd(i);
        bd.center.lineCombin2(_be(bd.be1).p, 0.5f, _be(bd.be2).p, 0.5f);
    }
}

void MoleculeRenderInternal::_findNeighbors()
{
    // prepare bond end data
    for (int i = _mol->vertexBegin(); i < _mol->vertexEnd(); i = _mol->vertexNext(i))
    {
        const Vertex& vertex = _mol->getVertex(i);
        for (int j = vertex.neiBegin(); j < vertex.neiEnd(); j = vertex.neiNext(j))
        {
            int be1idx = _getBondEndIdx(i, j);
            BondEnd& be1 = _be(be1idx);
            if (vertex.degree() > 1)
            {
                // find right and left neighbor of each bond end
                for (int k = vertex.neiBegin(); k < vertex.neiEnd(); k = vertex.neiNext(k))
                {
                    if (k == j)
                        continue;
                    int be2idx = _getBondEndIdx(i, k);
                    BondEnd& be2 = _be(be2idx);
                    float dot = Vec2f::dot(be1.dir, be2.dir);
                    float cross = -Vec2f::cross(be1.dir, be2.dir);
                    float angle = atan2(cross, dot);
                    if (angle < 0)
                        angle += 2 * (float)M_PI;
                    float rAngle = (2 * (float)M_PI - angle);
                    if (be1.lnei < 0 || angle < be1.lang)
                    {
                        be1.lnei = be2idx;
                        be1.lsin = cross;
                        be1.lcos = dot;
                        be1.lang = angle;
                    }
                    if (be1.rnei < 0 || rAngle < be1.rang)
                    {
                        be1.rnei = be2idx;
                        be1.rsin = sin(rAngle);
                        be1.rcos = cos(rAngle);
                        be1.rang = rAngle;
                    }
                }
                int prev = _getOpposite(be1.lnei);
                _be(prev).next = be1idx;
            }
            else if (vertex.degree() == 1)
            {
                int prev = _getOpposite(be1idx);
                _be(prev).next = be1idx;
                be1.lnei = be1.rnei = be1idx;
                be1.lsin = be1.rsin = 0;
                be1.lcos = be1.rcos = 1;
                be1.lang = be1.rang = (float)(2 * M_PI);
            }
        }
    }
}

void MoleculeRenderInternal::_findCenteredCase()
{
    // bonds with both labels visible
    for (int i = _mol->edgeBegin(); i < _mol->edgeEnd(); i = _mol->edgeNext(i))
    {
        BondEnd& be1 = _be(_bd(i).be1);
        BondEnd& be2 = _be(_bd(i).be2);
        if (_ad(be1.aid).showLabel && _ad(be2.aid).showLabel)
            be1.centered = be2.centered = true;
    }

    // other cases
    for (int i = _mol->vertexBegin(); i < _mol->vertexEnd(); i = _mol->vertexNext(i))
    {
        const Vertex& vertex = _mol->getVertex(i);
        const AtomDesc& ad = _ad(i);
        for (int j = vertex.neiBegin(); j < vertex.neiEnd(); j = vertex.neiNext(j))
        {
            if (_bd(vertex.neiEdge(j)).inRing)
                continue;
            BondEnd& be = _getBondEnd(i, j);
            if (ad.showLabel)
            {
                be.centered = true;
                continue; // prolongation is not required when label is shown
            }

            if (be.lnei == be.rnei) // zero or one neighbor
                continue;
            if (be.lsin < _settings.prolongAdjSinTreshold || be.rsin < _settings.prolongAdjSinTreshold) // angle should not be to large or too small
                continue;
            const BondDescr& bdl = _bd(_be(be.lnei).bid);
            const BondDescr& bdr = _bd(_be(be.rnei).bid);

            if (((bdl.type == BOND_SINGLE && (bdl.stereodir == 0 || _opt.centerDoubleBondWhenStereoAdjacent)) &&
                 (bdr.type == BOND_SINGLE && (bdr.stereodir == 0 || _opt.centerDoubleBondWhenStereoAdjacent))) ||
                (bdr.type == BOND_SINGLE && bdl.type == BOND_SINGLE && bdl.stereodir != 0 && bdr.stereodir != 0))
            {
                be.centered = true;
                be.prolong = true;
            }
        }
    }

    for (int i = _mol->edgeBegin(); i < _mol->edgeEnd(); i = _mol->edgeNext(i))
    {
        BondDescr& bd = _bd(i);
        bd.centered = _be(bd.be1).centered && _be(bd.be2).centered;
    }
}

float MoleculeRenderInternal::_getBondOffset(int aid, const Vec2f& pos, const Vec2f& dir, const float bondWidth)
{
    if (!_ad(aid).showLabel)
        return -1;

    float maxOffset = 0, offset = 0;
    for (int k = 0; k < _ad(aid).ticount; ++k)
    {
        TextItem& item = _data.textitems[_ad(aid).tibegin + k];
        if (item.noBondOffset)
            continue;
        if (_clipRayBox(offset, pos, dir, item.bbp, item.bbsz, bondWidth))
            maxOffset = std::max(maxOffset, offset);
    }
    for (int k = 0; k < _ad(aid).gicount; ++k)
    {
        GraphItem& item = _data.graphitems[_ad(aid).gibegin + k];
        if (item.noBondOffset)
            continue;
        if (_clipRayBox(offset, pos, dir, item.bbp, item.bbsz, bondWidth))
            maxOffset = std::max(maxOffset, offset);
    }
    return maxOffset + _settings.unit * 2;
}

void MoleculeRenderInternal::_calculateBondOffset()
{
    // calculate offset for bonds
    for (int i = _mol->vertexBegin(); i < _mol->vertexEnd(); i = _mol->vertexNext(i))
    {
        const Vertex& vertex = _mol->getVertex(i);
        for (int j = vertex.neiBegin(); j < vertex.neiEnd(); j = vertex.neiNext(j))
        {
            BondEnd& be1 = _getBondEnd(i, j);
            be1.offset = std::max(be1.offset, _getBondOffset(i, be1.p, be1.dir, be1.width));
        }
    }

    for (int i = _mol->edgeBegin(); i < _mol->edgeEnd(); i = _mol->edgeNext(i))
    {
        const BondDescr& bd = _bd(i);
        BondEnd& be1 = _be(bd.be1);
        BondEnd& be2 = _be(bd.be2);

        // if the offsets may result in too short bond,
        float offsum = be1.offset + be2.offset;
        if (offsum > 1e-3f && bd.length < offsum + _settings.minBondLength)
        {
            // if possible, scale down the offsets so that the bond has at least the specified length
            if (bd.length > _settings.minBondLength)
            {
                float factor = (bd.length - _settings.minBondLength) / offsum;
                be1.offset *= factor;
                be2.offset *= factor;
            }
            // otherwise, ignore offsets (extreme case)
            else
            {
                be1.offset = be2.offset = 0;
            }
        }
    }
}

void MoleculeRenderInternal::_initBondData()
{
    float thicknessHighlighted = _cw.highlightedBondLineWidth();

    for (int i = _mol->edgeBegin(); i < _mol->edgeEnd(); i = _mol->edgeNext(i))
    {
        BondDescr& d = _bd(i);
        d.type = _mol->getBondOrder(i);
        d.thickness = _edgeIsHighlighted(i) ? thicknessHighlighted : _settings.bondLineWidth;
        d.queryType = -1;
        QUERY_MOL_BEGIN(_mol);
        {
            QueryMolecule::Bond& qb = qmol.getBond(i);
            d.queryType = QueryMolecule::getQueryBondType(qb);
            d.stereoCare = qmol.bondStereoCare(i);
            if (qb.hasConstraint(QueryMolecule::BOND_TOPOLOGY))
            {
                bool chainPossible = qb.possibleValue(QueryMolecule::BOND_TOPOLOGY, TOPOLOGY_CHAIN);
                bool ringPossible = qb.possibleValue(QueryMolecule::BOND_TOPOLOGY, TOPOLOGY_RING);
                d.topology = 0;
                if (chainPossible && !ringPossible)
                {
                    d.topology = TOPOLOGY_CHAIN;
                }
                if (ringPossible && !chainPossible)
                {
                    d.topology = TOPOLOGY_RING;
                }
            }
        }
        QUERY_MOL_END;
        const Edge& edge = _mol->getEdge(i);
        d.beg = edge.beg;
        d.end = edge.end;
        d.vb = _ad(d.beg).pos;
        d.ve = _ad(d.end).pos;
        d.dir.diff(d.ve, d.vb);
        d.length = d.dir.length();
        d.dir.normalize();
        d.norm.set(-d.dir.y, d.dir.x);
        d.isShort = d.length < (_settings.bondSpace + _settings.bondLineWidth) * 2; // TODO: check

        d.stereodir = _mol->getBondDirection(i);
        d.cistrans = _mol->cis_trans.isIgnored(i);
        int ubid = static_cast<long>(_bondMappingInv.size()) > i ? _bondMappingInv.at(i) : i;
        if (_data.reactingCenters.size() > ubid)
            d.reactingCenter = _data.reactingCenters[ubid];
    }
}

void MoleculeRenderInternal::_initBoldStereoBonds()
{
    if (!_opt.boldBondDetection)
        return;
    for (int i = _mol->edgeBegin(); i < _mol->edgeEnd(); i = _mol->edgeNext(i))
    {
        BondDescr& d = _bd(i);
        const Vertex& v1 = _mol->getVertex(d.beg);
        const Vertex& v2 = _mol->getVertex(d.end);
        bool hasNeighboringUpBond1 = false;
        for (int j = v1.neiBegin(); j < v1.neiEnd(); j = v1.neiNext(j))
            if (v1.neiEdge(j) != i && _bd(v1.neiEdge(j)).stereodir == BOND_UP && _bd(v1.neiEdge(j)).end == d.beg)
                hasNeighboringUpBond1 = true;
        bool hasNeighboringUpBond2 = false;
        for (int j = v2.neiBegin(); j < v2.neiEnd(); j = v2.neiNext(j))
            if (v2.neiEdge(j) != i && _bd(v2.neiEdge(j)).stereodir == BOND_UP && _bd(v2.neiEdge(j)).end == d.end)
                hasNeighboringUpBond2 = true;
        if (hasNeighboringUpBond1 && hasNeighboringUpBond2)
            d.stereodir = BOND_STEREO_BOLD;
    }
}

void MoleculeRenderInternal::_initBondEndData()
{
    for (int i = _mol->edgeBegin(); i < _mol->edgeEnd(); i = _mol->edgeNext(i))
    {
        const Edge& edge = _mol->getEdge(i);
        const AtomDesc& bdesc = _ad(edge.beg);
        const AtomDesc& edesc = _ad(edge.end);

        BondDescr& bondd = _bd(i);

        bondd.be1 = _data.bondends.size();
        _data.bondends.push();
        bondd.be2 = _data.bondends.size();
        _data.bondends.push();

        BondEnd& be1 = _be(bondd.be1);
        BondEnd& be2 = _be(bondd.be2);
        be1.clear();
        be2.clear();
        be1.bid = be2.bid = i;
        be1.aid = edge.beg;
        be1.dir.diff(edesc.pos, bdesc.pos);
        be1.dir.normalize();
        be1.lnorm.copy(be1.dir);
        be1.lnorm.rotate(1, 0);
        be1.p.copy(bdesc.pos);

        be2.aid = edge.end;
        be2.dir.negation(be1.dir);
        be2.lnorm.negation(be1.lnorm);
        be2.p.copy(edesc.pos);
    }

    for (int i = 0; i < _data.bondends.size(); ++i)
    {
        BondEnd& be = _be(i);
        const BondDescr& bd = _bd(be.bid);
        if (bd.type == BOND_SINGLE)
            if (bd.stereodir != 0)
                if (bd.be1 == i) // is source end?
                    be.width = 0;
                else
                    be.width = 2 * (_settings.bondSpace + _settings.bondLineWidth);
            else
                be.width = 2 * _settings.bondSpace + _settings.bondLineWidth;
        else if (bd.type == BOND_DOUBLE || bd.type == BOND_AROMATIC || bd.type == BOND_TRIPLE || bd.queryType >= 0)
            be.width = 4 * _settings.bondSpace + _settings.bondLineWidth;
        else if (bd.type == _BOND_HYDROGEN || bd.type == _BOND_COORDINATION)
            be.width = 2 * (_settings.bondSpace + _settings.bondLineWidth);
        else
        {
            Array<char> buf;
            _mol->getBondDescription(be.bid, buf);
            throw Error("Unknown bond type %s. Can not determine bond width.", buf.ptr());
        }
    }
}

void MoleculeRenderInternal::_extendRenderItems()
{
    for (int i = 0; i < _data.textitems.size(); ++i)
        _extendRenderItem(_data.textitems[i], _settings.boundExtent);
    for (int i = 0; i < _data.graphitems.size(); ++i)
        _extendRenderItem(_data.graphitems[i], _settings.boundExtent);
}

BondEnd& MoleculeRenderInternal::_getBondEnd(int aid, int nei)
{
    return _be(_getBondEndIdx(aid, nei));
}

int MoleculeRenderInternal::_getBondEndIdx(int aid, int nei)
{
    int ne = _mol->getVertex(aid).neiEdge(nei);
    int be = _bd(ne).getBondEnd(aid);
    return be;
}

void MoleculeRenderInternal::_drawAtom(const AtomDesc& desc)
{
#ifdef RENDER_SHOW_BACKGROUND
    for (int i = 0; i < desc.ticount; ++i)
        _cw.drawItemBackground(_data.textitems[i + desc.tibegin]);
    for (int i = 0; i < desc.gicount; ++i)
        _cw.drawItemBackground(_data.graphitems[i + desc.gibegin]);
#endif

    _cw.setSingleSource(desc.color);
    for (int i = 0; i < desc.ticount; ++i)
    {
        if (desc.hcolorSet)
            _cw.drawTextItemText(_data.textitems[i + desc.tibegin], desc.hcolor, _idle);
        else
            _cw.drawTextItemText(_data.textitems[i + desc.tibegin], _idle);
    }
    for (int i = 0; i < desc.attachmentPointCount; ++i)
        _cw.drawAttachmentPoint(_data.attachmentPoints[desc.attachmentPointBegin + i], _idle);
    for (int i = 0; i < desc.rSiteAttachmentIndexCount; ++i)
        _cw.drawRSiteAttachmentIndex(_data.rSiteAttachmentIndices[desc.rSiteAttachmentIndexBegin + i]);
    for (int i = 0; i < desc.gicount; ++i)
    {
        if (desc.hcolorSet)
            _cw.drawGraphItem(_data.graphitems[i + desc.gibegin], desc.hcolor);
        else
            _cw.drawGraphItem(_data.graphitems[i + desc.gibegin]);
    }
}

void MoleculeRenderInternal::_writeQueryAtomToString(Output& output, int aid)
{
    BaseMolecule& bm = *_mol;
    AtomDesc& ad = _ad(aid);

    if (bm.isRSite(aid))
    {
        QS_DEF(Array<int>, rg);
        bm.getAllowedRGroups(aid, rg);

        if (rg.size() == 0)
            output.printf("R");
        else
            for (int i = 0; i < rg.size(); ++i)
            {
                if (i > 0)
                    output.printf(",");
                output.printf("R%i", rg[i]);
            }
    }
    else
    {
        if (!bm.isQueryMolecule())
            throw Error("Atom type %d not supported in non-queries", ad.queryLabel);

        if (ad.queryLabel == QueryMolecule::QUERY_ATOM_A)
        {
            output.printf("A");
        }
        else if (ad.queryLabel == QueryMolecule::QUERY_ATOM_X)
        {
            output.printf("X");
        }
        else if (ad.queryLabel == QueryMolecule::QUERY_ATOM_Q)
        {
            output.printf("Q");
        }
        else if (ad.queryLabel == QueryMolecule::QUERY_ATOM_STAR)
        {
            output.printf("*");
        }
        else if (ad.queryLabel == QueryMolecule::QUERY_ATOM_LIST || ad.queryLabel == QueryMolecule::QUERY_ATOM_NOTLIST)
        {
            QueryMolecule& qm = bm.asQueryMolecule();
            QueryMolecule::Atom& qa = qm.getAtom(aid);
            QueryMolecule::writeSmartsAtom(output, &qa, -1, -1, 0, false, false, bm.original_format);
        }
        else
        {
            throw Error("Query atom type %d not supported", ad.queryLabel);
        }
    }
}

bool MoleculeRenderInternal::_writeDelimiter(bool needDelimiter, Output& output)
{
    if (needDelimiter)
        output.printf(",");
    else
        output.printf("(");
    return true;
}

QueryMolecule::Atom* atomNodeInConjunction(QueryMolecule::Atom& qa, int type)
{
    if (qa.type != QueryMolecule::OP_AND)
        return NULL;
    for (int i = 0; i < qa.children.size(); ++i)
        if (qa.child(i)->type == type)
            return qa.child(i);
    return NULL;
}

void MoleculeRenderInternal::_writeQueryModifier(Output& output, int aid)
{
    QUERY_MOL_BEGIN(_mol);
    {
        bool needDelimiter = false;
        QueryMolecule::Atom& qa = qmol.getAtom(aid);

        if (qa.hasConstraint(QueryMolecule::ATOM_SUBSTITUENTS))
        {
            int subst = qmol.getAtomSubstCount(aid);
            needDelimiter = _writeDelimiter(needDelimiter, output);
            if (subst >= 0)
                output.printf("s%i", subst);
        }

        if (qa.hasConstraint(QueryMolecule::ATOM_SUBSTITUENTS_AS_DRAWN))
        {
            needDelimiter = _writeDelimiter(needDelimiter, output);
            output.printf("s*");
        }

        if (qa.hasConstraint(QueryMolecule::ATOM_RING_BONDS))
        {
            int ringBondCount = qmol.getAtomRingBondsCount(aid);
            needDelimiter = _writeDelimiter(needDelimiter, output);
            if (ringBondCount >= 0)
                output.printf("rb%i", ringBondCount);
        }

        if (qa.hasConstraint(QueryMolecule::ATOM_RING_BONDS_AS_DRAWN))
        {
            needDelimiter = _writeDelimiter(needDelimiter, output);
            output.printf("rb*");
        }

        if (qa.hasConstraint(QueryMolecule::ATOM_UNSATURATION))
        {
            needDelimiter = _writeDelimiter(needDelimiter, output);
            output.printf("u");
        }

        if (qa.hasConstraint(QueryMolecule::ATOM_TOTAL_H))
        {
            QueryMolecule::Atom* qc = atomNodeInConjunction(qa, QueryMolecule::ATOM_TOTAL_H);
            if (qc != NULL)
            {
                int totalH = qc->value_min;
                needDelimiter = _writeDelimiter(needDelimiter, output);
                output.printf("H%i", totalH);
            }
        }

        if (_ad(aid).fixed)
        {
            needDelimiter = _writeDelimiter(needDelimiter, output);
            output.printf("f");
        }

        if (needDelimiter)
            output.printf(")");

        if (_ad(aid).exactChange)
        {
            output.printf(".ext.");
        }
    }
    QUERY_MOL_END;
}

static void _expandBoundRect(AtomDesc& ad, const RenderItem& item)
{
    Vec2f min, max;
    min.diff(item.bbp, ad.pos);
    max.sum(min, item.bbsz);
    ad.boundBoxMin.min(min);
    ad.boundBoxMax.max(max);
}

int MoleculeRenderInternal::_findClosestBox(Vec2f& p, int aid, const Vec2f& sz, float mrg, int skip)
{
    const Vertex& vertex = _mol->getVertex(aid);
    const AtomDesc& ad = _ad(aid);
    float w2 = sz.x / 2 + mrg;
    float h2 = sz.y / 2 + mrg;

    if (vertex.degree() == 0)
    {
        p.set(0, -h2);
        p.add(ad.pos);
        return -1;
    }
    if (vertex.degree() == 1)
    {
        float offset = 0;
        const Vec2f& d = _getBondEnd(aid, vertex.neiBegin()).dir;
        float val = -1, dx = fabs(d.x), dy = fabs(d.y);
        if (dx > 0.01)
            if (val < 0 || val > w2 / dx)
                val = w2 / dx;
        if (dy > 0.01)
            if (val < 0 || val > h2 / dy)
                val = h2 / dy;
        if (val > offset)
            offset = val;
        p.lineCombin(ad.pos, d, -offset);
        return vertex.neiBegin();
    }
    int iMin = -1;
    for (int i = vertex.neiBegin(); i < vertex.neiEnd(); i = vertex.neiNext(i))
    {
        if (skip == i)
            continue;
        Vec2f q;
        const BondEnd& lbe = _getBondEnd(aid, i);
        const BondEnd& rbe = _be(lbe.rnei);
        const BondDescr& lbd = _bd(lbe.bid);
        const BondDescr& rdb = _bd(rbe.bid);

        // correction of positioning according to bond types
        float turn = atan(_settings.bondSpace / lbd.length);
        float leftShift = 0, rightShift = 0;
        bool leftBondCentered = _be(lbd.be1).centered && _be(lbd.be2).centered;
        bool leftTurn = false, rightTurn = false;
        bool leftIsSourceEnd = (lbd.beg == aid);
        bool leftBondOrientedInwards = (leftIsSourceEnd == lbd.lineOnTheRight);
        bool leftIsStereo = lbd.stereodir > 0;
        bool rightBondCentered = _be(rdb.be1).centered && _be(rdb.be2).centered;
        bool rightIsSourceEnd = (rdb.beg == aid);
        bool rightIsStereo = rdb.stereodir > 0;
        bool rightBondOrientedInwards = (rightIsSourceEnd == !rdb.lineOnTheRight);

        if (lbd.type == BOND_DOUBLE)
        {
            if (leftBondCentered)
                leftShift = _settings.bondSpace;
            else if (leftBondOrientedInwards)
                leftShift = 2 * _settings.bondSpace;
        }
        else if (lbd.type == BOND_TRIPLE)
            leftShift = 2 * _settings.bondSpace;
        else if (leftIsStereo)
        {
            if (leftIsSourceEnd)
                leftTurn = true;
            else
                leftShift = _settings.bondSpace;
        }

        if (rdb.type == BOND_DOUBLE)
        {
            if (rightBondCentered)
                rightShift = _settings.bondSpace;
            else if (rightBondOrientedInwards)
                rightShift = 2 * _settings.bondSpace;
        }
        else if (rdb.type == BOND_TRIPLE)
            rightShift = 2 * _settings.bondSpace;
        else if (rightIsStereo)
        {
            if (rightIsSourceEnd)
                rightTurn = true;
            else
                rightShift = _settings.bondSpace;
        }

        Vec2f leftDir, rightDir;
        leftDir.copy(lbe.dir);
        rightDir.copy(rbe.dir);
        if (leftTurn)
            leftDir.rotateL(turn);
        if (rightTurn)
            rightDir.rotateL(-turn);
        Vec2f origin;
        float si = Vec2f::cross(leftDir, rightDir);
        float co = Vec2f::dot(leftDir, rightDir);
        float ang = atan2(si, co);
        if (ang < 0)
            ang += 2 * (float)M_PI;

        float factor = fabs(sin(ang / 2));
        Vec2f rightNorm(rightDir), leftNorm(leftDir);
        rightNorm.rotateL((float)M_PI / 2);
        leftNorm.rotateL(-(float)M_PI / 2);

        float rightOffset = 0, leftOffset = 0;
        rightOffset = w2 * fabs(leftNorm.x) + h2 * fabs(leftNorm.y);
        leftOffset = w2 * fabs(rightNorm.x) + h2 * fabs(rightNorm.y);

        float t = std::max(leftShift + rightOffset, leftOffset + rightShift) / factor;
        Vec2f median(rightDir);
        median.rotateL(ang / 2);
        q.addScaled(median, t);
        if (iMin < 0 || q.lengthSqr() < p.lengthSqr())
        {
            iMin = i;
            p.copy(q);
        }
    }
    p.add(ad.pos);
    return iMin;
}

int MoleculeRenderInternal::_findClosestCircle(Vec2f& p, int aid, float radius, int skip)
{
    const Vertex& vertex = _mol->getVertex(aid);
    const AtomDesc& ad = _ad(aid);
    if (vertex.degree() == 0)
    {
        p.copy(ad.pos);
        p.y -= radius;
        return -1;
    }
    if (vertex.degree() == 1)
    {
        p.lineCombin(ad.pos, _getBondEnd(aid, vertex.neiBegin()).dir, -radius);
        return vertex.neiBegin();
    }
    int iMin = -1;
    for (int i = vertex.neiBegin(); i < vertex.neiEnd(); i = vertex.neiNext(i))
    {
        if (skip == i)
            continue;
        Vec2f q;
        const BondEnd& rbe = _be(i);
        const BondEnd& lbe = _be(rbe.lnei);
        ;
        const BondDescr& rbd = _bd(rbe.bid);
        const BondDescr& lbd = _bd(lbe.bid);

        // correction of positioning according to bond types
        float turn = atan(_settings.bondSpace / rbd.length);
        float rightShift = 0, leftShift = 0;
        bool rightBondCentered = _be(rbd.be1).centered && _be(rbd.be2).centered;
        bool rightTurn = false, leftTurn = false;
        bool rightIsSourceEnd = rbd.beg == aid;
        bool rightBondOrientationLeft = !rbd.lineOnTheRight;
        bool rightBondOrientedInwards = (rightIsSourceEnd && rightBondOrientationLeft) || (!rightIsSourceEnd && !rightBondOrientationLeft);
        bool rightIsStereo = rbd.stereodir > 0;
        bool leftBondCentered = _getBondEnd(aid, lbd.be1).centered && _getBondEnd(aid, lbd.be2).centered;
        bool leftIsSourceEnd = lbd.beg == aid;
        bool leftIsStereo = lbd.stereodir > 0;
        bool leftBondOrientationRight = lbd.lineOnTheRight;
        bool leftBondOrientedInwards = (leftIsSourceEnd && leftBondOrientationRight) || (!leftIsSourceEnd && !leftBondOrientationRight);

        if (rbd.type == BOND_DOUBLE)
        {
            if (rightBondCentered)
                rightShift = _settings.bondSpace;
            else if (rightBondOrientedInwards)
                rightShift = 2 * _settings.bondSpace;
        }
        else if (rbd.type == BOND_TRIPLE)
            rightShift = 2 * _settings.bondSpace;
        else if (rightIsStereo)
        {
            if (rightIsSourceEnd)
                rightTurn = true;
            else
                rightShift = _settings.bondSpace;
        }

        if (lbd.type == BOND_DOUBLE)
        {
            if (leftBondCentered)
                leftShift = _settings.bondSpace;
            else if (leftBondOrientedInwards)
                leftShift = 2 * _settings.bondSpace;
        }
        else if (lbd.type == BOND_TRIPLE)
            leftShift = 2 * _settings.bondSpace;
        else if (leftIsStereo)
        {
            if (leftIsSourceEnd)
                leftTurn = true;
            else
                leftShift = _settings.bondSpace;
        }

        Vec2f rightDir, leftDir;
        rightDir.copy(rbe.dir);
        leftDir.copy(lbe.dir);
        if (rightTurn)
            rightDir.rotate(turn);
        if (leftTurn)
            leftDir.rotate(-turn);
        Vec2f origin;
        float si = Vec2f::cross(rightDir, leftDir);
        float co = Vec2f::dot(rightDir, leftDir);
        float ang = atan2(si, co);
        if (ang < 0)
            ang += (float)(2 * M_PI);

        float factor = std::max(fabsf(si), 0.01f);
        if (rightShift > 0)
            origin.addScaled(leftDir, rightShift / factor);
        if (leftShift > 0)
            origin.addScaled(rightDir, leftShift / factor);

        q.copy(rightDir);
        q.rotate(ang / 2);

        float dst = radius / (float)sin2c(Vec2f::dot(rightDir, leftDir));
        q.scale(dst);
        q.add(origin);

        if (iMin < 0 || q.lengthSqr() < p.lengthSqr())
        {
            iMin = i;
            p.copy(q);
        }
        q.add(ad.pos);
        //_cw.setLineWidth(_settings.unit);
        //_cw.setSingleSource(CWC_BLUE);
        //_cw.drawCircle(q, radius);
    }
    p.add(ad.pos);
    return iMin;
}

enum CHARCAT
{
    DIGIT = 0,
    LETTER,
    TAG_SUPERSCRIPT,
    TAG_SUBSCRIPT,
    TAG_NORMAL,
    SIGN,
    WHITESPACE
};
enum SCRIPT
{
    MAIN,
    SUB,
    SUPER
};

void MoleculeRenderInternal::_preparePseudoAtom(int aid, int color, bool highlighted)
{
    AtomDesc& ad = _ad(aid);
    const char* str = ad.pseudo.ptr();

    int cnt = 0;
    CHARCAT a = WHITESPACE, b = WHITESPACE;
    int i0 = 0, i1;
    SCRIPT script = MAIN, newscript = MAIN;
    int len = (int)strlen(str);
    GraphItem::TYPE signType = GraphItem::DOT;
    // TODO: replace remembering item ids and shifting each of them with single offset value for an atom
    Array<int> tis, gis;

    TextItem fake;
    fake.fontsize = FONT_SIZE_LABEL;
    fake.text.push('C');
    fake.text.push((char)0);
    _cw.setTextItemSize(fake, ad.pos);
    float xpos = fake.bbp.x, width = fake.bbsz.x, offset = _settings.unit / 2, totalwdt = 0, upshift = -0.6f, downshift = 0.2f, space = width / 2;
    ad.ypos = fake.bbp.y;
    ad.height = fake.bbsz.y;
    ad.leftMargin = ad.rightMargin = xpos;

    if (ad.pseudoAtomStringVerbose)
    {
        int id = _pushTextItem(ad, RenderItem::RIT_PSEUDO, color, highlighted);
        tis.push(id);
        TextItem& item = _data.textitems[id];
        item.fontsize = FONT_SIZE_ATTR;
        item.text.copy(str, len);
        item.text.push((char)0);
        _cw.setTextItemSize(item);
        item.bbp.set(xpos, ad.ypos);
        _expandBoundRect(ad, item);
        totalwdt += item.bbsz.x;
    }
    else
    {
        for (int i = 0; i <= len; ++i)
        {
            i1 = i;
            a = b;

            bool tag = false;
            {
                unsigned char c = (i == len ? ' ' : str[i]);
                if (std::isspace(c))
                {
                    b = WHITESPACE;
                }
                else if (std::isdigit(c))
                {
                    b = DIGIT;
                }
                else if (c == '+' || c == '-')
                {
                    b = SIGN, signType = ((c == '+') ? GraphItem::PLUS : GraphItem::MINUS);
                }
                else if (c == '\\' && i < len - 1 && str[i + 1] == 'S')
                {
                    b = TAG_SUPERSCRIPT, ++i, tag = true;
                }
                else if (c == '\\' && i < len - 1 && str[i + 1] == 's')
                {
                    b = TAG_SUBSCRIPT, ++i, tag = true;
                }
                else if (c == '\\' && i < len - 1 && str[i + 1] == 'n')
                {
                    b = TAG_NORMAL, ++i, tag = true;
                }
                else
                {
                    b = LETTER;
                }
            }

            {
                bool aTag = a == TAG_SUPERSCRIPT || a == TAG_SUBSCRIPT || a == TAG_NORMAL;
                if (b == TAG_SUPERSCRIPT)
                {
                    newscript = SUPER;
                }
                else if (b == TAG_SUBSCRIPT)
                {
                    newscript = SUB;
                }
                else if (b == TAG_NORMAL)
                {
                    newscript = MAIN;
                }
                else if ((b == WHITESPACE && a != WHITESPACE) || (b != WHITESPACE && a == WHITESPACE) || (b == LETTER && !aTag))
                {
                    newscript = MAIN;
                }
                else if (b == DIGIT && a == SIGN)
                {
                    newscript = script;
                }
                else if (b == DIGIT && a != DIGIT && !aTag)
                {
                    newscript = ((a == LETTER) ? SUB : MAIN);
                }
                else if (b == SIGN)
                {
                    if (a == LETTER || a == DIGIT)
                    {
                        newscript = SUPER;
                    }
                }
                else if (a == SIGN && script == SUPER)
                {
                    newscript = MAIN;
                }
                else
                {
                    continue;
                }
            }

            if (i1 > i0)
            {
                if (a == SIGN && script == SUPER)
                {
                    int id = _pushGraphItem(ad, RenderItem::RIT_CHARGESIGN, color, highlighted);
                    gis.push(id);
                    GraphItem& sign = _data.graphitems[id];
                    _cw.setGraphItemSizeSign(sign, signType);

                    totalwdt += offset;
                    sign.bbp.set(xpos + totalwdt, ad.ypos + ad.height - sign.bbsz.y + upshift * ad.height);
                    _expandBoundRect(ad, sign);
                    totalwdt += sign.bbsz.x;
                }
                else if (a == WHITESPACE)
                {
                    totalwdt += space * (i1 - i0);
                }
                else
                {
                    float shift = (script == SUB) ? downshift : ((script == SUPER) ? upshift : 0);
                    int id = _pushTextItem(ad, RenderItem::RIT_PSEUDO, color, highlighted);
                    tis.push(id);
                    TextItem& item = _data.textitems[id];
                    item.fontsize = (script == MAIN) ? FONT_SIZE_LABEL : FONT_SIZE_ATTR;
                    item.text.copy(str + i0, i1 - i0);
                    item.text.push((char)0);
                    _cw.setTextItemSize(item);

                    if (cnt > 0)
                        totalwdt += offset;
                    item.bbp.set(xpos + totalwdt, ad.ypos + ad.height - item.relpos.y + shift * ad.height);
                    _expandBoundRect(ad, item);
                    totalwdt += item.bbsz.x;
                }
                cnt++;
            }
            script = newscript;
            i0 = i + (tag ? 1 : 0);
        }
    }
    ad.rightMargin += totalwdt;
    if (ad.hydroPos == HYDRO_POS_LEFT)
    {
        float dx = totalwdt - width;
        for (int i = 0; i < tis.size(); ++i)
            _data.textitems[tis[i]].bbp.x -= dx;
        for (int i = 0; i < gis.size(); ++i)
            _data.graphitems[gis[i]].bbp.x -= dx;
        ad.leftMargin -= dx;
        ad.rightMargin -= dx;
    }
}

template <class Owner>
MoleculeRenderInternal::ChargeItems MoleculeRenderInternal::_pushChargeItems(Owner& owner, int charge, float size_factor, int color, bool highlighted)
{
    ChargeItems items;
    if (abs(charge) != 1)
    {
        items.value = _pushTextItem(owner, RenderItem::RIT_CHARGEVAL, color, highlighted);
        TextItem& value = _data.textitems[items.value];
        value.fontsize = FONT_SIZE_ATTR;
        value.size = size_factor * _cw.fontGetSize(FONT_SIZE_ATTR);
        bprintf(value.text, "%i", abs(charge));
        _cw.setTextItemSize(value);
    }

    items.sign = _pushGraphItem(owner, RenderItem::RIT_CHARGESIGN, color, highlighted);
    GraphItem& sign = _data.graphitems[items.sign];
    sign.sizeFactor = size_factor;
    _cw.setGraphItemSizeSign(sign, charge > 0 ? GraphItem::PLUS : GraphItem::MINUS);
    return items;
}

template <class Owner>
MoleculeRenderInternal::RadicalItems MoleculeRenderInternal::_pushRadicalItems(Owner& owner, int radical, float size_factor, int color, bool highlighted)
{
    RadicalItems items;
    items.first = _pushGraphItem(owner, RenderItem::RIT_RADICAL, color, highlighted);
    if (radical != RADICAL_DOUBLET)
        items.second = _pushGraphItem(owner, RenderItem::RIT_RADICAL, color, highlighted);

    GraphItem& first = _data.graphitems[items.first];
    first.sizeFactor = size_factor;
    if (radical == RADICAL_DOUBLET)
    {
        _cw.setGraphItemSizeDot(first);
        return items;
    }

    GraphItem& second = _data.graphitems[items.second];
    second.sizeFactor = size_factor;
    if (radical == RADICAL_SINGLET)
    {
        _cw.setGraphItemSizeDot(first);
        _cw.setGraphItemSizeDot(second);
        items.spacing = size_factor * _settings.radicalTopDistDot;
    }
    else // RADICAL_TRIPLET
    {
        _cw.setGraphItemSizeCap(first);
        _cw.setGraphItemSizeCap(second);
        items.spacing = size_factor * _settings.radicalTopDistCap;
    }
    return items;
}

void MoleculeRenderInternal::_prepareChargeLabel(int aid, int color, bool highlighted)
{
    AtomDesc& ad = _ad(aid);
    BaseMolecule& bm = *_mol;

    int charge = bm.getAtomCharge(aid);
    if (charge != CHARGE_UNKNOWN && charge != 0)
    {
        ad.rightMargin += _settings.labelInternalOffset;
        const ChargeItems items = _pushChargeItems(ad, charge, ATOM_LABEL_SIZE_FACTOR, color, highlighted);
        if (items.value >= 0)
        {
            TextItem& itemChargeValue = _data.textitems[items.value];
            itemChargeValue.bbp.set(ad.rightMargin, ad.ypos + _settings.upperIndexShift * ad.height);
            _expandBoundRect(ad, itemChargeValue);
            ad.rightMargin += itemChargeValue.bbsz.x;
        }

        GraphItem& itemChargeSign = _data.graphitems[items.sign];
        itemChargeSign.bbp.set(ad.rightMargin, ad.ypos + _settings.upperIndexShift * ad.height);
        _expandBoundRect(ad, itemChargeSign);
        ad.rightMargin += itemChargeSign.bbsz.x;
    }
}

void MoleculeRenderInternal::_prepareCIPLabel(const int aid, int& skip)
{
    BaseMolecule& bm = *_mol;
    const CIPDesc cip = bm.getAtomCIP(aid);

    if (bm.getShowAtomCIP(aid) && cip != CIPDesc::UNKNOWN && cip != CIPDesc::NONE)
    {
        AtomDesc& ad = _ad(aid);
        int tiCIP = _pushTextItem(ad, RenderItem::RIT_CIP, CWC_BASE, false);

        TextItem& itemCIP = _data.textitems[tiCIP];
        itemCIP.fontsize = FONT_SIZE_ATTR;
        bprintf(itemCIP.text, "(%s)", CIPToString(cip).c_str());
        _cw.setTextItemSize(itemCIP);

        if (ad.showLabel)
        {
            ad.leftMargin -= itemCIP.bbsz.x + _settings.labelInternalOffset;
            itemCIP.bbp.set(ad.leftMargin, ad.ypos + _settings.lowerIndexShift * ad.height);
        }
        else
        {
            Vec2f p;
            skip = _findClosestBox(p, aid, itemCIP.bbsz, _settings.unit, skip);

            p.addScaled(itemCIP.bbsz, -0.5);
            itemCIP.bbp.copy(p);
        }
        _expandBoundRect(ad, itemCIP);
    }
}

void MoleculeRenderInternal::_prepareLabelText(int aid)
{
    AtomDesc& ad = _ad(aid);
    BaseMolecule& bm = *_mol;
    ad.boundBoxMin.set(0, 0);
    ad.boundBoxMax.set(0, 0);

    if (ad.label == 0 && ad.hydroPos == HYDRO_POS_LEFT)
    {
        _reverseLabelText(aid);
    }

    int color = ad.color;
    bool highlighted = _vertexIsHighlighted(aid);

    int tilabel = -1, tihydro = -1, tiHydroIndex = -1, tiValence = -1, tiIsotope = -1, tiindex = -1;
    int giChargeSign = -1;
    ad.rightMargin = ad.leftMargin = ad.ypos = ad.height = 0;
    int isotope = bm.getAtomIsotope(aid);

    if (ad.type == AtomDesc::TYPE_PSEUDO)
    {
        _preparePseudoAtom(aid, CWC_BASE, highlighted);

        bool chargeSignAdded = false;
        for (auto i = 0; i < _data.graphitems.size(); i++)
        {
            if (_data.graphitems[i].ritype == RenderItem::RIT_CHARGESIGN)
            {
                chargeSignAdded = true;
                break;
            }
        }

        if (!chargeSignAdded)
        {
            _prepareChargeLabel(aid, color, highlighted);
        }
    }
    else if (ad.showLabel)
    {
        tilabel = _pushTextItem(ad, RenderItem::RIT_LABEL, color, highlighted);
        {
            TextItem& label = _data.textitems[tilabel];
            label.fontsize = FONT_SIZE_LABEL;
            ArrayOutput output(label.text);
            if (ad.type == AtomDesc::TYPE_REGULAR)
                if (ad.label == ELEM_H && isotope == DEUTERIUM)
                    output.printf("D");
                else if (ad.label == ELEM_H && isotope == TRITIUM)
                    output.printf("T");
                else
                    output.printf(Element::toString(ad.label));
            else if (ad.type == AtomDesc::TYPE_QUERY)
                _writeQueryAtomToString(output, aid);
            else
                throw Error("Neither label nor query atom type available");

            _writeQueryModifier(output, aid);
            output.writeChar(0);

            _cw.setTextItemSize(label, ad.pos);
            _expandBoundRect(ad, label);
            ad.rightMargin = label.bbp.x + label.bbsz.x;
            ad.leftMargin = label.bbp.x;
            ad.ypos = label.bbp.y;
            ad.height = label.bbsz.y;
        }

        if (bm.isRSite(aid) && bm.getVertex(aid).degree() > 1)
        {
            // show indices for attachment bonds
            const Vertex& v = bm.getVertex(aid);
            ad.rSiteAttachmentIndexBegin = _data.rSiteAttachmentIndices.size();

            // Check if there are indices for attachment bonds
            bool hasAttachmentIndex = false, hasNoAttachmentIndex = false;
            for (int k = v.neiBegin(), j = 0; k < v.neiEnd(); k = v.neiNext(k), ++j)
            {
                int apIdx = bm.getRSiteAttachmentPointByOrder(aid, j);
                hasAttachmentIndex |= (apIdx != -1);
                hasNoAttachmentIndex |= (apIdx == -1);
            }
            if (hasAttachmentIndex && hasNoAttachmentIndex)
                throw Error("RSite %d is invalid: some attachments indices are specified and some are not");
            if (hasNoAttachmentIndex)
                ad.rSiteAttachmentIndexCount = 0;
            else
            {
                ad.rSiteAttachmentIndexCount = v.degree();

                for (int k = v.neiBegin(), j = 0; k < v.neiEnd(); k = v.neiNext(k), ++j)
                {
                    int apIdx = bm.getRSiteAttachmentPointByOrder(aid, j);
                    int i = v.findNeiVertex(apIdx);
                    BondEnd& be = _getBondEnd(aid, i);
                    int tii = _pushTextItem(ad, RenderItem::RIT_ATTACHMENTPOINT, CWC_BASE, false);
                    TextItem& ti = _data.textitems[tii];
                    RenderItemRSiteAttachmentIndex& item = _data.rSiteAttachmentIndices.push();
                    item.number = j + 1;
                    item.radius = 0.7f * _settings.fzz[FONT_SIZE_RSITE_ATTACHMENT_INDEX];
                    item.bbsz.set(2 * item.radius, 2 * item.radius);
                    item.bbp = ad.pos;
                    item.color = CWC_BASE;
                    item.highlighted = false;
                    item.noBondOffset = true;
                    bprintf(ti.text, "%d", item.number);
                    ti.fontsize = FONT_SIZE_RSITE_ATTACHMENT_INDEX;
                    ti.noBondOffset = true;
                    _cw.setTextItemSize(ti, ad.pos);
                    TextItem& label = _data.textitems[tilabel];
                    // this is just an upper bound, it won't be used
                    float shift = item.bbsz.length() + label.bbsz.length();
                    // one of the next conditions should be satisfied
                    if (fabs(be.dir.x) > 1e-3)
                        shift = std::min(shift, (item.bbsz.x + label.bbsz.x) / 2.f / fabsf(be.dir.x));
                    if (fabs(be.dir.y) > 1e-3)
                        shift = std::min(shift, (item.bbsz.y + label.bbsz.y) / 2.f / fabsf(be.dir.y));
                    shift += _settings.unit;
                    item.bbp.addScaled(be.dir, shift);
                    ti.bbp.addScaled(be.dir, shift);
                    be.offset = shift + item.radius;
                }
            }
        }

        // isotope
        if (isotope > 0 && (ad.label != ELEM_H || isotope > 3 || isotope < 2))
        {
            tiIsotope = _pushTextItem(ad, RenderItem::RIT_ISOTOPE, color, highlighted);

            TextItem& itemIsotope = _data.textitems[tiIsotope];
            itemIsotope.fontsize = FONT_SIZE_ATTR;
            bprintf(itemIsotope.text, "%i", isotope);
            _cw.setTextItemSize(itemIsotope);

            ad.leftMargin -= _settings.labelInternalOffset + itemIsotope.bbsz.x;
            itemIsotope.bbp.set(ad.leftMargin, ad.ypos + _settings.upperIndexShift * ad.height);
            _expandBoundRect(ad, itemIsotope);
        }

        // hydrogen drawing
        ad.showHydro = false;
        if (!bm.isQueryMolecule())
        {
            int implicit_h = 0;

            if (!bm.isRSite(aid) && !bm.isPseudoAtom(aid) && !bm.isTemplateAtom(aid))
                implicit_h = bm.asMolecule().getImplicitH_NoThrow(aid, 0);

            if (implicit_h > 0 && _opt.implHVisible)
            {
                ad.showHydro = true;

                tihydro = _pushTextItem(ad, RenderItem::RIT_HYDROGEN, color, highlighted);
                Vec2f hydrogenGroupSz;
                {

                    TextItem& itemHydrogen = _data.textitems[tihydro];
                    itemHydrogen.fontsize = FONT_SIZE_LABEL;
                    bprintf(itemHydrogen.text, "H");
                    _cw.setTextItemSize(itemHydrogen, ad.pos);
                    hydrogenGroupSz.x = itemHydrogen.bbsz.x + _settings.labelInternalOffset;
                    hydrogenGroupSz.y = itemHydrogen.bbsz.y;
                }

                if (implicit_h > 1)
                {
                    tiHydroIndex = _pushTextItem(ad, RenderItem::RIT_HYDROINDEX, color, highlighted);

                    TextItem& itemHydroIndex = _data.textitems[tiHydroIndex];
                    TextItem& itemHydrogen = _data.textitems[tihydro];
                    itemHydroIndex.fontsize = FONT_SIZE_ATTR;
                    bprintf(itemHydroIndex.text, "%i", implicit_h);
                    _cw.setTextItemSize(itemHydroIndex, ad.pos);
                    hydrogenGroupSz.x += itemHydroIndex.bbsz.x + _settings.labelInternalOffset;
                    hydrogenGroupSz.y = std::max(hydrogenGroupSz.y, _settings.lowerIndexShift * itemHydrogen.bbsz.y + itemHydroIndex.bbsz.y);
                }

                // take new reference, old one may be corrupted after adding 'tiHydroIndex'
                TextItem& itemHydrogen = _data.textitems[tihydro];
                if (ad.hydroPos == HYDRO_POS_LEFT)
                {
                    ad.leftMargin -= hydrogenGroupSz.x;
                    itemHydrogen.bbp.set(ad.leftMargin, ad.ypos);
                }
                else if (ad.hydroPos == HYDRO_POS_RIGHT)
                {
                    ad.rightMargin += _settings.labelInternalOffset;
                    itemHydrogen.bbp.set(ad.rightMargin, ad.ypos);
                    ad.rightMargin += hydrogenGroupSz.x;
                }
                else if (ad.hydroPos == HYDRO_POS_UP)
                {
                    itemHydrogen.bbp.y = ad.pos.y + ad.boundBoxMin.y - hydrogenGroupSz.y - _settings.unit;
                }
                else if (ad.hydroPos == HYDRO_POS_DOWN)
                {
                    itemHydrogen.bbp.y = ad.pos.y + ad.boundBoxMax.y + _settings.unit;
                }
                else
                {
                    throw Error("hydrogen position value invalid");
                }
                _expandBoundRect(ad, itemHydrogen);
                if (tiHydroIndex > 0)
                {
                    _data.textitems[tiHydroIndex].bbp.set(itemHydrogen.bbp.x + itemHydrogen.bbsz.x + _settings.labelInternalOffset,
                                                          itemHydrogen.bbp.y + _settings.lowerIndexShift * itemHydrogen.bbsz.y);
                    _expandBoundRect(ad, _data.textitems[tiHydroIndex]);
                }
            }
        }

        // charge
        _prepareChargeLabel(aid, color, highlighted);

        // valence
        int valence = bm.getExplicitValence(aid);

        if (_opt.showValences && valence >= 0)
        {
            tiValence = _pushTextItem(ad, RenderItem::RIT_VALENCE, color, highlighted);

            TextItem& itemValence = _data.textitems[tiValence];
            itemValence.fontsize = FONT_SIZE_ATTR;
            bprintf(itemValence.text, _valenceText(valence));
            _cw.setTextItemSize(itemValence);

            ad.rightMargin += _settings.labelInternalOffset;
            itemValence.bbp.set(ad.rightMargin, ad.ypos + _settings.upperIndexShift * ad.height);
            _expandBoundRect(ad, itemValence);
            ad.rightMargin += itemValence.bbsz.x;
        }

        // radical
        int radical = -1;

        if (!bm.isRSite(aid) && !bm.isPseudoAtom(aid) && !bm.isTemplateAtom(aid))
            radical = bm.getAtomRadical_NoThrow(aid, -1);

        if (radical > 0)
        {
            const TextItem& label = _data.textitems[tilabel];
            Vec2f ltc(label.bbp);
            const RadicalItems items = _pushRadicalItems(ad, radical, ATOM_LABEL_SIZE_FACTOR, color, highlighted);

            if (items.second < 0)
            {
                GraphItem& itemRadical = _data.graphitems[items.first];

                if (!(ad.showHydro && ad.hydroPos == HYDRO_POS_RIGHT) && giChargeSign < 0 && tiValence < 0)
                {
                    ltc.x += label.bbsz.x + _settings.radicalRightOffset;
                    ltc.y += _settings.radicalRightVertShift * ad.height;
                    itemRadical.bbp.copy(ltc);
                }
                else
                {
                    ltc.x += label.bbsz.x / 2 - itemRadical.bbsz.x / 2;
                    ltc.y -= itemRadical.bbsz.y + _settings.radicalTopOffset;
                    itemRadical.bbp.copy(ltc);
                }
                _expandBoundRect(ad, itemRadical);
            }
            else
            {
                GraphItem& itemRadical1 = _data.graphitems[items.first];
                GraphItem& itemRadical2 = _data.graphitems[items.second];

                ltc.y -= itemRadical1.bbsz.y + _settings.radicalTopOffset;
                ltc.x += label.bbsz.x / 2 - items.spacing / 2 - itemRadical1.bbsz.x;
                itemRadical1.bbp.copy(ltc);
                ltc.x += items.spacing + itemRadical1.bbsz.x;
                itemRadical2.bbp.copy(ltc);
                _expandBoundRect(ad, itemRadical1);
                _expandBoundRect(ad, itemRadical2);
            }
        }
    }

    int bondEndRightToStereoGroupLabel = -1;
    // prepare stereogroup labels
    if ((ad.stereoGroupType > 0 && ad.stereoGroupType != MoleculeStereocenters::ATOM_ANY) || ad.inversion == STEREO_INVERTS || ad.inversion == STEREO_RETAINS)
    {
        int tiStereoGroup = _pushTextItem(ad, RenderItem::RIT_STEREOGROUP, CWC_BASE, false);

        TextItem& itemStereoGroup = _data.textitems[tiStereoGroup];
        itemStereoGroup.fontsize = FONT_SIZE_ATTR;
        ArrayOutput itemOutput(itemStereoGroup.text);
        if (ad.stereoGroupType > 0 && ad.stereoGroupType != MoleculeStereocenters::ATOM_ANY)
        {
            const char* stereoGroupText = _getStereoGroupText(ad.stereoGroupType);
            itemOutput.printf("%s", stereoGroupText);
            if (ad.stereoGroupType != MoleculeStereocenters::ATOM_ABS)
                itemOutput.printf("%i", ad.stereoGroupNumber);
        }
        if (ad.inversion == STEREO_INVERTS || ad.inversion == STEREO_RETAINS)
        {
            if (itemOutput.tell() > 0)
                itemOutput.printf(",");
            itemOutput.printf("%s", ad.inversion == STEREO_INVERTS ? "Inv" : "Ret");
        }
        itemOutput.writeChar(0);

        _cw.setTextItemSize(itemStereoGroup);

        if (ad.showLabel)
        {
            // label visible - put stereo group label on the over or under the label
            const Vertex& v = bm.getVertex(aid);
            float vMin = 0, vMax = 0;
            for (int i = v.neiBegin(); i < v.neiEnd(); i = v.neiNext(i))
            {
                float y = _getBondEnd(aid, i).dir.y;
                if (y > vMax)
                    vMax = y;
                if (y < vMin)
                    vMin = y;
            }
            if (vMax > -vMin)
                itemStereoGroup.bbp.set(ad.pos.x - itemStereoGroup.bbsz.x / 2,
                                        ad.pos.y + ad.boundBoxMin.y - itemStereoGroup.bbsz.y - _settings.stereoGroupLabelOffset);
            else
                itemStereoGroup.bbp.set(ad.pos.x - itemStereoGroup.bbsz.x / 2, ad.pos.y + ad.boundBoxMax.y + _settings.stereoGroupLabelOffset);
        }
        else
        {
            // label hidden - position stereo group label independently
            Vec2f p;
            bondEndRightToStereoGroupLabel = _findClosestBox(p, aid, itemStereoGroup.bbsz, _settings.unit);

            p.addScaled(itemStereoGroup.bbsz, -0.5);
            itemStereoGroup.bbp.copy(p);
        }
        _expandBoundRect(ad, itemStereoGroup);
    }

    // CIP
    if (_opt.showCIPLabels)
    {
        _prepareCIPLabel(aid, bondEndRightToStereoGroupLabel);
    }

    // prepare AAM labels
    if (ad.aam > 0)
    {
        int tiAAM = _pushTextItem(ad, RenderItem::RIT_AAM, CWC_BASE, false);

        TextItem& itemAAM = _data.textitems[tiAAM];
        itemAAM.fontsize = FONT_SIZE_ATTR;
        bprintf(itemAAM.text, "%i", abs(ad.aam));
        _cw.setTextItemSize(itemAAM);

        if (ad.showLabel)
        {
            ad.leftMargin -= itemAAM.bbsz.x + _settings.labelInternalOffset;
            itemAAM.bbp.set(ad.leftMargin, ad.ypos + _settings.lowerIndexShift * ad.height);
        }
        else
        {
            Vec2f p;
            _findClosestBox(p, aid, itemAAM.bbsz, _settings.unit, bondEndRightToStereoGroupLabel);

            p.addScaled(itemAAM.bbsz, -0.5);
            itemAAM.bbp.copy(p);
        }
        _expandBoundRect(ad, itemAAM);
    }

    // prepare R-group attachment point labels
    QS_DEF(Array<float>, angles);
    QS_DEF(Array<int>, split);
    QS_DEF(Array<int>, rGroupAttachmentIndices);

    if (ad.isRGroupAttachmentPoint)
    {
        // collect the angles between adjacent bonds
        const Vertex& v = bm.getVertex(aid);
        angles.clear();
        split.clear();
        if (v.degree() != 0)
        {
            for (int i = v.neiBegin(); i < v.neiEnd(); i = v.neiNext(i))
            {
                float a = _getBondEnd(aid, i).lang;
                angles.push(a);
                split.push(1);
            }
        }
        // collect attachment point indices
        rGroupAttachmentIndices.clear();
        bool multipleAttachmentPoints = _mol->attachmentPointCount() > 1;
        for (int i = 1; i <= _mol->attachmentPointCount(); ++i)
            for (int j = 0, k; (k = _mol->getAttachmentPoint(i, j)) >= 0; ++j)
                if (k == aid)
                    rGroupAttachmentIndices.push(i);
        if (v.degree() != 0)
        {
            for (int j = 0; j < rGroupAttachmentIndices.size(); ++j)
            {
                int i0 = -1;
                for (int i = 0; i < angles.size(); ++i)
                    if (i0 < 0 || angles[i] / (split[i] + 1) > angles[i0] / (split[i0] + 1))
                        i0 = i;
                split[i0]++;
            }
        }
        // arrange the directions of the attachment points
        QS_DEF(Array<Vec2f>, attachmentDirection);
        attachmentDirection.clear();
        if (v.degree() == 0)
        {
            // if no adjacent bonds present
            if (rGroupAttachmentIndices.size() == 1)
                attachmentDirection.push().set(0, -1);
            else if (rGroupAttachmentIndices.size() == 2)
            {
                attachmentDirection.push().set(cos((float)M_PI / 6), -sin((float)M_PI / 6));
                attachmentDirection.push().set(cos(5 * (float)M_PI / 6), -sin(5 * (float)M_PI / 6));
            }
            else
            {
                for (int j = 0; j < rGroupAttachmentIndices.size(); ++j)
                {
                    float a = j * 2 * (float)M_PI / rGroupAttachmentIndices.size();
                    attachmentDirection.push().set(cos(a), sin(a));
                }
            }
        }
        else
        {
            // split the angles
            for (int i = 0; i < split.size(); ++i)
            {
                angles[i] /= split[i];
            }
            for (int j = 0; j < rGroupAttachmentIndices.size(); ++j)
            {
                int i0 = -1, n = v.neiBegin();
                for (int i = 0; i < split.size(); ++i, n = v.neiNext(n))
                {
                    if (split[i] > 1)
                    {
                        i0 = i;
                        break;
                    }
                }
                if (i0 < 0)
                    throw Error("Error while arranging attachment points");
                Vec2f d;
                d.copy(_getBondEnd(aid, n).dir);
                d.rotateL(angles[i0] * (--split[i0]));
                attachmentDirection.push(d);
            }
        }
        // create the attachment point items
        ad.attachmentPointBegin = _data.attachmentPoints.size();
        ad.attachmentPointCount = rGroupAttachmentIndices.size();
        for (int j = 0; j < rGroupAttachmentIndices.size(); ++j)
        {
            RenderItemAttachmentPoint& attachmentPoint = _data.attachmentPoints.push();
            float offset = std::min(std::max(_getBondOffset(aid, ad.pos, attachmentDirection[j], _settings.unit), 0.f), 0.4f);
            attachmentPoint.dir.copy(attachmentDirection[j]);
            attachmentPoint.p0.lineCombin(ad.pos, attachmentDirection[j], offset);
            attachmentPoint.p1.lineCombin(ad.pos, attachmentDirection[j], 0.8f);
            attachmentPoint.color = CWC_BASE;
            attachmentPoint.highlighted = false;
            if (multipleAttachmentPoints)
            {
                attachmentPoint.number = rGroupAttachmentIndices[j];
            }
        }
    }

    // prepare atom id's
    if (_opt.showAtomIds)
    {
        tiindex = _pushTextItem(ad, RenderItem::RIT_ATOMID, CWC_BLUE, false);

        TextItem& index = _data.textitems[tiindex];
        index.fontsize = FONT_SIZE_INDICES;

        int base = _opt.atomBondIdsFromOne ? 1 : 0;
        bprintf(index.text, "%i", aid + base);
        _cw.setTextItemSize(index, ad.pos);

        if (ad.showLabel)
            index.bbp.set(ad.rightMargin + _settings.labelInternalOffset, ad.ypos + 0.5f * ad.height);
    }
}

void MoleculeRenderInternal::_reverseLabelText(const int aid)
{
    AtomDesc& ad = _ad(aid);
    if (ad.pseudo.size() > 0)
    {
        std::ostringstream stream;
        for (int i = 0; i < ad.pseudo.size() - 1; ++i)
        {
            stream << ad.pseudo[i];
        }

        std::vector<std::string> splitLabel = _splitLabelText(stream.str());
        std::reverse(splitLabel.begin(), splitLabel.end());
        std::ostringstream outStream;
        for (const auto& label : splitLabel)
        {
            outStream << label;
        }
        std::string reversedLabel = outStream.str();

        for (int i = 0; i < ad.pseudo.size() - 1; ++i)
        {
            ad.pseudo[i] = reversedLabel[i];
        }
    }
}

std::vector<std::string> MoleculeRenderInternal::_splitLabelText(const std::string& label) const
{
    std::vector<std::string> result;
    std::regex pattern("(([nst]-Bu|sBu|[ni]-Pr|iPr|Tol|Me|Et|Ac|Tf|Si|Na|Mg|Cl|Br)\\d*)");
    std::sregex_token_iterator iter(label.begin(), label.end(), pattern);
    std::sregex_token_iterator end;

    while (iter != end)
    {
        result.push_back(*iter);
        ++iter;
    }

    size_t length = 0;
    for (const auto& abbreviation : result)
    {
        length += abbreviation.length();
    }

    if (length != label.length())
    {
        return {label};
    }

    return result;
}

int MoleculeRenderInternal::_pushTextItem(RenderItem::TYPE ritype, int color, bool highlighted)
{
    _data.textitems.push();
    _data.textitems.top().clear();
    _data.textitems.top().ritype = ritype;
    _data.textitems.top().color = color;
    _data.textitems.top().highlighted = highlighted;
    return _data.textitems.size() - 1;
}

int MoleculeRenderInternal::_pushTextItem(AtomDesc& ad, RenderItem::TYPE ritype, int color, bool highlighted)
{
    int res = _pushTextItem(ritype, color, highlighted);
    if (ad.tibegin < 0)
        ad.tibegin = res;
    ad.ticount++;
    return res;
}

int MoleculeRenderInternal::_pushTextItem(Sgroup& sg, RenderItem::TYPE ritype, int color)
{
    int res = _pushTextItem(ritype, color, false);
    if (sg.tibegin < 0)
        sg.tibegin = res;
    sg.ticount++;
    return res;
}

int MoleculeRenderInternal::_pushTextItem(AttachmentGroupLabel& label, RenderItem::TYPE ritype, int color, bool highlighted)
{
    int res = _pushTextItem(ritype, color, highlighted);
    if (label.tibegin < 0)
        label.tibegin = res;
    label.ticount++;
    return res;
}

int MoleculeRenderInternal::_pushGraphItem(RenderItem::TYPE ritype, int color, bool highlighted)
{
    _data.graphitems.push();
    _data.graphitems.top().clear();
    _data.graphitems.top().ritype = ritype;
    _data.graphitems.top().color = color;
    _data.graphitems.top().highlighted = highlighted;
    return _data.graphitems.size() - 1;
}

int MoleculeRenderInternal::_pushGraphItem(AtomDesc& ad, RenderItem::TYPE ritype, int color, bool highlighted)
{
    int res = _pushGraphItem(ritype, color, highlighted);
    if (ad.gibegin < 0)
        ad.gibegin = res;
    ad.gicount++;
    return res;
}

int MoleculeRenderInternal::_pushGraphItem(AttachmentGroupLabel& label, RenderItem::TYPE ritype, int color, bool highlighted)
{
    int res = _pushGraphItem(ritype, color, highlighted);
    if (label.gibegin < 0)
        label.gibegin = res;
    label.gicount++;
    return res;
}

const char* MoleculeRenderInternal::_valenceText(const int valence)
{
    const char* vt[] = {"(0)", "(I)", "(II)", "(III)", "(IV)", "(V)", "(VI)", "(VII)", "(VIII)", "(IX)", "(X)", "(XI)", "(XII)", "(XIII)"};
    if (valence < 0 || valence >= NELEM(vt))
        throw Error("valence value %i out of range", valence);
    return vt[valence];
}

#ifdef _WIN32
#pragma warning(pop)
#endif
