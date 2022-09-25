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

#ifndef __reaction_json_loader__
#define __reaction_json_loader__

#include <list>
#include <rapidjson/document.h>

#include "base_cpp/exception.h"
#include "molecule/ket_commons.h"
#include "molecule/molecule.h"
#include "molecule/molecule_json_loader.h"
#include "molecule/molecule_stereocenter_options.h"
#include "molecule/query_molecule.h"

namespace indigo
{
    inline void merge_bbox(Rect2f& bb1, const Rect2f& bb2)
    {
        Vec2f lb, rt;
        lb.x = std::min(bb1.left(), bb2.left());
        rt.x = std::max(bb1.right(), bb2.right());
        lb.y = std::min(bb1.bottom(), bb2.bottom());
        rt.y = std::max(bb1.top(), bb2.top());
        bb1 = Rect2f(lb, rt);
    }

    class Scanner;
    class BaseReaction;
    class Reaction;
    class QueryReaction;
    class QueryMolecule;
    class MoleculeJsonLoader;

    class ReactionJsonLoader
    {
    public:
        DECL_ERROR;

        typedef std::pair<float, int> FLOAT_INT_PAIR;
        typedef std::vector<FLOAT_INT_PAIR> FLOAT_INT_PAIRS;

        ReactionJsonLoader(rapidjson::Document& ket);
        ~ReactionJsonLoader();

        void loadReaction(BaseReaction& rxn);

        StereocentersOptions stereochemistry_options;
        bool ignore_bad_valence;
        bool ignore_noncritical_query_features;
        bool treat_x_as_pseudoatom;
        bool ignore_no_chiral_flag;

        const Vec2f PLUS_BBOX_SHIFT = {0.9, 0.9};
        const Vec2f ARROW_BBOX_SHIFT = {0.0, 0.9};

    private:
        ReactionJsonLoader(const ReactionJsonLoader&); // no implicit copy
        void parseOneArrowReaction(BaseReaction& rxn);
        void parseMultipleArrowReaction(BaseReaction& rxn);
        void constructMultipleArrowReaction(BaseReaction& rxn);
        bool findPlusNeighbours(const Vec2f& plus_pos, const FLOAT_INT_PAIRS& mol_tops, const FLOAT_INT_PAIRS& mol_bottoms, const FLOAT_INT_PAIRS& mol_lefts,
                                const FLOAT_INT_PAIRS& mol_rights, std::pair<int, int>& connection);

        rapidjson::Value _molecule;
        MoleculeJsonLoader _loader;

        Reaction* _prxn;
        QueryReaction* _pqrxn;
        Molecule _mol;
        QueryMolecule _qmol;
        BaseMolecule* _pmol;
        std::vector<ReactionComponent> _reaction_components;
        std::vector<MolSumm> _component_summ_blocks;
        std::list<MolSumm> _component_summ_blocks_list;

        std::unordered_map<std::string, int> _arrow_string2type = {
            {"open-angle", ReactionComponent::ARROW_BASIC},
            {"filled-triangle", ReactionComponent::ARROW_FILLED_TRIANGLE},
            {"filled-bow", ReactionComponent::ARROW_FILLED_BOW},
            {"dashed-open-angle", ReactionComponent::ARROW_DASHED},
            {"failed", ReactionComponent::ARROW_FAILED},
            {"both-ends-filled-triangle", ReactionComponent::ARROW_BOTH_ENDS_FILLED_TRIANGLE},
            {"equilibrium-filled-half-bow", ReactionComponent::ARROW_EQUILIBRIUM_FILLED_HALF_BOW},
            {"equilibrium-filled-triangle", ReactionComponent::ARROW_EQUILIBRIUM_FILLED_TRIANGLE},
            {"equilibrium-open-angle", ReactionComponent::ARROW_EQUILIBRIUM_OPEN_ANGLE},
            {"unbalanced-equilibrium-filled-half-bow", ReactionComponent::ARROW_UNBALANCED_EQUILIBRIUM_FILLED_HALF_BOW},
            {"unbalanced-equilibrium-large-filled-half-bow", ReactionComponent::ARROW_UNBALANCED_EQUILIBRIUM_LARGE_FILLED_HALF_BOW},
            {"unbalanced-equilibrium-filled-half-triangle", ReactionComponent::ARROW_BOTH_ENDS_FILLED_TRIANGLE}};
    };

} // namespace indigo

#endif /* reaction_json_loader_h */
