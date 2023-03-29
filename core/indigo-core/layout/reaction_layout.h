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

#ifndef __reaction_layout__

#include "layout/metalayout.h"
#include "reaction/base_reaction.h"

namespace indigo
{

    class Reaction;
    class Molecule;
    struct Vec2f;

    class ReactionLayout
    {
    public:
        explicit ReactionLayout(BaseReaction& r, bool smart_layout = false);

        void make();

        float bond_length;
        float plus_interval_factor;
        float arrow_interval_factor;
        float horizontal_interval_factor;
        bool preserve_molecule_layout;
        int max_iterations;
        bool _smart_layout;
        layout_orientation_value layout_orientation;

    private:
        Metalayout::LayoutItem& _pushMol(Metalayout::LayoutLine& line, int id, bool is_agent = false);
        Metalayout::LayoutItem& _pushSpace(Metalayout::LayoutLine& line, float size);
        BaseMolecule& _getMol(int id);
        void _shiftMol(const Metalayout::LayoutItem& item, const Vec2f& pos);
        void _make();

        static BaseMolecule& cb_getMol(int id, void* context);
        static void cb_process(Metalayout::LayoutItem& item, const Vec2f& pos, void* context);

        ReactionLayout(const ReactionLayout& r); // no implicit copy

        BaseReaction& _r;
        Metalayout _ml;
    };

} // namespace indigo

#endif
