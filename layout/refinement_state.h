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

#ifndef __refinement_state_h__
#define __refinement_state_h__

#include "layout/molecule_layout_graph.h"

namespace indigo
{

    struct RefinementState
    {
        explicit RefinementState(MoleculeLayoutGraph& graph);

        void calcHeight();
        void calcDistance(int v1, int v2);
        void calcEnergy();

        void copy(const RefinementState& other); // existing states
        void copyFromGraph();
        void applyToGraph();

        void flipBranch(const Filter& branch, const RefinementState& state, int v1_idx, int v2_idx);
        void rotateBranch(const Filter& branch, const RefinementState& state, int v_idx, float angle);
        void stretchBranch(const Filter& branch, const RefinementState& state, int v1, int v2, int d);
        void rotateLayout(const RefinementState& state, int v_idx, float angle);
        bool is_small_cycle();
        float calc_best_angle();

        float dist;
        double energy;
        float height;
        CP_DECL;
        TL_CP_DECL(Array<Vec2f>, layout);

        DECL_ERROR;

    private:
        MoleculeLayoutGraph& _graph;
    };

} // namespace indigo

#endif
