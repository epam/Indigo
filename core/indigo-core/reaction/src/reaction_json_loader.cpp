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

#include <algorithm>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "molecule/molecule_json_loader.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction.h"
#include "reaction/reaction_json_loader.h"

using namespace indigo;
using namespace rapidjson;

IMPL_ERROR(ReactionJsonLoader, "reaction KET loader");

ReactionJsonLoader::ReactionJsonLoader(Document& ket) : _rgroups(kArrayType), _molecule(kArrayType), _pluses(kArrayType), _arrows(kArrayType)
{
    ignore_bad_valence = false;
    Value& root = ket["root"];
    Value& nodes = root["nodes"];
    // rewind to first molecule node
    for (int i = 0; i < nodes.Size(); ++i)
    {
        Value& rnode = nodes[i];
        if (rnode.HasMember("$ref"))
        {
            const char* node_name = rnode["$ref"].GetString();
            Value& node = ket[node_name];
            std::string node_type = node["type"].GetString();
            if (node_type == "molecule")
            {
                _molecule.PushBack(node, ket.GetAllocator());
            }
            else if (node_type == "rgroup")
            {
                _rgroups.PushBack(node, ket.GetAllocator());
            }
            else
                throw Error("Unknows JSON node: %s", node_type.c_str());
        }
        else if (rnode.HasMember("type"))
        {
            std::string node_type = rnode["type"].GetString();
            if (node_type == "arrow")
                _arrows.PushBack(rnode, ket.GetAllocator());
            else if (node_type == "plus")
                _pluses.PushBack(rnode, ket.GetAllocator());
            else
                throw Error("Unknown reaction node: %s", node_type.c_str());
        }
        else
            throw Error("Unknows JSON node");
    }
}

ReactionJsonLoader::~ReactionJsonLoader()
{
}

void ReactionJsonLoader::loadReaction(BaseReaction& rxn)
{
    enum RecordIndexes
    {
        LEFT_BOUND_IDX = 0,
        FRAGMENT_TYPE_IDX,
        MOLECULE_IDX
    };

    enum class ReactionFramentType
    {
        MOLECULE,
        PLUS,
        ARROW
    };

    typedef std::tuple<float, ReactionFramentType, std::unique_ptr<BaseMolecule>> ReactionComponent;

    MoleculeJsonLoader loader(_molecule, _rgroups);
    _prxn = dynamic_cast<Reaction*>(&rxn);
    _pqrxn = dynamic_cast<QueryReaction*>(&rxn);

    std::unique_ptr<BaseMolecule> merged_molecule;

    if (_prxn)
    {
        _pmol = &_mol;
        loader.loadMolecule(_mol);
        merged_molecule = std::make_unique<Molecule>();
    }
    else if (_pqrxn)
    {
        loader.loadMolecule(_qmol);
        _pmol = &_qmol;
        merged_molecule = std::make_unique<QueryMolecule>();
    }
    else
        throw Error("unknown reaction type: %s", typeid(rxn).name());

    if (_arrows.Size() > 1)
        throw Error("Multiple arrows are not supported");

    if (_arrows.Size() == 0)
        throw Error("No arrow in the reaction");

    int count = _pmol->countComponents();

    std::vector<ReactionComponent> components;

    for (int index = 0; index < count; ++index)
    {
        if (_pmol->isQueryMolecule())
            components.emplace_back(0, ReactionFramentType::MOLECULE, new QueryMolecule);
        else
            components.emplace_back(0, ReactionFramentType::MOLECULE, new Molecule);
        Filter filter(_pmol->getDecomposition().ptr(), Filter::EQ, index);
        ReactionComponent& rc = components.back();
        BaseMolecule& mol = *(std::get<MOLECULE_IDX>(rc));
        mol.makeSubmolecule(*_pmol, filter, 0, 0);
        Vec2f a, b;
        for (int atom_idx = 0; atom_idx < mol.vertexCount(); ++atom_idx)
        {
            auto vec = mol.getAtomXyz(atom_idx);
            if (!atom_idx)
            {
                a.x = vec.x;
                a.y = vec.y;
                b = a;
            }
            else
            {
                // calculate bounding box
                a.x = std::min(a.x, vec.x);
                a.y = std::min(a.y, vec.y);
                b.x = std::max(b.x, vec.x);
                b.y = std::max(b.y, vec.y);
            }
        }
        std::get<LEFT_BOUND_IDX>(rc) = a.x;
    }

    const rapidjson::Value& arrow_location = _arrows[0]["location"];
    float arrow_x = arrow_location[0].GetFloat();
    components.emplace_back(arrow_x, ReactionFramentType::ARROW, nullptr);
    for (int i = 0; i < _pluses.Size(); ++i)
    {
        const rapidjson::Value& plus = _pluses[i];
        const rapidjson::Value& plus_location = plus["location"];
        components.emplace_back(plus_location[0].GetFloat(), ReactionFramentType::PLUS, nullptr);
    }

    std::sort(components.begin(), components.end(),
              [](const ReactionComponent& a, const ReactionComponent& b) -> bool { return std::get<LEFT_BOUND_IDX>(a) < std::get<LEFT_BOUND_IDX>(b); });

    bool is_arrow_passed = false;
    for (const auto& comp : components)
    {
        switch (std::get<FRAGMENT_TYPE_IDX>(comp))
        {
        case ReactionFramentType::MOLECULE:
            merged_molecule->mergeWithMolecule(*std::get<MOLECULE_IDX>(comp), 0, 0);
            break;
        case ReactionFramentType::ARROW:
            rxn.addReactantCopy(*merged_molecule, 0, 0);
            is_arrow_passed = true;
            merged_molecule->clear();
            break;
        case ReactionFramentType::PLUS:
            if (is_arrow_passed)
                rxn.addProductCopy(*merged_molecule, 0, 0);
            else
                rxn.addReactantCopy(*merged_molecule, 0, 0);
            merged_molecule->clear();
            break;
        }
    }
    rxn.addProductCopy(*merged_molecule, 0, 0);
}
