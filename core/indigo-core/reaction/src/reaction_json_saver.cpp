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
    for (int i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
    {
        Vec3f& p = mol.getAtomXyz(i);
        Vec2f p2(p.x, p.y);

        if (i == mol.vertexBegin())
            min_vec = max_vec = p2;
        else
        {
            min_vec.min(p2);
            max_vec.max(p2);
        }
    }

    min_vec.scale(scale);
    max_vec.scale(scale);
}

ReactionJsonSaver::ReactionJsonSaver(Output& output) : _output(output), _add_stereo_desc(false)
{
}

ReactionJsonSaver::~ReactionJsonSaver()
{
}

void ReactionJsonSaver::saveReactionWithMetadata(BaseReaction& rxn, BaseMolecule& merged, MoleculeJsonSaver& json_saver)
{

   for (int i = rxn.begin(); i != rxn.end(); i = rxn.next(i))
        merged.mergeWithMolecule(rxn.getBaseMolecule(i), 0, 0);

    merged.cloneMetaData(rxn);

    StringBuffer s;
    Writer<StringBuffer> writer(s);
    json_saver.saveMolecule(merged, writer);

    Document ket;
    ket.Parse(s.GetString());
    if (!(ket.HasMember("root") && ket["root"].HasMember("nodes")))
        throw Error("reaction_json_saver: MoleculeJsonSaver::saveMolecule failed");

    auto& nodes = ket["root"]["nodes"];
    auto& md = rxn.metaData();
    for (int meta_index = 0; meta_index < md.size(); ++meta_index)
    {
        auto pobj = md[meta_index];
        switch (pobj->_class_id)
        {
        case KETReactionArrow::cid: {
            KETReactionArrow& ar = (KETReactionArrow&)(*pobj);
            Value arrow(kObjectType);
            arrow.AddMember("type", "arrow", ket.GetAllocator());
            Value data(kObjectType);
            Value pos_array(kArrayType);
            Value pos1(kObjectType);
            Value pos2(kObjectType);
            pos1.AddMember("x", Value().SetDouble(ar._begin.x), ket.GetAllocator());
            pos1.AddMember("y", Value().SetDouble(-ar._begin.y), ket.GetAllocator());
            pos1.AddMember("z", Value().SetDouble(0.0), ket.GetAllocator());
            pos2.AddMember("x", Value().SetDouble(ar._begin.x), ket.GetAllocator());
            pos2.AddMember("y", Value().SetDouble(-ar._begin.y), ket.GetAllocator());
            pos2.AddMember("z", Value().SetDouble(0.0), ket.GetAllocator());
            pos_array.PushBack(pos2, ket.GetAllocator());
            pos_array.PushBack(pos1, ket.GetAllocator());
            std::string arrow_mode = "open-angle";
            auto at_it = _arrow_type2string.find(ar._arrow_type);
            if (at_it != _arrow_type2string.end())
                arrow_mode = at_it->second;
            data.AddMember("mode", StringRef(arrow_mode.c_str()), ket.GetAllocator());
            data.AddMember("pos", pos_array, ket.GetAllocator());
            arrow.AddMember("data", data, ket.GetAllocator());
            nodes.PushBack(arrow, ket.GetAllocator());
        }
        break;
        case KETReactionPlus::cid: {
            KETReactionPlus& rp = (KETReactionPlus&)(*pobj);
            Value plus(kObjectType);
            plus.AddMember("type", "plus", ket.GetAllocator());
            Value location(kArrayType);
            location.PushBack(Value().SetDouble(rp._pos.x), ket.GetAllocator());
            location.PushBack(Value().SetDouble(-rp._pos.y), ket.GetAllocator());  // TODO: remove -
            location.PushBack(Value().SetDouble(0.0), ket.GetAllocator());         // TODO: remove -
            plus.AddMember("location", location, ket.GetAllocator());
            nodes.PushBack(plus, ket.GetAllocator());
        }
        break;
        }
    }
    s.Clear();
    writer.Reset(s);
    ket.Accept(writer);
    _output.printf("%s", s.GetString());

}

void ReactionJsonSaver::saveSingleReaction(BaseReaction& rxn, BaseMolecule& merged, MoleculeJsonSaver& json_saver)
{
    std::vector<Vec2f> pluses;

    Vec2f rmin(0, 0), rmax(0, 0), pmin(0, 0), pmax(0, 0);

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

    // dump molecules
    merged.cloneMetaData(rxn);
    StringBuffer s;
    Writer<StringBuffer> writer(s);
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
        location.PushBack(Value().SetDouble(-plus_offset.y), ket.GetAllocator()); // TODO: remove -
        location.PushBack(Value().SetDouble(0.0), ket.GetAllocator());
        plus.AddMember("location", location, ket.GetAllocator());
        nodes.PushBack(plus, ket.GetAllocator());
    }

    // calculate arrow
    Vec2f p1(0, 0);
    Vec2f p2(0, 0);
    if (rxn.reactantsCount() || rxn.productsCount())
    {
        if (rxn.productsCount() == 0)
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
                p2.x = (rmax.x + pmin.x) / 2 - (pmin.x - rmax.x) / 8;
                p2.y = (rmin.y + rmax.y) / 2;
            }
            else
            {
                p2.x = (rmax.x + pmin.x) / 2 - 1.0f;
                p2.y = (rmin.y + rmax.y) / 2;
            }

            if ((pmin.x - rmax.x) > 0)
            {
                p1.x = (rmax.x + pmin.x) / 2.f + (pmin.x - rmax.x) / 8.f;
                p1.y = (pmin.y + pmax.y) / 2.f;
            }
            else
            {
                p1.x = (rmax.x + pmin.x) / 2 + 1.0f;
                p1.y = (pmin.y + pmax.y) / 2;
            }
        }

        Value arrow(kObjectType);
        arrow.AddMember("type", "arrow", ket.GetAllocator());
        Value data(kObjectType);
        Value pos_array(kArrayType);
        Value pos1(kObjectType);
        Value pos2(kObjectType);
        pos1.AddMember("x", Value().SetDouble(p1.x), ket.GetAllocator());
        pos1.AddMember("y", Value().SetDouble(-p1.y), ket.GetAllocator());
        pos1.AddMember("z", Value().SetDouble(0.0), ket.GetAllocator());
        pos2.AddMember("x", Value().SetDouble(p2.x), ket.GetAllocator());
        pos2.AddMember("y", Value().SetDouble(-p2.y), ket.GetAllocator());
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
    json_saver._add_stereo_desc = _add_stereo_desc;
    std::unique_ptr<BaseMolecule> merged;
    if (rxn.isQueryReaction())
    {
        merged = std::make_unique<QueryMolecule>();
    }
    else
    {
        merged = std::make_unique<Molecule>();
    }

    if (rxn.metaData().size())
    {
        saveReactionWithMetadata(rxn, *merged, json_saver);
    }
    else
    {
        saveSingleReaction(rxn, *merged, json_saver);
    }
}
