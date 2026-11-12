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

#ifndef __render_sgroups_h__
#define __render_sgroups_h__

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "render_common.h"

namespace indigo
{
    class MoleculeRenderInternal;
    class SGroup;

    // A bracket on the page: drawing coordinates, y down
    struct BracketLine
    {
        Vec2f from, to;
    };

    // The brackets of one S-group and what the drawing rules read off them: the closing bracket, the ends of a
    // bracket and the way its hooks turn
    class SGroupBrackets
    {
    public:
        explicit SGroupBrackets(std::vector<BracketLine> brackets);

        bool empty() const;
        const std::vector<BracketLine>& all() const;
        // The rightmost bracket by its middle; of brackets one above the other, the lower one
        const BracketLine& closing() const;
        // Between the middles of a pair of brackets; none for another number of brackets or a pair in one place
        std::optional<float> pairGap() const;
        // The end lower on the page; of a horizontal bracket, the left end
        static Vec2f lowerEnd(const BracketLine& bracket);
        // The end higher on the page; of a horizontal bracket, the right end
        static Vec2f upperEnd(const BracketLine& bracket);
        // Unit normal to the bracket on the side of `towards`
        static Vec2f hookDirection(const BracketLine& bracket, const Vec2f& towards);

    private:
        static bool _closesLaterThan(const BracketLine& bracket, const BracketLine& other);
        static Vec2f _middle(const BracketLine& bracket);

        std::vector<BracketLine> _brackets;
    };

    // What a drawer of S-groups gets from the drawing of the molecule: where the atoms are on the page, and a place
    // for brackets and text. The items added belong to the S-group being drawn.
    class SGroupCanvas
    {
    public:
        SGroupCanvas(MoleculeRenderInternal& render, Sgroup& items);

        Vec2f atomPosition(int atom) const;
        Rect2f atomBox(int atom) const;
        Vec2f atomsCentre(const Array<int>& atoms) const;
        std::optional<Rect2f> positionsBox(const Array<int>& atoms) const;
        Vec2f toDrawing(const Vec2f& point) const;
        Vec2f toDrawingOffset(const Vec2f& offset) const;
        float lineWidth() const;

        void addBracket(const BracketLine& bracket, const Vec2f& hook_direction, float hook_length);
        void hideBrackets();
        // Measured, not placed: the caller sets bbp, the top left corner
        TextItem& addText(const std::string& text, FONT_SIZE font_size, RenderItem::TYPE type);

    private:
        MoleculeRenderInternal& _render;
        Sgroup& _items;
    };

    // Draws an S-group: its brackets with hooks towards its atoms, its label at the lower end of the closing bracket
    // and its repeat pattern at the upper end. Subclasses change what differs for their type.
    class SGroupRenderer
    {
    public:
        // In bond lengths of the drawing
        static constexpr float LABEL_OFFSET = 0.05f;
        static constexpr float BRACKET_HOOK = 0.15f;

        static constexpr float HOOK_SHARE_OF_BRACKET = 0.15f;
        // The hooks of a narrow pair leave a third of the gap free
        static constexpr float LONGEST_HOOK_SHARE_OF_GAP = 1.0f / 3.0f;

        // nullptr for a type that is not drawn
        static std::unique_ptr<SGroupRenderer> create(const SGroup& sgroup);
        virtual ~SGroupRenderer() = default;

        // `frame` is what a relative data value without atoms is drawn from
        virtual void draw(const SGroup& sgroup, const std::optional<Rect2f>& frame, SGroupCanvas& canvas) const;
        virtual std::optional<Rect2f> frameForChildren(const SGroup& sgroup, const std::optional<Rect2f>& frame, const SGroupCanvas& canvas) const;

    protected:
        virtual SGroupBrackets _brackets(const SGroup& sgroup, const SGroupCanvas& canvas) const;
        virtual float _hookLength(const BracketLine& bracket) const;
        virtual float _labelOffset(const SGroupCanvas& canvas) const;
        virtual void _placeLabel(TextItem& label, const Vec2f& lower_end, float offset) const;
        virtual std::string _label(const SGroup& sgroup) const;
        virtual std::string _repeatPattern(const SGroup& sgroup) const;

        static std::string _connectivityText(const SGroup& sgroup);
        static float _hookOfBracketLength(const BracketLine& bracket);
        static SGroupBrackets _storedBrackets(const SGroup& sgroup, const SGroupCanvas& canvas);
        static SGroupBrackets _pairAroundAtoms(const SGroup& sgroup, const SGroupCanvas& canvas);
        static SGroupBrackets _storedOrPairAroundAtoms(const SGroup& sgroup, const SGroupCanvas& canvas);
    };

    class RepeatingUnitRenderer : public SGroupRenderer
    {
    public:
        std::optional<Rect2f> frameForChildren(const SGroup& sgroup, const std::optional<Rect2f>& frame, const SGroupCanvas& canvas) const override;

    protected:
        std::string _label(const SGroup& sgroup) const override;
        std::string _repeatPattern(const SGroup& sgroup) const override;
    };

    class CopolymerRenderer : public SGroupRenderer
    {
    protected:
        std::string _label(const SGroup& sgroup) const override;
        std::string _repeatPattern(const SGroup& sgroup) const override;
    };

    class ComponentRenderer : public SGroupRenderer
    {
    protected:
        std::string _label(const SGroup& sgroup) const override;
    };

    class MixtureRenderer : public SGroupRenderer
    {
    protected:
        std::string _label(const SGroup& sgroup) const override;
    };

    class FormulationRenderer : public SGroupRenderer
    {
    protected:
        std::string _label(const SGroup& sgroup) const override;
    };

    class MultipleGroupRenderer : public SGroupRenderer
    {
    public:
        std::optional<Rect2f> frameForChildren(const SGroup& sgroup, const std::optional<Rect2f>& frame, const SGroupCanvas& canvas) const override;

    protected:
        SGroupBrackets _brackets(const SGroup& sgroup, const SGroupCanvas& canvas) const override;
        std::string _label(const SGroup& sgroup) const override;
    };

    class GenericGroupRenderer : public SGroupRenderer
    {
    public:
        std::optional<Rect2f> frameForChildren(const SGroup& sgroup, const std::optional<Rect2f>& frame, const SGroupCanvas& canvas) const override;

    protected:
        SGroupBrackets _brackets(const SGroup& sgroup, const SGroupCanvas& canvas) const override;
        float _hookLength(const BracketLine& bracket) const override;
    };

    class SuperatomRenderer : public SGroupRenderer
    {
    public:
        void draw(const SGroup& sgroup, const std::optional<Rect2f>& frame, SGroupCanvas& canvas) const override;
        std::optional<Rect2f> frameForChildren(const SGroup& sgroup, const std::optional<Rect2f>& frame, const SGroupCanvas& canvas) const override;

    protected:
        SGroupBrackets _brackets(const SGroup& sgroup, const SGroupCanvas& canvas) const override;
        float _hookLength(const BracketLine& bracket) const override;
        float _labelOffset(const SGroupCanvas& canvas) const override;
        // Kept as it was: the label ends at the lower end instead of starting there
        void _placeLabel(TextItem& label, const Vec2f& lower_end, float offset) const override;
        std::string _label(const SGroup& sgroup) const override;
    };

    class DataSGroupRenderer : public SGroupRenderer
    {
    public:
        void draw(const SGroup& sgroup, const std::optional<Rect2f>& frame, SGroupCanvas& canvas) const override;
        std::optional<Rect2f> frameForChildren(const SGroup& sgroup, const std::optional<Rect2f>& frame, const SGroupCanvas& canvas) const override;
    };
} // namespace indigo

#endif //__render_sgroups_h__
