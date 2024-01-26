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
#include <numeric>
#include <vector>

#include <rapidjson/document.h>

#include "base_cpp/output.h"
#include "layout/reaction_layout.h"
#include "molecule/molecule_json_saver.h"
#include "molecule/query_molecule.h"
#include "reaction/reaction.h"
#include "reaction/reaction_json_saver.h"

using namespace indigo;
using namespace indigo;
using namespace rapidjson;

IMPL_ERROR(ReactionJsonSaver, "reaction KET saver");

void ReactionJsonSaver::_fixLayout(BaseReaction& rxn)
{
    Vec2f rmax{Vec2f::min_coord(), Vec2f::min_coord()}, pmin{Vec2f::max_coord(), Vec2f::max_coord()};
    Rect2f bb;
    // Calculate rightTop of reactant bounding box
    for (int i = rxn.reactantBegin(); i != rxn.reactantEnd(); i = rxn.reactantNext(i))
    {
        rxn.getBaseMolecule(i).getBoundingBox(bb);
        rmax.max(bb.rightTop());
    }

    // Calculate leftBottom of product bounding box
    for (int i = rxn.productBegin(); i != rxn.productEnd(); i = rxn.productNext(i))
    {
        rxn.getBaseMolecule(i).getBoundingBox(bb);
        pmin.min(bb.leftBottom());
    }

    // if left side of product bb at left of right side of reactant bb - fix layout
    if (rmax.x > pmin.x)
    {
        ReactionLayout rl(rxn, true);
        rl.preserve_molecule_layout = true;
        rl.make();
    }
}

void ReactionJsonSaver::_getBounds(BaseMolecule& mol, Vec2f& min_vec, Vec2f& max_vec, float scale)
{
    Rect2f bbox;
    mol.getBoundingBox(bbox);
    min_vec.copy(bbox.leftBottom());
    max_vec.copy(bbox.rightTop());
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

static void _processSideBoxes(std::unique_ptr<BaseReaction>& reaction, BaseMolecule& merged, std::vector<Vec2f>& pluses, Rect2f& type_box, int side)
{
    int begin = reaction->sideBegin(side);
    std::vector<Rect2f> boxes;

    for (int i = begin; i != reaction->sideEnd(); i = reaction->sideNext(side, i))
    {
        BaseMolecule& mol = reaction->getBaseMolecule(i);
        merged.mergeWithMolecule(mol, 0, 0);

        Rect2f box;
        mol.getBoundingBox(box);
        if (i == begin)
            type_box.copy(box);
        else
            type_box.extend(box);

        if (side != BaseReaction::CATALYST)
            boxes.emplace_back(box);
    }

    // For REACTANT and PRODUCT insert pluses between boxes
    if (side != BaseReaction::CATALYST && boxes.size() > 1)
    {
        std::accumulate(std::next(boxes.begin()), boxes.end(), boxes[0], [&pluses](Rect2f left, Rect2f right) {
            pluses.emplace_back(right.between_left_box(left), left.middleY());
            return right;
        });
    }
}

void ReactionJsonSaver::saveReaction(BaseReaction& rxn, BaseMolecule& merged, MoleculeJsonSaver& json_saver)
{
    std::vector<Vec2f> pluses;
    Rect2f react_box, product_box, catalyst_box;
    bool last_single_reactant = false;
    bool first_single_product = false;
    std::unique_ptr<BaseReaction> reaction(rxn.neu());
    reaction->clone(rxn);
    _fixLayout(*reaction);
    if (reaction->reactantsCount() > 0)
    {
        _processSideBoxes(reaction, merged, pluses, react_box, BaseReaction::REACTANT);
        for (int i = reaction->reactantBegin(); i != reaction->reactantEnd(); i = reaction->reactantNext(i))
        {
            last_single_reactant = reaction->getBaseMolecule(i).vertexCount() == 1;
        }
    }

    if (reaction->productsCount() > 0)
    {
        _processSideBoxes(reaction, merged, pluses, product_box, BaseReaction::PRODUCT);
        first_single_product = reaction->getBaseMolecule(reaction->productBegin()).vertexCount() == 1;
    }

    if (reaction->catalystCount() > 0)
    {
        _processSideBoxes(reaction, merged, pluses, catalyst_box, BaseReaction::CATALYST);
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
    if (reaction->reactantsCount() || reaction->productsCount())
    {
        constexpr float shift = 1.0f;
        if (reaction->productsCount() == 0)
        {
            arrow_tail.x = react_box.right() + shift;
            arrow_tail.y = react_box.middleY();
            arrow_head.x = arrow_tail.x + shift;
            arrow_head.y = arrow_tail.y;
        }
        else if (reaction->reactantsCount() == 0)
        {
            arrow_head.x = product_box.left() - shift;
            arrow_head.y = product_box.middleY();
            arrow_tail.x = arrow_head.x - shift;
            arrow_tail.y = arrow_head.y;
        }
        else
        {
            const float ptab = first_single_product ? 2.0f : 1.0f;
            const float rtab = last_single_reactant ? 2.0f : 1.0f;

            arrow_head.y = product_box.middleY();
            arrow_tail.y = react_box.middleY();

            if (product_box.left() > react_box.right())
            {
                arrow_head.x = product_box.left() - ptab;
                arrow_tail.x = react_box.right() + rtab;
            }
            else
            {
                arrow_head.x = react_box.right() + rtab;
                arrow_tail.x = product_box.left() - ptab;
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
