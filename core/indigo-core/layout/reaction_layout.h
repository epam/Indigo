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
#define __reaction_layout__

#include "layout/metalayout.h"
#include "reaction/base_reaction.h"

#ifdef _MSC_VER
#pragma warning(push, 4)
#endif

namespace indigo
{

    class Reaction;
    class Molecule;
    struct Vec2f;

    class ReactionLayout
    {
    public:
        explicit ReactionLayout(BaseReaction& r, bool smart_layout = false);
        explicit ReactionLayout(BaseReaction& r, bool smart_layout, const LayoutOptions& options);

        static constexpr float DEFAULT_HOR_INTERVAL_FACTOR = 1.4f;

        void make();

        // layout if reaction components are not in the places
        void fixLayout();
        void processSideBoxes(std::vector<Vec2f>& pluses, Rect2f& type_box, int side);

        void setMaxIterations(int count)
        {
            max_iterations = count;
        };

        void setLayoutOrientation(LAYOUT_ORIENTATION orientation)
        {
            layout_orientation = orientation;
        };

        void setPreserveMoleculeLayout(bool preserve)
        {
            preserve_molecule_layout = preserve;
        };

    private:
        void _updateMetadata();
        void _pushMol(Metalayout::LayoutLine& line, int id, bool is_agent = false);
        void _pushSpace(Metalayout::LayoutLine& line, float size);
        BaseMolecule& _getMol(int id);

        static BaseMolecule& cb_getMol(int id, void* context);
        static void cb_process(Metalayout::LayoutItem& item, const Vec2f& pos, void* context);

        ReactionLayout(const ReactionLayout& r); // no implicit copy

        const float bond_length;
        const float atom_label_margin;
        const float default_plus_size;
        const float default_arrow_size;
        const float reaction_margin_size;
        bool preserve_molecule_layout = false;
        int max_iterations = 0;
        bool _smart_layout = false;
        LAYOUT_ORIENTATION layout_orientation = UNCPECIFIED;

        BaseReaction& _r;
        Metalayout _ml;
    };

} // namespace indigo

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
