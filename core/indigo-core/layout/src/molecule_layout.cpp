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
#include "base_cpp/obj_array.h"
#include "graph/filter.h"
#include <vector>

using namespace indigo;

IMPL_ERROR(MoleculeLayout, "molecule_layout");

MoleculeLayout::MoleculeLayout(BaseMolecule& molecule, bool smart_layout) : _molecule(molecule), _smart_layout(smart_layout)
{
    _hasMulGroups = _molecule.sgroups.getSGroupCount(SGroup::SG_TYPE_MUL) > 0;
    _init(smart_layout);
    _query = _molecule.isQueryMolecule();
}

void MoleculeLayout::_init(bool smart_layout)
{
    bond_length = 1.f;
    respect_existing_layout = false;
    filter = 0;
    _smart_layout = smart_layout;
    if (_smart_layout)
        _layout_graph = std::make_unique<MoleculeLayoutGraphSmart>();
    else
        _layout_graph = std::make_unique<MoleculeLayoutGraphSimple>();

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

void _placeSGroupBracketsCrossBonds(Array<Vec2f[2]>& brackets, BaseMolecule& mol, const Array<int>& atoms, const Array<int>& crossBonds,
                                    const Array<bool>& crossBondOut, float bondLength)
{
    brackets.clear();
    if (crossBonds.size() == 2)
    {
        int bid1 = crossBonds[0], bid2 = crossBonds[1];
        const Edge& edge1 = mol.getEdge(bid1);
        const Edge& edge2 = mol.getEdge(bid2);
        Vec2f pb1, pe1, pb2, pe2;
        Vec2f::projectZ(pb1, mol.getAtomXyz(edge1.beg));
        Vec2f::projectZ(pe1, mol.getAtomXyz(edge1.end));
        Vec2f::projectZ(pb2, mol.getAtomXyz(edge2.beg));
        Vec2f::projectZ(pe2, mol.getAtomXyz(edge2.end));
        Vec2f d1, d2;
        d1.diff(pe1, pb1);
        if (!crossBondOut[0])
            d1.scale(-1);
        d1.normalize();
        d2.diff(pe2, pb2);
        if (!crossBondOut[1])
            d2.scale(-1);
        d2.normalize();
        if (Vec2f::dot(d1, d2) < -0.3)
        {
            Vec2f d, n;
            d.add(pb1);
            d.add(pe1);
            d.sub(pb2);
            d.sub(pe2);
            d.normalize();
            n.copy(d);
            n.rotate(1, 0);
            Vec2f min, max, a, b, c;
            c.add(pb1);
            c.add(pe1);
            c.add(pb2);
            c.add(pe2);
            c.scale(0.25f);
            for (int i = 0; i < atoms.size(); ++i)
            {
                int aid = atoms[i];
                const Vec3f& pos = mol.getAtomXyz(aid);
                Vec2f p2d;
                Vec2f::projectZ(p2d, pos);
                p2d.sub(c);
                p2d.set(Vec2f::dot(p2d, d), Vec2f::dot(p2d, n));
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
            Vec2f b1(c), b2;
            b1.addScaled(d, max.x + 0.3f * bondLength);
            b2.copy(b1);
            float factor = 0.5;
            b1.addScaled(n, factor * bondLength);
            b2.addScaled(n, -factor * bondLength);
            Vec2f* const& bracket1 = brackets.push();
            bracket1[0].copy(b1);
            bracket1[1].copy(b2);

            b1.copy(c);
            b1.addScaled(d, min.x - 0.3f * bondLength);
            b2.copy(b1);
            b1.addScaled(n, -factor * bondLength);
            b2.addScaled(n, factor * bondLength);
            Vec2f* const& bracket2 = brackets.push();
            bracket2[0].copy(b1);
            bracket2[1].copy(b2);
            return;
        }
    }
    for (int i = 0; i < crossBonds.size(); ++i)
    {
        int bid = crossBonds[i];
        const Edge& edge = mol.getEdge(bid);
        int aidIn = edge.beg, aidOut = edge.end;
        if (!crossBondOut[i])
        {

            std::swap(aidIn, aidOut);
        }
        Vec2f p2dIn, p2dOut, d, n, b1, b2;
        Vec2f::projectZ(p2dIn, mol.getAtomXyz(aidIn));
        Vec2f::projectZ(p2dOut, mol.getAtomXyz(aidOut));
        d.diff(p2dOut, p2dIn);
        d.normalize();
        n.copy(d);
        n.rotate(1, 0);
        float offset = 1.0f / 3;
        b1.lineCombin2(p2dIn, 1 - offset, p2dOut, offset);
        b2.copy(b1);
        float factor = 0.5;
        b1.addScaled(n, factor * bondLength);
        b2.addScaled(n, -factor * bondLength);
        Vec2f* const& bracket = brackets.push();
        bracket[0].copy(b1);
        bracket[1].copy(b2);
    }
}

void _placeSGroupBracketsCrossBondSingle(Array<Vec2f[2]>& brackets, BaseMolecule& mol, const Array<int>& atoms, int bid, bool out, float bondLength)
{
    brackets.clear();
    const Edge& edge = mol.getEdge(bid);
    int aidIn = edge.beg, aidOut = edge.end;
    if (!out)
    {

        std::swap(aidIn, aidOut);
    }

    Vec2f p2dIn, p2dOut, d, n, b1, b2;
    Vec2f::projectZ(p2dIn, mol.getAtomXyz(aidIn));
    Vec2f::projectZ(p2dOut, mol.getAtomXyz(aidOut));
    d.diff(p2dOut, p2dIn);
    d.normalize();
    n.copy(d);
    n.rotate(1, 0);
    Vec2f min, max, a, b;

    for (int i = 0; i < atoms.size(); ++i)
    {
        int aid = atoms[i];
        const Vec3f& pos = mol.getAtomXyz(aid);
        Vec2f p2d;
        Vec2f::projectZ(p2d, pos);
        p2d.sub(p2dIn);
        p2d.set(Vec2f::dot(p2d, d), Vec2f::dot(p2d, n));
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

    b1.lineCombin(p2dIn, d, max.x + 0.3f * bondLength);
    b2.copy(b1);
    float factor = 0.5;
    b1.addScaled(n, factor * bondLength);
    b2.addScaled(n, -factor * bondLength);
    Vec2f* const& bracket1 = brackets.push();
    bracket1[0].copy(b1);
    bracket1[1].copy(b2);

    b1.lineCombin(p2dIn, d, min.x - 0.3f * bondLength);
    b2.copy(b1);
    b1.addScaled(n, -factor * bondLength);
    b2.addScaled(n, factor * bondLength);
    Vec2f* const& bracket2 = brackets.push();
    bracket2[0].copy(b1);
    bracket2[1].copy(b2);
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
            if (!group.relative)
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
                group.display_pos.add(delta);
            }
        }
    }
}

void MoleculeLayout::_make()
{
    _layout_graph->max_iterations = max_iterations;
    _layout_graph->layout_orientation = layout_orientation;

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

    if (_hasMulGroups)
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
    item.type = 0;
    item.fragment = true;
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
            PtrPool<BaseMolecule>& frags = rg.fragments;
            for (int j = frags.begin(); j != frags.end(); j = frags.next(j))
            {
                BaseMolecule& mol = *frags[j];
                if (filter == NULL)
                {
                    MoleculeLayout layout(mol, _smart_layout);
                    layout.max_iterations = max_iterations;
                    layout.layout_orientation = layout_orientation;
                    layout.bond_length = bond_length;
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
        _ml.scaleSz();
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

        _layout_graph->layout(*_bm, bond_length, &new_filter, respect_existing_layout);
    }
    else
        _layout_graph->layout(*_bm, bond_length, 0, respect_existing_layout);
}

void MoleculeLayout::_updateRepeatingUnits()
{
    QS_DEF(Array<int>, crossBonds);
    QS_DEF(Array<bool>, crossBondOut);
    for (int i = _molecule.sgroups.begin(); i != _molecule.sgroups.end(); i = _molecule.sgroups.next(i))
    {
        SGroup& sg = _molecule.sgroups.getSGroup(i);
        if (sg.sgroup_type == SGroup::SG_TYPE_SRU)
        {
            RepeatingUnit& ru = (RepeatingUnit&)sg;
            crossBonds.clear();
            crossBondOut.clear();
            _collectCrossBonds(crossBonds, crossBondOut, _molecule, ru.atoms);
            if (crossBonds.size() > 1)
                _placeSGroupBracketsCrossBonds(ru.brackets, _molecule, ru.atoms, crossBonds, crossBondOut, bond_length);
            else if (crossBonds.size() == 1)
                _placeSGroupBracketsCrossBondSingle(ru.brackets, _molecule, ru.atoms, crossBonds[0], crossBondOut[0], bond_length);
            else
                _placeSGroupBracketsHorizontal(ru.brackets, _molecule, ru.atoms, bond_length);
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
