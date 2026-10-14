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

#include "layout/molecule_layout.h"
#include "base_cpp/array.h"
#include "graph/filter.h"
#include <algorithm>
#include <cmath>
#include <map>
#include <utility>
#include <vector>

using namespace indigo;

// S-group bracket sizes, in bond lengths
constexpr float CHAIN_BRACKET_LENGTH = 1.5f; // the pair around a chain unit, as BIOVIA and Ketcher draw it
constexpr float BOND_BRACKET_LENGTH = 1.f;   // one bracket per bond, for two bonds that do not point apart
constexpr float SHORT_BRACKET_LENGTH = 0.6f; // one per bond for three or more, as BIOVIA draws them; longer ones close up
constexpr float FAR_BRACKET_GAP = 0.5f;      // beyond the farthest atom of a group with a single crossing bond

IMPL_ERROR(MoleculeLayout, "molecule_layout");

MoleculeLayout::MoleculeLayout(BaseMolecule& molecule, bool smart_layout)
    : _molecule(molecule), _smart_layout(smart_layout), respect_existing_layout(false), multiple_distance(std::nullopt), respect_cycles_direction(false),
      flexible_fixed_components(false), sequence_layout(false)
{
    _hasMulGroups = _molecule.sgroups.getSGroupCount(SGroup::SG_TYPE_MUL) > 0;
    _init(smart_layout);
    _query = _molecule.isQueryMolecule();
}

void MoleculeLayout::_init(bool smart_layout)
{
    bond_length = 1.f;
    haptic_bond_multiplier = MoleculeLayoutGraph::DEFAULT_HAPTIC_BOND_MULTIPLIER;
    filter = 0;
    _smart_layout = smart_layout;
    if (_smart_layout)
        _layout_graph = std::make_unique<MoleculeLayoutGraphSmart>();
    else
    {
        _layout_graph = std::make_unique<MoleculeLayoutGraphSimple>();
        _layout_graph->respect_cycles_direction = respect_cycles_direction;
        _layout_graph->sequence_layout = sequence_layout;
    }

    max_iterations = LAYOUT_MAX_ITERATION;
    _query = false;
    _atomMapping.clear();

    _bm = &_molecule;
    if (_hasMulGroups)
    {
        if (_molecule.isQueryMolecule())
            _molCollapsed = std::make_unique<QueryMolecule>();
        else
            _molCollapsed = std::make_unique<Molecule>();
        _molCollapsed->clone(_molecule, &_atomMapping, NULL);
        QS_DEF(BaseMolecule::Mapping, atomMapCollapse);
        QS_DEF(BaseMolecule::Mapping, bondMapInv);

        for (int i = _molCollapsed->sgroups.begin(); i != _molCollapsed->sgroups.end(); i = _molCollapsed->sgroups.next(i))
        {
            SGroup& sg = _molCollapsed->sgroups.getSGroup(i);
            if (sg.sgroup_type == SGroup::SG_TYPE_MUL)
            {
                // collapse multiple group
                atomMapCollapse.clear();
                bondMapInv.clear();
                BaseMolecule::collapse(*_molCollapsed, i, atomMapCollapse, bondMapInv);

                // modify the atom mapping
                for (int j = 0; j < _atomMapping.size(); ++j)
                    if (atomMapCollapse.find(_atomMapping[j]) != atomMapCollapse.end())
                        _atomMapping[j] = atomMapCollapse.at(_atomMapping[j]);
            }
        }
        _bm = _molCollapsed.get();
    }

    _giveAtomToAtomHapticBondsToTheLayout();

    _layout_graph->makeOnGraph(*_bm);

    for (int i = _layout_graph->vertexBegin(); i < _layout_graph->vertexEnd(); i = _layout_graph->vertexNext(i))
    {
        const Vec3f& pos = _bm->getAtomXyz(_layout_graph->getVertexExtIdx(i));

        _layout_graph->getPos(i).set(pos.x, pos.y);
    }
}

void _collectCrossBonds(Array<int>& crossBonds, Array<bool>& crossBondOut, BaseMolecule& mol, const Array<int>& atoms)
{
    QS_DEF(Array<bool>, atomMask);
    atomMask.clear_resize(mol.vertexEnd());
    atomMask.fill(false);
    for (int i = 0; i < atoms.size(); ++i)
    {
        int aid = atoms[i];
        atomMask[aid] = true;
    }
    crossBonds.clear();
    crossBondOut.clear();
    for (int i = 0; i < atoms.size(); ++i)
    {
        int aid = atoms[i];
        const Vertex& v = mol.getVertex(aid);
        for (int j = v.neiBegin(); j < v.neiEnd(); j = v.neiNext(j))
        {
            int naid = v.neiVertex(j);
            if (!atomMask[naid])
            {
                int bid = v.neiEdge(j);
                crossBonds.push(bid);
                crossBondOut.push(mol.getEdge(bid).beg == aid);
            }
        }
    }
}

// Walking from the first end of a bracket to the second keeps the group on the right: the renderer
// turns the bracket hooks by this order.
static void _pushSGroupBracket(Array<Vec2f[2]>& brackets, const Vec2f& center, const Vec2f& outward, float length)
{
    Vec2f across(outward);
    across.rotate(1, 0);
    Vec2f* const& bracket = brackets.push();
    bracket[0].lineCombin(center, across, 0.5f * length);
    bracket[1].lineCombin(center, across, -0.5f * length);
}

// The renderer writes the subscript at the last bracket; like BIOVIA, put it at the right one, or at the
// lower one of a vertical pair. The tolerance keeps rounding from deciding a vertical pair.
static void _putRightBracketLast(Array<Vec2f[2]>& brackets, float bondLength)
{
    if (brackets.size() != 2)
        return;
    const float x0 = (brackets[0][0].x + brackets[0][1].x) / 2, y0 = (brackets[0][0].y + brackets[0][1].y) / 2;
    const float x1 = (brackets[1][0].x + brackets[1][1].x) / 2, y1 = (brackets[1][0].y + brackets[1][1].y) / 2;
    const float tolerance = 0.01f * bondLength;
    if (x0 > x1 + tolerance || (std::fabs(x0 - x1) <= tolerance && y0 < y1))
    {
        std::swap(brackets[0][0], brackets[1][0]);
        std::swap(brackets[0][1], brackets[1][1]);
    }
}

static void _getCrossBond(BaseMolecule& mol, int bid, bool out, Vec2f& inside, Vec2f& middle, Vec2f& outward)
{
    const Edge& edge = mol.getEdge(bid);
    Vec2f outside;
    Vec2f::projectZ(inside, mol.getAtomXyz(out ? edge.beg : edge.end));
    Vec2f::projectZ(outside, mol.getAtomXyz(out ? edge.end : edge.beg));
    middle.lineCombin2(inside, 0.5f, outside, 0.5f);
    outward.diff(outside, inside);
    outward.normalize();
}

// A median, not a mean: the long bonds to the `*` ends of a chain, common in drawings from Ketcher,
// would stretch a mean.
static float _medianBondLength(BaseMolecule& mol)
{
    std::vector<float> lengths;
    for (int i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
    {
        const Edge& edge = mol.getEdge(i);
        Vec2f beg, end;
        Vec2f::projectZ(beg, mol.getAtomXyz(edge.beg));
        Vec2f::projectZ(end, mol.getAtomXyz(edge.end));
        end.sub(beg);
        // Coincident atoms, such as the copies of a multiple group during layout, say nothing about the scale
        const float length = end.length();
        if (length > EPSILON)
            lengths.push_back(length);
    }
    if (lengths.empty())
        return 0;
    auto middle = lengths.begin() + lengths.size() / 2;
    std::nth_element(lengths.begin(), middle, lengths.end());
    return *middle;
}

void _placeSGroupBracketsCrossBonds(Array<Vec2f[2]>& brackets, BaseMolecule& mol, const Array<int>& crossBonds, const Array<bool>& crossBondOut,
                                    float bondLength)
{
    brackets.clear();
    std::vector<Vec2f> middles(crossBonds.size()), outwards(crossBonds.size());
    Vec2f inside;
    for (int i = 0; i < crossBonds.size(); ++i)
        _getCrossBond(mol, crossBonds[i], crossBondOut[i], inside, middles[i], outwards[i]);

    if (crossBonds.size() == 2 && Vec2f::dot(outwards[0], outwards[1]) < -0.3f)
    {
        // Each bond has to point away from the other along the axis: offset antiparallel bonds would lie along
        // brackets across the axis, and bonds pointing at each other would turn the hooks away from the group
        Vec2f axis;
        axis.diff(middles[0], middles[1]);
        if (axis.normalize() && Vec2f::dot(axis, outwards[0]) >= 0.5f && Vec2f::dot(axis, outwards[1]) <= -0.5f)
        {
            Vec2f back(axis);
            back.negate();
            _pushSGroupBracket(brackets, middles[0], axis, CHAIN_BRACKET_LENGTH * bondLength);
            _pushSGroupBracket(brackets, middles[1], back, CHAIN_BRACKET_LENGTH * bondLength);
            _putRightBracketLast(brackets, bondLength);
            return;
        }
    }

    const float length = (crossBonds.size() > 2 ? SHORT_BRACKET_LENGTH : BOND_BRACKET_LENGTH) * bondLength;
    for (int i = 0; i < crossBonds.size(); ++i)
        _pushSGroupBracket(brackets, middles[i], outwards[i], length);
    _putRightBracketLast(brackets, bondLength);
}

void _placeSGroupBracketsCrossBondSingle(Array<Vec2f[2]>& brackets, BaseMolecule& mol, const Array<int>& atoms, int bid, bool out, float bondLength)
{
    brackets.clear();
    Vec2f inside, middle, outward;
    _getCrossBond(mol, bid, out, inside, middle, outward);
    _pushSGroupBracket(brackets, middle, outward, CHAIN_BRACKET_LENGTH * bondLength);

    Vec2f across(outward);
    across.rotate(1, 0);
    float farthest = 0, side = 0;
    for (int aid : atoms)
    {
        Vec2f p;
        Vec2f::projectZ(p, mol.getAtomXyz(aid));
        p.sub(inside);
        farthest = std::min(farthest, Vec2f::dot(p, outward));
        side += Vec2f::dot(p, across);
    }
    if (atoms.size() > 0)
        side /= atoms.size();
    Vec2f center, back(outward);
    back.negate();
    center.lineCombin(inside, outward, farthest - FAR_BRACKET_GAP * bondLength);
    center.addScaled(across, side);
    _pushSGroupBracket(brackets, center, back, CHAIN_BRACKET_LENGTH * bondLength);
    _putRightBracketLast(brackets, bondLength);
}

void _placeSGroupBracketsHorizontal(Array<Vec2f[2]>& brackets, BaseMolecule& mol, const Array<int>& atoms, float bondLength)
{
    brackets.clear();
    Vec2f min, max, a, b;
    for (int i = 0; i < atoms.size(); ++i)
    {
        int aid = atoms[i];
        const Vec3f& pos = mol.getAtomXyz(aid);
        Vec2f p2d;
        Vec2f::projectZ(p2d, pos);
        if (i == 0)
        {
            min.copy(p2d);
            max.copy(p2d);
        }
        else
        {
            min.min(p2d);
            max.max(p2d);
        }
    }

    float extent = 0.5f * bondLength;
    min.sub(Vec2f(extent, extent));
    max.add(Vec2f(extent, extent));

    Vec2f* const& left = brackets.push();
    left[0].set(min.x, min.y);
    left[1].set(min.x, max.y);
    Vec2f* const& right = brackets.push();
    right[0].set(max.x, max.y);
    right[1].set(max.x, min.y);
}

void MoleculeLayout::_updateDataSGroups()
{
    // Move Data-SGroups with absolute coordinates according to new position
    QS_DEF(Array<int>, layout_graph_mapping);
    layout_graph_mapping.resize(_molecule.vertexEnd());
    layout_graph_mapping.fffill();
    for (int i = _layout_graph->vertexBegin(); i < _layout_graph->vertexEnd(); i = _layout_graph->vertexNext(i))
    {
        int vi = _layout_graph->getVertexExtIdx(i);
        layout_graph_mapping[vi] = i;
    }

    for (int i = _molecule.sgroups.begin(); i != _molecule.sgroups.end(); i = _molecule.sgroups.next(i))
    {
        SGroup& sg = _molecule.sgroups.getSGroup(i);
        if (sg.sgroup_type == SGroup::SG_TYPE_DAT)
        {
            DataSGroup& group = (DataSGroup&)sg;
            if (!group.relative && group.display_pos.has_value())
            {
                Vec2f before;
                _molecule.getSGroupAtomsCenterPoint(group, before);

                Vec2f after;
                for (int j = 0; j < group.atoms.size(); j++)
                {
                    int ai = group.atoms[j];
                    const LayoutVertex& vert = _layout_graph->getLayoutVertex(layout_graph_mapping[ai]);
                    after.x += vert.pos.x;
                    after.y += vert.pos.y;
                }

                if (group.atoms.size() != 0)
                    after.scale(1.0f / group.atoms.size());

                Vec2f delta;
                delta.diff(after, before);
                Vec2f dp = group.display_pos.value();
                dp.add(delta);
                group.display_pos.set(dp);
            }
        }
    }
}

std::vector<std::pair<int, int>> MoleculeLayout::_atomToAtomHapticRings() const
{
    // Which haptic bonds between two atoms close a ring of such bonds. HapticLayout
    // places one component after another, and a ring cannot be closed that way - the
    // last component would have to satisfy two bonds at once - so a ring goes to the
    // ordinary layout, which sees all of it at once. A lone bond between two
    // fragments stays here: HapticLayout draws it longer when that is what keeps the
    // fragments apart, and the ordinary layout has no such freedom.
    const Array<int>& component = _molecule.getDecomposition();

    std::vector<std::pair<int, int>> joining;
    const MoleculeHapticBonds& bonds = _molecule.haptic_bonds;
    for (int i = bonds.begin(); i != bonds.end(); i = bonds.next(i))
    {
        const HapticBond& bond = bonds.at(i);
        if (bond.type() != _BOND_HAPTIC || bond.begin().isGroup() || bond.end().isGroup())
            continue;

        const int beg = bond.begin().index(), end = bond.end().index();
        if (beg == end || !_molecule.hasVertex(beg) || !_molecule.hasVertex(end) || _molecule.findEdgeIndex(beg, end) >= 0)
            continue;

        // Both ends in one component: the edges between them already settle the
        // geometry, and adding one more would change the shape of the fragment.
        if (component[beg] == component[end])
            continue;

        joining.push_back(std::make_pair(beg, end));
    }

    std::vector<std::pair<int, int>> ringed;
    if (joining.empty())
        return ringed;

    // Group the bonds by what they connect. A group of components joined into a
    // tree has one bond fewer than it has components; as many bonds as components
    // means a ring somewhere in it.
    std::map<int, int> parent;
    for (const std::pair<int, int>& pair : joining)
        for (int atom : {pair.first, pair.second})
            parent.insert(std::make_pair(component[atom], component[atom]));

    const auto root = [&parent](int of) {
        while (parent[of] != of)
        {
            parent[of] = parent[parent[of]];
            of = parent[of];
        }
        return of;
    };

    for (const std::pair<int, int>& pair : joining)
    {
        const int one = root(component[pair.first]), two = root(component[pair.second]);
        if (one != two)
            parent[one] = two;
    }

    std::map<int, int> components_of, bonds_of;
    for (const std::pair<const int, int>& entry : parent)
        components_of[root(entry.first)]++;
    for (const std::pair<int, int>& pair : joining)
        bonds_of[root(component[pair.first])]++;

    for (const std::pair<int, int>& pair : joining)
    {
        const int group = root(component[pair.first]);
        if (bonds_of[group] >= components_of[group])
            ringed.push_back(pair);
    }

    return ringed;
}

void MoleculeLayout::_giveAtomToAtomHapticBondsToTheLayout()
{
    if (_molecule.isQueryMolecule() || _molecule.haptic_bonds.isEmpty())
        return;

    const std::vector<std::pair<int, int>> ringed = _atomToAtomHapticRings();
    if (ringed.empty())
        return;

    if (!_molCollapsed)
    {
        _molCollapsed = std::make_unique<Molecule>();
        _molCollapsed->clone(_molecule, &_atomMapping, NULL);
        _bm = _molCollapsed.get();
    }

    Molecule& copy = _bm->asMolecule();
    for (const std::pair<int, int>& pair : ringed)
    {
        const int beg = _atomMapping[pair.first], end = _atomMapping[pair.second];
        if (beg < 0 || end < 0 || !copy.hasVertex(beg) || !copy.hasVertex(end) || copy.findEdgeIndex(beg, end) >= 0)
            continue;

        // Silent: the copy is thrown away once the coordinates are read off it, and
        // the valence of the original never hears about this bond (#3837).
        copy.addBond_Silent(beg, end, BOND_SINGLE);
    }
}

void MoleculeLayout::_make()
{
    _layout_graph->max_iterations = max_iterations;
    _layout_graph->layout_orientation = layout_orientation;
    _layout_graph->respect_cycles_direction = respect_cycles_direction;
    _layout_graph->sequence_layout = sequence_layout;
    _layout_graph->haptic_bond_multiplier = haptic_bond_multiplier;
    // 0. Find 2D coordinates via proxy _layout_graph object
    _layout_graph->max_iterations = max_iterations;
    _makeLayout();

    // 1. Update data-sgroup label position before changing molecule atoms positions
    _updateDataSGroups();

    // 2. Update atoms
    for (int i = _layout_graph->vertexBegin(); i < _layout_graph->vertexEnd(); i = _layout_graph->vertexNext(i))
    {
        const LayoutVertex& vert = _layout_graph->getLayoutVertex(i);
        _bm->setAtomXyz(vert.ext_idx, vert.pos.x, vert.pos.y, 0.f);
    }

    if (_molCollapsed)
    {
        for (int j = 0; j < _atomMapping.size(); ++j)
        {
            int i = _atomMapping[j];
            _molecule.setAtomXyz(j, _molCollapsed->getAtomXyz(i));
        }
        _molCollapsed.reset(nullptr);
    }

    _updateMultipleGroups();

    _updateRepeatingUnits();

    _molecule.have_xyz = true;
}

Metalayout::LayoutItem& MoleculeLayout::_pushMol(Metalayout::LayoutLine& line, BaseMolecule& mol)
{
    Metalayout::LayoutItem& item = line.items.push();
    item.type = Metalayout::LayoutItem::Type::EMolecule;
    item.isMoleculeFragment = true;
    item.id = _map.size();
    _map.push(&mol);
    Metalayout::getBoundRect(item.min, item.max, mol);
    item.scaledSize.diff(item.max, item.min);
    return item;
}

BaseMolecule& MoleculeLayout::_getMol(int id)
{
    return *_map[id];
}

void MoleculeLayout::updateSGroups()
{
    _updateDataSGroups();
    _updateMultipleGroups();
    _updateRepeatingUnits();
}

void MoleculeLayout::make()
{
    _make();
    if (_molecule.rgroups.getRGroupCount() > 0)
    {
        MoleculeRGroups& rgs = ((QueryMolecule&)_molecule).rgroups;
        _ml.clear();
        _map.clear();
        _pushMol(_ml.newLine(), _molecule);
        for (int i = 1; i <= rgs.getRGroupCount(); ++i)
        {
            RGroup& rg = rgs.getRGroup(i);
            Metalayout::LayoutLine& line = _ml.newLine();
            PtrReusablePool<BaseMolecule>& frags = rg.fragments;
            for (int j = frags.begin(); j != frags.end(); j = frags.next(j))
            {
                BaseMolecule& mol = frags[j];
                if (filter == NULL)
                {
                    MoleculeLayout layout(mol, _smart_layout);
                    layout.max_iterations = max_iterations;
                    layout.layout_orientation = layout_orientation;
                    layout.bond_length = bond_length;
                    layout.haptic_bond_multiplier = haptic_bond_multiplier;
                    layout.multiple_distance = multiple_distance;
                    layout.make();
                }
                _pushMol(line, mol); // add molecule to metalayout AFTER its own layout is determined
            }
        }
        _ml.bondLength = bond_length;
        _ml.context = this;
        _ml.cb_getMol = cb_getMol;
        _ml.cb_process = cb_process;
        _ml.prepare();
        _ml.scaleMoleculesSize();
        _ml.calcContentSize();
        _ml.process();
    }
}

void MoleculeLayout::setCancellationHandler(CancellationHandler* cancellation)
{
    _layout_graph->cancellation = cancellation;
}

BaseMolecule& MoleculeLayout::cb_getMol(int id, void* context)
{
    return ((MoleculeLayout*)context)->_getMol(id);
}

void MoleculeLayout::cb_process(Metalayout::LayoutItem& item, const Vec2f& pos, void* context)
{
    MoleculeLayout* layout = (MoleculeLayout*)context;
    layout->_ml.adjustMol(layout->_getMol(item.id), item.min, pos);
}

void MoleculeLayout::_makeLayout()
{
    if (filter != 0)
    {
        QS_DEF(Array<int>, fixed_vertices);

        fixed_vertices.clear_resize(_layout_graph->vertexEnd());
        fixed_vertices.zerofill();

        for (int i = _layout_graph->vertexBegin(); i < _layout_graph->vertexEnd(); i = _layout_graph->vertexNext(i))
            if (!filter->valid(_layout_graph->getVertexExtIdx(i)))
                fixed_vertices[i] = 1;

        Filter new_filter(fixed_vertices.ptr(), Filter::NEQ, 1);

        _layout_graph->layout(*_bm, bond_length, &new_filter, respect_existing_layout, multiple_distance);
    }
    else
        _layout_graph->layout(*_bm, bond_length, 0, respect_existing_layout, multiple_distance);
}

void MoleculeLayout::_updateRepeatingUnits()
{
    QS_DEF(Array<int>, crossBonds);
    QS_DEF(Array<bool>, crossBondOut);
    float median_bond_length = -1;
    for (int i = _molecule.sgroups.begin(); i != _molecule.sgroups.end(); i = _molecule.sgroups.next(i))
    {
        SGroup& sg = _molecule.sgroups.getSGroup(i);
        if (sg.sgroup_type == SGroup::SG_TYPE_SRU)
        {
            if (median_bond_length < 0)
            {
                median_bond_length = _medianBondLength(_molecule);
                if (median_bond_length <= 0)
                    median_bond_length = bond_length;
            }
            RepeatingUnit& ru = (RepeatingUnit&)sg;
            crossBonds.clear();
            crossBondOut.clear();
            _collectCrossBonds(crossBonds, crossBondOut, _molecule, ru.atoms);
            if (crossBonds.size() > 1)
                _placeSGroupBracketsCrossBonds(ru.brackets, _molecule, crossBonds, crossBondOut, median_bond_length);
            else if (crossBonds.size() == 1)
                _placeSGroupBracketsCrossBondSingle(ru.brackets, _molecule, ru.atoms, crossBonds[0], crossBondOut[0], median_bond_length);
            else
                _placeSGroupBracketsHorizontal(ru.brackets, _molecule, ru.atoms, median_bond_length);
        }
    }
}

void MoleculeLayout::_updateMultipleGroups()
{
    for (int i = _molecule.sgroups.begin(); i != _molecule.sgroups.end(); i = _molecule.sgroups.next(i))
    {
        SGroup& sg = _molecule.sgroups.getSGroup(i);
        if (sg.sgroup_type == SGroup::SG_TYPE_MUL)
        {
            MultipleGroup& mg = (MultipleGroup&)sg;
            _placeSGroupBracketsHorizontal(mg.brackets, _molecule, mg.atoms, bond_length);
        }
    }
}
