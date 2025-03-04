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
        static constexpr float DEFAULT_VER_INTERVAL_FACTOR = 2.5f;

        const Vec2f MOL_COMPONENT_INTERVAL{0.5f, 0.5f};

        void make();

        void makePathwayFromSimple();

        // layout if reaction components are not in the places
        static bool hasAnyIntersect(const std::vector<Rect2f>& bblist);
        static bool validVerticalRange(const std::vector<Rect2f>& bblist);

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

        inline float ReactionMarginSize() const
        {
            return reaction_margin_size + (_font_size < EPSILON ? atom_label_margin : 0);
        };

    private:
        struct SweepEvent
        {
            float x;
            bool is_start;
            float y_start, y_end;

            bool operator<(const SweepEvent& other) const
            {
                if (x != other.x)
                    return x < other.x;
                return is_start > other.is_start;
            }
        };

        void _makePathway();
        void _updateMetadata();
        void _pushMol(Metalayout::LayoutLine& line, int id, bool is_agent = false);
        void _pushSpace(Metalayout::LayoutLine& line, float size);
        BaseMolecule& _getMol(int id);

        static BaseMolecule& cb_getMol(int id, void* context);
        static void cb_process(Metalayout::LayoutItem& item, const Vec2f& pos, void* context);

        ReactionLayout(const ReactionLayout& r); // no implicit copy

        const float bond_length; // in angstrom
        const float atom_label_margin;
        const float default_plus_size;
        const float default_arrow_size;
        const float reaction_margin_size;
        bool preserve_molecule_layout = false;
        int max_iterations = 0;
        bool _smart_layout = false;
        LAYOUT_ORIENTATION layout_orientation = UNCPECIFIED;
        LayoutOptions _options;
        BaseReaction& _r;
        Metalayout _ml;
        const float _font_size;
    };

} // namespace indigo

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
