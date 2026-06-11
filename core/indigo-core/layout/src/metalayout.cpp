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

#include "layout/metalayout.h"
#include "base_cpp/tlscont.h"
#include "molecule/molecule.h"

#ifdef _WIN32
#pragma warning(push, 4)
#endif

using namespace indigo;

Metalayout::LayoutLine::LayoutLine()
{
    clear();
}

Metalayout::LayoutLine::~LayoutLine()
{
}

void Metalayout::LayoutLine::clear()
{
    items.clear();
    offset = bottom_height = top_height = height = width = 0;
}

IMPL_ERROR(Metalayout, "metalayout");

Metalayout::Metalayout() : reactionComponentMarginSize(0.5f), verticalIntervalFactor(0.8f), bondLength(1.0f), _avel(1.0f), _scaleFactor(1.0f)
{
    clear();
}

void Metalayout::clear()
{
    _layout.clear();
}

bool Metalayout::isEmpty() const
{
    return _layout.size() == 0;
}

void Metalayout::prepare()
{
    _avel = _getAverageBondLength();
    if (_avel < 1e-4)
        throw Error("average bond length is too small");
    _scaleFactor = bondLength / _avel;
}

float Metalayout::getAverageBondLength() const
{
    return _avel;
}

float Metalayout::getScaleFactor() const
{
    return _scaleFactor;
}

const Vec2f& Metalayout::getContentSize() const
{
    return _contentSize;
}

Metalayout::LayoutLine& Metalayout::newLine()
{
    LayoutLine& line = _layout.push();
    return line;
}

void Metalayout::process()
{
    Vec2f pos;
    for (int i = 0; i < _layout.size(); ++i)
    {
        LayoutLine& line = _layout[i];
        pos.x = line.offset;

        for (int j = 0; j < line.items.size(); ++j)
        {
            LayoutItem& item = line.items[j];
            Vec2f offset(pos);
            auto shiftToAlignAboveVerticalCenter = line.top_height / 2;
            switch (item.verticalAlign)
            {
            case LayoutItem::ItemVerticalAlign::ECenter:
                break;
            case LayoutItem::ItemVerticalAlign::ETop:
                // catalyst
                offset.y += reactionComponentMarginSize + shiftToAlignAboveVerticalCenter;
                break;
            case LayoutItem::ItemVerticalAlign::EBottom:
                offset.y -= (bondLength + line.bottom_height) / 2;
                break;
            }
            cb_process(item, offset, context);

            pos.x += item.scaledSize.x;
        }
        pos.y -= line.height + verticalIntervalFactor * bondLength;
    }
}

void Metalayout::calcContentSize()
{
    _contentSize.set(0, 0);
    float regularWidth = 0.0f;
    for (int i = 0; i < _layout.size(); ++i)
    {
        LayoutLine& line = _layout[i];
        for (int j = 0; j < line.items.size(); ++j)
        {
            line.width += line.items[j].scaledSize.x;
            Metalayout::LayoutItem& item = line.items[j];
            switch (item.verticalAlign)
            {
            case LayoutItem::ItemVerticalAlign::ECenter:
                line.height = std::max(line.height, item.scaledSize.y);
                break;
            case LayoutItem::ItemVerticalAlign::ETop:
                line.top_height = std::max(line.top_height, item.scaledSize.y);
                break;
            case LayoutItem::ItemVerticalAlign::EBottom:
                line.bottom_height = std::max(line.bottom_height, item.scaledSize.y);
                break;
            }
        }
        line.width += reactionComponentMarginSize * bondLength * (line.items.size() - 1);
        _contentSize.x = std::max(_contentSize.x, line.width);
        _contentSize.y += line.height;
        if (regularWidth < line.width)
            regularWidth = line.width;
    }
    _contentSize.y += verticalIntervalFactor * bondLength * (_layout.size() - 1);
}

void Metalayout::scaleMoleculesSize()
{
    for (int i = 0; i < _layout.size(); ++i)
    {
        for (int j = 0; j < _layout[i].items.size(); ++j)
        {
            LayoutItem& item = _layout[i].items[j];
            if (item.isMoleculeFragment)
            {
                item.scaledSize.diff(item.max, item.min);
                item.scaledSize.scale(_scaleFactor);
                item.scaledSize.max(item.minScaledSize);
            }
        }
    }
}

float Metalayout::_getAverageBondLength()
{
    // get total bond length and count
    float totalBondLength = 0;
    int totalBondCount = 0;
    for (int i = 0; i < _layout.size(); ++i)
    {
        LayoutLine& line = _layout[i];
        for (int j = 0; j < line.items.size(); ++j)
        {
            LayoutItem& item = line.items[j];
            if (item.isMoleculeFragment)
            {
                BaseMolecule& mol = cb_getMol(item.id, context);
                totalBondCount += mol.edgeCount();
                totalBondLength += getTotalMoleculeBondLength(mol);
            }
        }
    }

    // if there are any bonds, calculate the average length
    if (totalBondCount > 0)
        return totalBondLength / totalBondCount;

    // get sum of distances from each vertex to the closest one
    float totalClosestDist = 0;
    int totalAtomCount = 0;
    for (int i = 0; i < _layout.size(); ++i)
    {
        LayoutLine& line = _layout[i];
        for (int j = 0; j < line.items.size(); ++j)
        {
            LayoutItem& item = line.items[j];
            if (item.isMoleculeFragment)
            {
                BaseMolecule& mol = cb_getMol(item.id, context);
                int atomCnt = mol.vertexCount();
                if (atomCnt > 1)
                {
                    totalClosestDist += getTotalMoleculeClosestDist(mol);
                    totalAtomCount += atomCnt;
                }
            }
        }
    }

    // if there are molecules with more than one vertex,
    // take average distance to closest vertex instead of
    // average bond length
    if (totalAtomCount > 0)
        return totalClosestDist / totalAtomCount;

    // if each molecule contains at most one atom,
    // the average bond length doesn't matter
    return 1.0f;
}

void Metalayout::getBoundRect(Vec2f& min, Vec2f& max, BaseMolecule& mol)
{
    if (mol.vertexCount() == 0)
    {
        min.zero();
        max.zero();
        return;
    }
    const Vec3f& v0 = mol.getAtomXyz(mol.vertexBegin());
    Vec2f::projectZ(min, v0);
    Vec2f::projectZ(max, v0);
    Vec2f v2;
    for (int i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
    {
        Vec2f::projectZ(v2, mol.getAtomXyz(i));
        min.min(v2);
        max.max(v2);
    }
}

float Metalayout::getTotalMoleculeBondLength(BaseMolecule& mol)
{
    Vec2f v1, v2;
    float sum = 0;
    for (int i = mol.edgeBegin(); i < mol.edgeEnd(); i = mol.edgeNext(i))
    {
        const Edge& edge = mol.getEdge(i);
        Vec2f::projectZ(v1, mol.getAtomXyz(edge.beg));
        Vec2f::projectZ(v2, mol.getAtomXyz(edge.end));
        sum += Vec2f::dist(v1, v2);
    }

    return sum;
}

float Metalayout::getTotalMoleculeClosestDist(BaseMolecule& mol)
{
    QS_DEF(Array<float>, dst);
    float sum = 0;

    dst.clear_resize(mol.vertexEnd());
    for (int i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
        dst[i] = -1;

    for (int i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
        for (int j = mol.vertexNext(i); j < mol.vertexEnd(); j = mol.vertexNext(j))
        {
            Vec2f u, v;
            Vec2f::projectZ(u, mol.getAtomXyz(i));
            Vec2f::projectZ(v, mol.getAtomXyz(j));
            float d = Vec2f::dist(u, v);
            if (dst[i] < 0 || dst[i] > d)
                dst[i] = d;
            if (dst[j] < 0 || dst[j] > d)
                dst[j] = d;
        }
    for (int i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
        sum += dst[i];
    return sum;
}

void Metalayout::adjustMol(BaseMolecule& mol, const Vec2f& min, const Vec2f& pos) const
{
    // Compute center points for the data sgroups
    QS_DEF(Array<Vec2f>, data_centers);
    data_centers.resize(mol.sgroups.getSGroupCount());
    for (int i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
    {
        SGroup& sg = mol.sgroups.getSGroup(i);
        if (sg.sgroup_type == SGroup::SG_TYPE_DAT)
        {
            DataSGroup& group = (DataSGroup&)sg;
            if (!group.relative)
                mol.getSGroupAtomsCenterPoint(group, data_centers[i]);
        }
    }

    for (int i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
    {
        Vec2f v;
        Vec2f::projectZ(v, mol.getAtomXyz(i));
        v.sub(min);
        v.scale(_scaleFactor);
        v.add(pos);
        mol.setAtomXyz(i, v.x, v.y, 0);
    }

    // Adjust data-sgroup label positions with absolute coordinates
    for (int i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
    {
        SGroup& sg = mol.sgroups.getSGroup(i);
        if (sg.sgroup_type == SGroup::SG_TYPE_DAT)
        {
            DataSGroup& group = (DataSGroup&)sg;
            if (!group.relative)
            {
                Vec2f new_center;
                mol.getSGroupAtomsCenterPoint(group, new_center);
                group.display_pos.add(new_center);
                group.display_pos.sub(data_centers[i]);
            }
        }
    }
}

#ifdef _WIN32
#pragma warning(pop)
#endif
