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

#include <memory>
#include <vector>

#include <rapidjson/document.h>

#include "base_cpp/output.h"
#include "molecule/molecule_json_saver.h"
#include "molecule/query_molecule.h"
#include "reaction/reaction.h"
#include "reaction/reaction_json_saver.h"

using namespace indigo;
using namespace indigo;
using namespace rapidjson;

IMPL_ERROR(ReactionJsonSaver, "reaction KET saver");

void ReactionJsonSaver::_getBounds(BaseMolecule& mol, Vec2f& min_vec, Vec2f& max_vec, float scale)
{
    mol.getBoundingBox(min_vec, max_vec);
    min_vec.scale(scale);
    max_vec.scale(scale);
}

ReactionJsonSaver::ReactionJsonSaver(Output& output) : _output(output), add_stereo_desc(false), pretty_json(false)
{
}

ReactionJsonSaver::~ReactionJsonSaver()
{
}

void ReactionJsonSaver::saveReactionWithMetaData(BaseReaction& rxn, BaseMolecule& merged, MoleculeJsonSaver& json_saver)
{
    for (int i = rxn.begin(); i != rxn.end(); i = rxn.next(i))
        merged.mergeWithMolecule(rxn.getBaseMolecule(i), 0, 0);

    merged.meta().clone(rxn.meta());
    StringBuffer s;
    JsonWriter writer(pretty_json);
    writer.Reset(s);
    json_saver.saveMolecule(merged, writer);
    _output.printf("%s", s.GetString());
}

void ReactionJsonSaver::saveReaction(BaseReaction& rxn, BaseMolecule& merged, MoleculeJsonSaver& json_saver)
{
    std::vector<Vec2f> pluses;
    Vec2f rmin(0, 0), rmax(0, 0), pmin(0, 0), pmax(0, 0), cmin(0, 0), cmax(0, 0);
    bool last_single_reactant = false;
    bool first_single_product = false;

    if (rxn.reactantsCount() > 0)
    {
        int rcount = 1;
        for (int i = rxn.reactantBegin(); i != rxn.reactantEnd(); i = rxn.reactantNext(i))
        {
            Vec2f min1, max1;
            _getBounds(rxn.getBaseMolecule(i), min1, max1, 1.0);
            merged.mergeWithMolecule(rxn.getBaseMolecule(i), 0, 0);

            if (i == rxn.reactantBegin())
            {
                rmin = min1;
                rmax = max1;
            }
            else
            {
                rmin.min(min1);
                rmax.max(max1);
            }

            if (rcount < rxn.reactantsCount())
            {
                Vec2f min2, max2;
                _getBounds(rxn.getBaseMolecule(rxn.reactantNext(i)), min2, max2, 1.0);
                pluses.emplace_back((max1.x + min2.x) / 2, (min1.y + max1.y) / 2);
                rcount++;
            }
            last_single_reactant = rxn.getBaseMolecule(i).vertexCount() == 1;
        }
    }

    if (rxn.productsCount() > 0)
    {
        int rcount = 1;
        Vec2f min1, max1;

        for (int i = rxn.productBegin(); i != rxn.productEnd(); i = rxn.productNext(i))
        {
            Vec2f min1, max1;
            _getBounds(rxn.getBaseMolecule(i), min1, max1, 1.0);
            merged.mergeWithMolecule(rxn.getBaseMolecule(i), 0, 0);

            if (i == rxn.productBegin())
            {
                pmin = min1;
                pmax = max1;
                first_single_product = rxn.getBaseMolecule(i).vertexCount() == 1;
            }
            else
            {
                pmin.min(min1);
                pmax.max(max1);
            }

            if (rcount < rxn.productsCount())
            {
                Vec2f min2, max2;
                _getBounds(rxn.getBaseMolecule(rxn.productNext(i)), min2, max2, 1.0);
                pluses.emplace_back((max1.x + min2.x) / 2, (min1.y + max1.y) / 2);
                rcount++;
            }
        }
    }

    if (rxn.catalystCount() > 0)
    {
        for (int i = rxn.catalystBegin(); i != rxn.catalystEnd(); i = rxn.catalystNext(i))
        {
            Vec2f min1, max1;
            _getBounds(rxn.getBaseMolecule(i), min1, max1, 1.0);
            merged.mergeWithMolecule(rxn.getBaseMolecule(i), 0, 0);
            if (i == rxn.catalystBegin())
            {
                cmin = min1;
                cmax = max1;
            }
            else
            {
                cmin.min(min1);
                cmax.max(max1);
            }
        }
    }

    // dump molecules
    StringBuffer s;
    JsonWriter writer(pretty_json);
    writer.Reset(s);
    json_saver.saveMolecule(merged, writer);

    Document ket;
    ket.Parse(s.GetString());
    if (!(ket.HasMember("root") && ket["root"].HasMember("nodes")))
        throw Error("reaction_json_saver: MoleculeJsonSaver::saveMolecule failed");

    auto& nodes = ket["root"]["nodes"];

    for (const auto& plus_offset : pluses)
    {
        Value plus(kObjectType);
        plus.AddMember("type", "plus", ket.GetAllocator());
        Value location(kArrayType);
        location.PushBack(Value().SetDouble(plus_offset.x), ket.GetAllocator());
        location.PushBack(Value().SetDouble(plus_offset.y), ket.GetAllocator());
        location.PushBack(Value().SetDouble(0.0), ket.GetAllocator());
        plus.AddMember("location", location, ket.GetAllocator());
        nodes.PushBack(plus, ket.GetAllocator());
    }

    // calculate arrow
    Vec2f arrow_head(0, 0);
    Vec2f arrow_tail(0, 0);
    if (rxn.reactantsCount() || rxn.productsCount())
    {
        if (rxn.productsCount() == 0)
        {
            arrow_tail.x = rmax.x + 1.0f;
            arrow_tail.y = (rmin.y + rmax.y) / 2;
            arrow_head.x = arrow_tail.x + 1.0f;
            arrow_head.y = arrow_tail.y;
        }
        else if (rxn.reactantsCount() == 0)
        {
            arrow_head.x = pmin.x - 1.0f;
            arrow_head.y = (pmin.y + pmax.y) / 2;
            arrow_tail.x = arrow_head.x - 1.0f;
            arrow_tail.y = arrow_head.y;
        }
        else
        {
            double ptab = first_single_product ? 2.0f : 1.0;
            double rtab = last_single_reactant ? 2.0f : 1.0;

            arrow_head.y = (pmin.y + pmax.y) / 2;
            arrow_tail.y = (rmin.y + rmax.y) / 2;

            if (pmin.x > rmax.x)
            {
                arrow_head.x = pmin.x - ptab;
                arrow_tail.x = rmax.x + rtab;
            }
            else
            {
                arrow_head.x = rmax.x + rtab;
                arrow_tail.x = pmin.x - ptab;
            }
        }

        Value arrow(kObjectType);
        arrow.AddMember("type", "arrow", ket.GetAllocator());
        Value data(kObjectType);
        Value pos_array(kArrayType);
        Value pos1(kObjectType);
        Value pos2(kObjectType);
        pos1.AddMember("x", Value().SetDouble(arrow_head.x), ket.GetAllocator());
        pos1.AddMember("y", Value().SetDouble(arrow_head.y), ket.GetAllocator());
        pos1.AddMember("z", Value().SetDouble(0.0), ket.GetAllocator());
        pos2.AddMember("x", Value().SetDouble(arrow_tail.x), ket.GetAllocator());
        pos2.AddMember("y", Value().SetDouble(arrow_tail.y), ket.GetAllocator());
        pos2.AddMember("z", Value().SetDouble(0.0), ket.GetAllocator());
        pos_array.PushBack(pos2, ket.GetAllocator());
        pos_array.PushBack(pos1, ket.GetAllocator());
        data.AddMember("mode", "open-angle", ket.GetAllocator());
        data.AddMember("pos", pos_array, ket.GetAllocator());
        arrow.AddMember("data", data, ket.GetAllocator());
        nodes.PushBack(arrow, ket.GetAllocator());
        s.Clear();
        writer.Reset(s);
        ket.Accept(writer);

        _output.printf("%s", s.GetString());
    }
    else
        throw Error("Empty reaction");
}

void ReactionJsonSaver::saveReaction(BaseReaction& rxn)
{
    MoleculeJsonSaver json_saver(_output);
    json_saver.add_stereo_desc = add_stereo_desc;
    std::unique_ptr<BaseMolecule> merged;
    if (rxn.isQueryReaction())
    {
        merged = std::make_unique<QueryMolecule>();
    }
    else
    {
        merged = std::make_unique<Molecule>();
    }

    int arrows_count = rxn.meta().getMetaCount(KETReactionArrow::CID);
    int simple_count = rxn.meta().getMetaCount(KETSimpleObject::CID) + rxn.meta().getMetaCount(KETTextObject::CID);
    if (arrows_count || simple_count)
    {
        // if metadata presents
        saveReactionWithMetaData(rxn, *merged, json_saver);
    }
    else
    {
        saveReaction(rxn, *merged, json_saver);
    }
}
