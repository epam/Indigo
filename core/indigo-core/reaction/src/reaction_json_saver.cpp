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
#include "layout/reaction_layout.h"
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

    std::unique_ptr<BaseReaction> reaction(rxn.neu());
    reaction->clone(rxn);
    ReactionLayout rl(*reaction);
    rl.fixLayout();

    // merge
    for (int i = reaction->begin(); i != reaction->end(); i = reaction->next(i))
        merged->mergeWithMolecule(reaction->getBaseMolecule(i), 0, 0);

    merged->meta().clone(reaction->meta());

    // dump molecules
    StringBuffer s;
    JsonWriter writer(pretty_json);
    writer.Reset(s);
    json_saver.saveMolecule(*merged, writer);

    Document ket;
    ket.Parse(s.GetString());
    if (!(ket.HasMember("root") && ket["root"].HasMember("nodes")))
        throw Error("reaction_json_saver: MoleculeJsonSaver::saveMolecule failed");

    s.Clear();
    writer.Reset(s);
    ket.Accept(writer);
    _output.printf("%s", s.GetString());
}
