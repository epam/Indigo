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
#include <array>
#include <iterator>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "reaction/query_reaction.h"
#include "reaction/reaction.h"
#include "reaction/reaction_json_loader.h"
#include "reaction/reaction_multistep_detector.h"

using namespace indigo;
using namespace rapidjson;

IMPL_ERROR(ReactionJsonLoader, "reaction KET loader");

ReactionJsonLoader::ReactionJsonLoader(Document& ket)
    : _loader(ket), _molecule(kArrayType), _prxn(nullptr), _pqrxn(nullptr), ignore_noncritical_query_features(false)
{
    ignore_bad_valence = false;
}

ReactionJsonLoader::~ReactionJsonLoader()
{
}

void ReactionJsonLoader::loadReaction(BaseReaction& rxn)
{
    _loader.stereochemistry_options = stereochemistry_options;
    _loader.ignore_noncritical_query_features = ignore_noncritical_query_features;
    _loader.treat_x_as_pseudoatom = treat_x_as_pseudoatom;
    _loader.ignore_no_chiral_flag = ignore_no_chiral_flag;

    if (rxn.isQueryReaction())
        _pqrxn = &rxn.asQueryReaction();
    else
        _prxn = &rxn.asReaction();

    if (_prxn)
    {
        _pmol = &_mol;
        _loader.loadMolecule(_mol, true);
    }
    else if (_pqrxn)
    {
        _loader.loadMolecule(_qmol, true);
        _pmol = &_qmol;
    }
    else
        throw Error("unknown reaction type: %s", typeid(rxn).name());

    rxn.original_format = BaseMolecule::KET;

    rxn.meta().clone(_pmol->meta());
    _pmol->meta().resetMetaData();

    int arrow_count = rxn.meta().getMetaCount(KETReactionArrow::CID);
    if (arrow_count == 0)
        throw Error("No arrow in the reaction");

    if (arrow_count > 1)
    {
        ReactionMultistepDetector md(*_pmol);
        md.buildReaction(rxn);
    }
    else
        parseOneArrowReaction(rxn);
}

void ReactionJsonLoader::parseOneArrowReaction(BaseReaction& rxn)
{
    enum RecordIndexes
    {
        BBOX_IDX = 0,
        FRAGMENT_TYPE_IDX,
        MOLECULE_IDX
    };

    enum class ReactionFragmentType
    {
        MOLECULE,
        PLUS,
        ARROW,
        TEXT
    };

    using ReactionComponent = std::tuple<Rect2f, ReactionFragmentType, std::unique_ptr<BaseMolecule>>;

    std::vector<ReactionComponent> components;

    std::list<std::unordered_set<int>> s_neighbors;
    getSGroupAtoms(*_pmol, s_neighbors);

    for (int index = 0; index < _pmol->countComponents(s_neighbors); ++index)
    {
        std::unique_ptr<BaseMolecule> mol;
        if (_pmol->isQueryMolecule())
            mol = std::make_unique<QueryMolecule>();
        else
            mol = std::make_unique<Molecule>();

        Filter filter(_pmol->getDecomposition().ptr(), Filter::EQ, index);

        mol->makeSubmolecule(*_pmol, filter, 0, 0);
        Rect2f bbox;
        mol->getBoundingBox(bbox);
        components.emplace_back(bbox, ReactionFragmentType::MOLECULE, std::move(mol));
    }

    auto& arrow = (const KETReactionArrow&)rxn.meta().getMetaObject(KETReactionArrow::CID, 0);
    bool reverseReactionOrder = arrow.getArrowType() == KETReactionArrow::ERetrosynthetic;

    if (reverseReactionOrder)
        rxn.setIsRetrosyntetic();

    for (int i = 0; i < rxn.meta().getMetaCount(KETTextObject::CID); ++i)
    {
        auto& text = (const KETTextObject&)rxn.meta().getMetaObject(KETTextObject::CID, i);
        Rect2f bbox(Vec2f(text._pos.x, text._pos.y), Vec2f(text._pos.x, text._pos.y)); // change to real text box later
        components.emplace_back(bbox, ReactionFragmentType::TEXT, nullptr);
    }

    int text_meta_idx = 0;
    for (const auto& comp : components)
    {
        switch (std::get<FRAGMENT_TYPE_IDX>(comp))
        {
        case ReactionFragmentType::MOLECULE: {
            auto& cmol = *std::get<MOLECULE_IDX>(comp);
            for (int idx = cmol.vertexBegin(); idx < cmol.vertexEnd(); idx = cmol.vertexNext(idx))
            {
                Vec3f& pt3d = cmol.getAtomXyz(idx);
                Vec2f pt(pt3d.x, pt3d.y);
                int side = !reverseReactionOrder ? getPointSide(pt, arrow.getTail(), arrow.getHead()) : getPointSide(pt, arrow.getHead(), arrow.getTail());
                switch (side)
                {
                case KETReagentUpArea:
                case KETReagentDownArea:
                    rxn.addCatalystCopy(cmol, 0, 0);
                    break;
                case KETProductArea:
                    rxn.addProductCopy(cmol, 0, 0);
                    break;
                default:
                    rxn.addReactantCopy(cmol, 0, 0);
                    break;
                }
                break;
            }
        }
        break;
        case ReactionFragmentType::TEXT: {
            const auto& bbox = std::get<BBOX_IDX>(comp);
            Vec2f pt(bbox.center());
            int side = !reverseReactionOrder ? getPointSide(pt, arrow.getTail(), arrow.getHead()) : getPointSide(pt, arrow.getHead(), arrow.getTail());
            if (side == KETReagentUpArea || side == KETReagentDownArea)
            {
                rxn.addSpecialCondition(text_meta_idx, bbox);
                break;
            }
            text_meta_idx++;
        }
        break;
        default:
            break;
        }
    }
}
