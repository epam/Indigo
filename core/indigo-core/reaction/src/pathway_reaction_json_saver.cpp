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

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "molecule/molecule.h"
#include "molecule/molecule_json_saver.h"
#include "reaction/pathway_reaction.h"
#include "reaction/pathway_reaction_json_saver.h"

using namespace indigo;

IMPL_ERROR(PathwayReactionJsonSaver, "pathway reaction KET saver");

PathwayReactionJsonSaver::PathwayReactionJsonSaver(Output& output) : _output(output), add_stereo_desc(), pretty_json()
{
}

void PathwayReactionJsonSaver::saveReaction(PathwayReaction& rxn)
{
    auto merged = std::make_unique<Molecule>();
    auto reaction = std::make_unique<PathwayReaction>();
    reaction->clone(rxn);

    std::vector<std::pair<int, Vec2f>> points;
    std::vector<std::vector<Vec2f>> arrows;
    std::tie(points, arrows) = reaction->makeTreePoints();
    // Ensure the same order across different platforms.
    std::sort(points.begin(), points.end());

    for (auto& p : points)
    {
        auto& molecule = reaction->getBaseMolecule(p.first);
        Rect2f box;
        molecule.getBoundingBox(box);
        auto offset = box.center();
        offset.negate();
        offset.add(p.second);
        for (int j = molecule.vertexBegin(); j != molecule.vertexEnd(); j = molecule.vertexNext(j))
        {
            Vec3f& xyz = molecule.getAtomXyz(j);
            xyz.add(offset);
        }
        merged->mergeWithMolecule(molecule, 0, 0);
    }

    rapidjson::StringBuffer buffer;
    JsonWriter writer(pretty_json);
    writer.Reset(buffer);
    MoleculeJsonSaver moleculeSaver(_output);
    moleculeSaver.add_stereo_desc = add_stereo_desc;
    moleculeSaver.saveMolecule(*merged, writer);
    _output.printf("%s", buffer.GetString());
}
