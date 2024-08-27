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

#ifndef __reaction_multistep_detector__
#define __reaction_multistep_detector__

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include <vector>
#include <utility>
#include <list>

#include "base_cpp/exception.h"
#include "molecule/ket_commons.h"

namespace indigo
{

    class BaseMolecule;
    class BaseReaction;

    class ReactionMultistepDetector
    {
    public:
        ReactionMultistepDetector(BaseMolecule& mol);
        ~ReactionMultistepDetector();
        void buildReaction(BaseReaction& rxn);
        typedef std::pair<float, int> FLOAT_INT_PAIR;
        typedef std::vector<FLOAT_INT_PAIR> FLOAT_INT_PAIRS;
        const Vec2f PLUS_BBOX_SHIFT = {0.9f, 0.9f};
        const Vec2f ARROW_BBOX_SHIFT = {0.0f, 0.9f};

        DECL_ERROR;

    private:
        void constructMultipleArrowReaction(BaseReaction& rxn);
        bool findPlusNeighbours(const Vec2f& plus_pos, const FLOAT_INT_PAIRS& mol_tops, const FLOAT_INT_PAIRS& mol_bottoms, const FLOAT_INT_PAIRS& mol_lefts,
                                const FLOAT_INT_PAIRS& mol_rights, std::pair<int, int>& connection);

        BaseMolecule& _bmol;
        std::vector<ReactionComponent> _reaction_components;
        std::vector<MolSumm> _component_summ_blocks;
        std::list<MolSumm> _component_summ_blocks_list;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
