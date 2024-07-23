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

namespace indigo
{

    IMPL_ERROR(PathwayReactionJsonSaver, "pathway reaction KET saver");

    PathwayReactionJsonSaver::PathwayReactionJsonSaver(Output& output) : _output(output), add_stereo_desc(), pretty_json()
    {
    }

    void PathwayReactionJsonSaver::saveReaction(PathwayReaction& rxn)
    {
        auto merged = std::make_unique<Molecule>();
        auto reaction = std::make_unique<PathwayReaction>();
        reaction->clone(rxn);

        constexpr int SPACE = 5;
        int offsetY = -1;
        float offsetX;
        int side;
        for (int i = reaction->begin(); i < reaction->end(); i = reaction->next(i))
        {
            if (offsetY != reaction->getReactionId(i) * -SPACE * 2)
            {
                offsetX = 0;
                side = BaseReaction::REACTANT;
            }
            offsetY = reaction->getReactionId(i) * -SPACE * 2;
            if (side != reaction->getSideType(i))
            {
                offsetX += SPACE;
                side = reaction->getSideType(i);
            }

            auto& mol = reaction->getBaseMolecule(i);
            Rect2f box;
            mol.getBoundingBox(box);
            auto normOffset = box.center();
            normOffset.negate();
            offsetX += box.width() / 2;
            for (int j = mol.vertexBegin(); j != mol.vertexEnd(); j = mol.vertexNext(j))
            {
                Vec3f& xyz = mol.getAtomXyz(j);
                xyz.add(normOffset);
                xyz.x += offsetX;
                xyz.y += offsetY;
            }
            merged->mergeWithMolecule(mol, 0, 0);
            offsetX += box.width() / 2;
            offsetX += SPACE;
        }

        rapidjson::StringBuffer s;
        JsonWriter writer(pretty_json);
        writer.Reset(s);
        MoleculeJsonSaver json_saver(_output);
        json_saver.add_stereo_desc = add_stereo_desc;
        json_saver.saveMolecule(*merged, writer);
        _output.printf("%s", s.GetString());
    }

} // namespace indigo
