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

#include "reaction/pathway_reaction.h"
#include "reaction/pathway_reaction_builder.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction.h"
#include "reaction/reaction_json_loader.h"
#include "reaction/reaction_multistep_detector.h"

using namespace indigo;
using namespace rapidjson;

IMPL_ERROR(ReactionJsonLoader, "reaction KET loader");

ReactionJsonLoader::ReactionJsonLoader(Document& ket, const LayoutOptions& options)
    : _loader(ket), _molecule(kArrayType), ignore_noncritical_query_features(false), _layout_options(options)
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
    {
        _loader.loadMolecule(_qmol, true);
        _pmol = &_qmol;
    }
    else
    {
        _pmol = &_mol;
        _loader.loadMolecule(_mol, true);
    }

    rxn.original_format = BaseMolecule::KET;

    rxn.meta().clone(_pmol->meta());

    int arrow_count = rxn.meta().getMetaCount(ReactionArrowObject::CID);
    int multi_count = rxn.meta().getMetaCount(ReactionMultitailArrowObject::CID);
    if (arrow_count == 0 && multi_count == 0)
        throw Error("No arrow in the reaction");

    if (arrow_count > 0 || multi_count > 0)
    {
        ReactionMultistepDetector md(*_pmol, _layout_options);
        switch (md.detectReaction())
        {
        case ReactionMultistepDetector::ReactionType::EPathwayReaction:
            md.constructPathwayReaction(static_cast<PathwayReaction&>(rxn));
            PathwayReactionBuilder::buildRootReaction(static_cast<PathwayReaction&>(rxn));
            break;
        case ReactionMultistepDetector::ReactionType::EMutistepReaction:
            md.constructMultipleArrowReaction(rxn);
            break;
        case ReactionMultistepDetector::ReactionType::ESimpleReaction:
            md.constructSimpleArrowReaction(rxn);
            break;
        }
    }
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

    auto& arrow = (const ReactionArrowObject&)rxn.meta().getMetaObject(ReactionArrowObject::CID, 0);
    bool reverseReactionOrder = arrow.getArrowType() == ReactionArrowObject::ERetrosynthetic;

    if (reverseReactionOrder)
        rxn.setIsRetrosyntetic();

    for (int i = 0; i < rxn.meta().getMetaCount(SimpleTextObject::CID); ++i)
    {
        auto& text = (const SimpleTextObject&)rxn.meta().getMetaObject(SimpleTextObject::CID, i);
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
                case KReagentUpArea:
                case KReagentDownArea:
                    rxn.addCatalystCopy(cmol, 0, 0);
                    break;
                case KProductArea:
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
            if (side == KReagentUpArea || side == KReagentDownArea)
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
