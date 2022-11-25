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

#include <string>

#include "base_cpp/output.h"
#include "molecule/ket_commons.h"
#include "molecule/molecule_cdxml_saver.h"
#include "reaction/reaction.h"
#include "reaction/reaction_cdxml_saver.h"

using namespace indigo;

// Molecule position on the page
struct Pos
{
    // Structure min and max coordinates
    Vec2f str_min, str_max;

    // Offset and size on the page
    Vec2f page_offset;
    Vec2f size, all_size;

    // Final offset for the coordinates
    Vec2f offset;
    float title_offset_y;

    // Structure scaling coefficient
    float scale;
};

void _getBounds(BaseMolecule& mol, Vec2f& min, Vec2f& max, float scale)
{
    for (int i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
    {
        Vec3f& p = mol.getAtomXyz(i);
        Vec2f p2(p.x, p.y);

        if (i == mol.vertexBegin())
            min = max = p2;
        else
        {
            min.min(p2);
            max.max(p2);
        }
    }

    min.scale(scale);
    max.scale(scale);
}

IMPL_ERROR(ReactionCdxmlSaver, "reaction CDXML saver");

ReactionCdxmlSaver::ReactionCdxmlSaver(Output& output) : _output(output), _id(0)
{
}

ReactionCdxmlSaver::~ReactionCdxmlSaver()
{
}

void ReactionCdxmlSaver::saveReaction(BaseReaction& rxn)
{
    int i;
    std::vector<int> mol_ids;
    std::vector<int> meta_ids;

    std::vector<std::vector<int>> nodes_ids;

    MoleculeCdxmlSaver molsaver(_output);
    MoleculeCdxmlSaver::Bounds b;

    molsaver.beginDocument(NULL);
    molsaver.addDefaultFontTable();
    molsaver.addDefaultColorTable();
    molsaver.beginPage(NULL);

    _id = molsaver.getId();
    _generateCdxmlObjIds(rxn, mol_ids, meta_ids, nodes_ids);

    std::vector<std::pair<int, int>> arrow_ids;
    int arrow_count = rxn.meta().getMetaCount(KETReactionArrow::CID);
    if (arrow_count)
    {
        for (int i = 0; i < arrow_count; ++i)
        {
            int array_index = rxn.meta().getMetaObjectIndex(KETReactionArrow::CID, i);
            arrow_ids.emplace_back(meta_ids[array_index], arrow_count > 1 ? array_index : -1);
        }
    }
    else
        arrow_ids.emplace_back(++_id, -1);

    Vec2f offset(0, 0);

    for (i = rxn.begin(); i != rxn.end(); i = rxn.next(i))
        molsaver.saveMoleculeFragment(rxn.getBaseMolecule(i), offset, 1, mol_ids[i], _id, nodes_ids[i]);

    if (rxn.meta().metaData().size()) // we have metadata
    {
        for (int i = 0; i < rxn.meta().metaData().size(); ++i)
        {
            molsaver.addMetaObject(*rxn.meta().metaData()[i], meta_ids[i]);
        }
    }
    else
    {
        _addPlusses(rxn, molsaver);
        _addArrow(rxn, molsaver, arrow_ids.front().first);
    }

    _addScheme(molsaver);
    for (const auto& ar_id : arrow_ids)
        _addStep(rxn, molsaver, mol_ids, nodes_ids, ar_id);
    _closeScheme(molsaver);

    if (rxn.name.size() > 0)
    {
        _addTitle(rxn, molsaver);
    }

    molsaver.endPage();
    molsaver.endDocument();
}

void ReactionCdxmlSaver::_addPlusses(BaseReaction& rxn, MoleculeCdxmlSaver& molsaver)
{
    Vec2f offset(0, 0);

    if (rxn.reactantsCount() > 1)
    {
        int rcount = 1;
        for (auto i = rxn.reactantBegin(); i != rxn.reactantEnd(); i = rxn.reactantNext(i))
        {
            if (rcount < rxn.reactantsCount())
            {
                Vec2f min1, max1;
                Vec2f min2, max2;
                _getBounds(rxn.getBaseMolecule(i), min1, max1, 1.0);
                _getBounds(rxn.getBaseMolecule(rxn.reactantNext(i)), min2, max2, 1.0);
                offset.x = (max1.x + min2.x) / 2;
                offset.y = (min1.y + max1.y) / 2;

                molsaver.addText(offset, "+");
                rcount++;
            }
        }
    }

    if (rxn.productsCount() > 1)
    {
        int pcount = 1;
        for (auto i = rxn.productBegin(); i != rxn.productEnd(); i = rxn.productNext(i))
        {
            if (pcount < rxn.productsCount())
            {
                Vec2f min1, max1;
                Vec2f min2, max2;
                _getBounds(rxn.getBaseMolecule(i), min1, max1, 1.0);
                _getBounds(rxn.getBaseMolecule(rxn.productNext(i)), min2, max2, 1.0);
                offset.x = (max1.x + min2.x) / 2;
                offset.y = (min1.y + max1.y) / 2;

                molsaver.addText(offset, "+");
                pcount++;
            }
        }
    }
}

void ReactionCdxmlSaver::_addArrow(BaseReaction& rxn, MoleculeCdxmlSaver& molsaver, int arrow_id)
{
    Vec2f p1(0, 0);
    Vec2f p2(0, 0);
    PropertiesMap attrs;
    attrs.clear();

    Vec2f rmin(0, 0);
    Vec2f rmax(0, 0);

    if (rxn.reactantsCount() > 0)
    {
        for (auto i = rxn.reactantBegin(); i != rxn.reactantEnd(); i = rxn.reactantNext(i))
        {
            Vec2f min, max;
            _getBounds(rxn.getBaseMolecule(i), min, max, 1.0);
            if (i == rxn.reactantBegin())
            {
                rmin = min;
                rmax = max;
            }
            else
            {
                rmin.min(min);
                rmax.max(max);
            }
        }
    }

    Vec2f pmin(0, 0);
    Vec2f pmax(0, 0);

    if (rxn.productsCount() > 0)
    {
        for (auto i = rxn.productBegin(); i != rxn.productEnd(); i = rxn.productNext(i))
        {
            Vec2f min, max;
            _getBounds(rxn.getBaseMolecule(i), min, max, 1.0);
            if (i == rxn.productBegin())
            {
                pmin = min;
                pmax = max;
            }
            else
            {
                pmin.min(min);
                pmax.max(max);
            }
        }
    }

    if (rxn.reactantsCount() == 0 && rxn.productsCount() == 0)
        return;
    else if (rxn.productsCount() == 0)
    {
        p2.x = rmax.x + 1.0f;
        p2.y = (rmin.y + rmax.y) / 2;
        p1.x = p2.x + 1.0f;
        p1.y = p2.y;
    }
    else if (rxn.reactantsCount() == 0)
    {
        p1.x = pmin.x - 1.0f;
        p1.y = (pmin.y + pmax.y) / 2;
        p2.x = p1.x - 1.0f;
        p2.y = p1.y;
    }
    else
    {
        if ((pmin.x - rmax.x) > 0)
        {
            p2.x = (rmax.x + pmin.x) / 2 - (pmin.x - rmax.x) / 4;
            p2.y = (rmin.y + rmax.y) / 2;
        }
        else
        {
            p2.x = (rmax.x + pmin.x) / 2 - 1.0f;
            p2.y = (rmin.y + rmax.y) / 2;
        }

        if ((pmin.x - rmax.x) > 0)
        {
            p1.x = (rmax.x + pmin.x) / 2.f + (pmin.x - rmax.x) / 4.f;
            p1.y = (pmin.y + pmax.y) / 2.f;
        }
        else
        {
            p1.x = (rmax.x + pmin.x) / 2 + 1.0f;
            p1.y = (pmin.y + pmax.y) / 2;
        }
    }

    molsaver.addArrow(arrow_id, KETReactionArrow::EOpenAngle, p2, p1);
}

void ReactionCdxmlSaver::_addScheme(MoleculeCdxmlSaver& molsaver)
{
    int id = -1;
    Array<char> name;
    PropertiesMap attrs;

    name.clear();
    attrs.clear();

    name.readString("scheme", true);
    molsaver.startCurrentElement(id, name, attrs);
}

void ReactionCdxmlSaver::_closeScheme(MoleculeCdxmlSaver& molsaver)
{
    molsaver.endCurrentElement();
}

void ReactionCdxmlSaver::_addStep(BaseReaction& rxn, MoleculeCdxmlSaver& molsaver, std::vector<int>& mol_ids, std::vector<std::vector<int>>& nodes_ids,
                                  const std::pair<int, int>& arrow_id)
{
    int id = -1;
    Array<char> name;
    PropertiesMap attrs;

    name.clear();
    attrs.clear();

    name.readString("step", true);

    Array<char> buf;
    ArrayOutput buf_out(buf);

    if (arrow_id.second < 0)
    {
        for (auto i = rxn.reactantBegin(); i < rxn.reactantEnd(); i = rxn.reactantNext(i))
        {
            if (mol_ids[i] > 0)
                buf_out.printf("%d ", mol_ids[i]);
        }
    }
    else
    {
        auto& rb = rxn.reactionBlock(arrow_id.second);
        for (auto i : rb.reactants)
            buf_out.printf("%d ", mol_ids[i]);
    }

    if (buf.size() > 1)
    {
        buf.pop();
        buf.push(0);
        attrs.insert("ReactionStepReactants", buf.ptr());
    }

    buf.clear();
    if (arrow_id.second < 0)
    {
        for (auto i = rxn.productBegin(); i < rxn.productEnd(); i = rxn.productNext(i))
        {
            if (mol_ids[i] > 0)
                buf_out.printf("%d ", mol_ids[i]);
        }
    }
    else
    {
        auto& rb = rxn.reactionBlock(arrow_id.second);
        for (auto i : rb.products)
            buf_out.printf("%d ", mol_ids[i]);
    }

    if (buf.size() > 1)
    {
        buf.pop();
        buf.push(0);
        attrs.insert("ReactionStepProducts", buf.ptr());
    }

    std::string below_arrow, above_arrow;
    for (auto i = rxn.catalystBegin(); i < rxn.catalystEnd(); i = rxn.catalystNext(i))
    {
        if (mol_ids[i] > 0)
        {
            if (above_arrow.size())
                above_arrow += " ";
            above_arrow += std::to_string(mol_ids[i]);
        }
    }

    if (above_arrow.size())
        attrs.insert("ReactionStepObjectsAboveArrow", above_arrow.c_str());

    if (below_arrow.size())
        attrs.insert("ReactionStepObjectsAboveArrow", above_arrow.c_str());

    buf.clear();
    buf_out.printf("%d", arrow_id.first);
    buf.push(0);
    attrs.insert("ReactionStepArrows", buf.ptr());

    buf.clear();
    for (auto i = rxn.reactantBegin(); i != rxn.reactantEnd(); i = rxn.reactantNext(i))
    {
        BaseMolecule& mol = rxn.getBaseMolecule(i);

        for (auto j = mol.vertexBegin(); j != mol.vertexEnd(); j = mol.vertexNext(j))
        {
            int aam = rxn.findAamNumber(&mol, j);
            if (aam > 0)
            {
                for (auto k = rxn.productBegin(); k != rxn.productEnd(); k = rxn.productNext(k))
                {
                    int mapped_atom = rxn.findAtomByAAM(k, aam);
                    if (mapped_atom != -1)
                    {
                        buf_out.printf("%d %d ", nodes_ids[i][j], nodes_ids[k][mapped_atom]);
                    }
                }
            }
        }
    }

    if (buf.size() > 1)
    {
        buf.pop();
        buf.push(0);
        attrs.insert("ReactionStepAtomMap", buf.ptr());
        attrs.insert("ReactionStepAtomMapManual", buf.ptr());
    }

    molsaver.addCustomElement(id, name, attrs);
}

void ReactionCdxmlSaver::_generateCdxmlObjIds(BaseReaction& rxn, std::vector<int>& mol_ids, std::vector<int>& meta_ids,
                                              std::vector<std::vector<int>>& nodes_ids)
{
    for (auto i = rxn.begin(); i != rxn.end(); i = rxn.next(i))
    {
        mol_ids.push_back(++_id);

        BaseMolecule& mol = rxn.getBaseMolecule(i);
        nodes_ids.push_back({});

        for (auto j = mol.vertexBegin(); j != mol.vertexEnd(); j = mol.vertexNext(j))
            nodes_ids[i].push_back(++_id);
    }

    // generate ids for meta objects. 1 node and 1 extra object. text or graphics
    for (auto i = 0; i < rxn.meta().metaData().size(); ++i)
    {
        int r_id = i + rxn.end();
        meta_ids.push_back(++_id);
        nodes_ids.push_back({});
        _id += 2;
        nodes_ids[r_id].push_back(_id);
    }
}

void ReactionCdxmlSaver::_addTitle(BaseReaction& rxn, MoleculeCdxmlSaver& molsaver)
{
    Vec2f p(0, 0);
    PropertiesMap attrs;
    attrs.clear();

    Vec2f rmin, rmax;

    if (rxn.reactantsCount() > 0)
    {
        for (auto i = rxn.reactantBegin(); i != rxn.reactantEnd(); i = rxn.reactantNext(i))
        {
            Vec2f min, max;
            _getBounds(rxn.getBaseMolecule(i), min, max, 1.0);
            if (i == rxn.reactantBegin())
            {
                rmin = min;
                rmax = max;
            }
            else
            {
                rmin.min(min);
                rmax.max(max);
            }
        }
    }

    Vec2f pmin, pmax;

    if (rxn.productsCount() > 0)
    {
        for (auto i = rxn.productBegin(); i != rxn.productEnd(); i = rxn.productNext(i))
        {
            Vec2f min, max;
            _getBounds(rxn.getBaseMolecule(i), min, max, 1.0);
            if (i == rxn.productBegin())
            {
                pmin = min;
                pmax = max;
            }
            else
            {
                pmin.min(min);
                pmax.max(max);
            }
        }
    }

    p.x = (rmin.x + pmax.x) / 2 - rxn.name.size() * 0.1f;
    p.y = (rmax.y > pmax.y ? rmax.y : pmax.y) + 1.0f;

    molsaver.addTitle(p, rxn.name.ptr());
}
