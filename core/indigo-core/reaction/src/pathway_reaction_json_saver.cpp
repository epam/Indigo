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

#include <queue>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "molecule/inchi_wrapper.h"
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

        std::vector<std::string> inchiKeys(reaction->reactionsCount());
        InchiWrapper inchiWrapper;
        Array<char> inchi, inchiKey;
        for (int i = reaction->begin(); i < reaction->end(); i = reaction->next(i))
        {
            auto& molecule = dynamic_cast<Molecule&>(reaction->getBaseMolecule(i));
            inchiWrapper.saveMoleculeIntoInchi(molecule, inchi);
            InchiWrapper::InChIKey(inchi.ptr(), inchiKey);
            inchiKeys.at(i).assign(inchiKey.ptr(), inchiKey.size());
        }

        int finalProductId;
        std::vector<std::vector<int>> reactantIdsByReactions(reaction->reactionsCount());
        std::unordered_map<std::string, int> productIds;
        for (int i = reaction->begin(); i < reaction->end(); i = reaction->next(i))
        {
            if (BaseReaction::REACTANT == reaction->getSideType(i))
                reactantIdsByReactions.at(reaction->reactionId(i)).push_back(i);
            else if (BaseReaction::PRODUCT == reaction->getSideType(i))
            {
                productIds.emplace(inchiKeys.at(i), i);
                finalProductId = i;
            }
        }

        std::unordered_map<int, Vec2f> points;
        points.reserve(reaction->reactionsCount());
        constexpr int SPACE = 5;
        constexpr float NARROWING_QUOTIENT = 0.8f;
        float multiplierY = 1.f;
        std::queue<int> q;
        q.push(finalProductId);
        while (!q.empty())
        {
            auto size = q.size();
            for (size_t i = 0; i < size; i++)
            {
                auto id = q.front();
                q.pop();

                auto productIter = productIds.find(inchiKeys.at(id));
                if (productIter == productIds.cend())
                    continue;

                auto zero = points[id];
                id = productIter->second;
                float offsetY = reactantIdsByReactions[reaction->reactionId(id)].size() > 1 ? -2.f * SPACE : 0;
                offsetY *= multiplierY;
                for (int reactantId : reactantIdsByReactions[reaction->reactionId(id)])
                {
                    points[reactantId] = zero - Vec2f(3 * SPACE, offsetY);
                    offsetY += 4 * SPACE * multiplierY;
                    q.push(reactantId);
                }
                multiplierY *= NARROWING_QUOTIENT;
            }
        }

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

} // namespace indigo
