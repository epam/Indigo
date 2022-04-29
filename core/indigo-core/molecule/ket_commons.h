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

#ifndef __ket_commons_h__
#define __ket_commons_h__

#include <string>

#include "common/math/algebra.h"
#include "graph/graph.h"
#include "reaction/base_reaction.h"

namespace indigo
{
    constexpr std::uint32_t string_hash(char const* s, std::size_t count)
    {
        return ((count ? string_hash(s, count - 1) : 2166136261u) ^ s[count]) * 16777619u;
    }

    constexpr std::uint32_t operator"" _hash(char const* s, std::size_t count)
    {
        return string_hash(s, count);
    }

    class KETSimpleObject : public MetaObject
    {
    public:
        static const std::uint32_t CID = "KET simple object"_hash;
        KETSimpleObject(int mode, const std::pair<Vec2f, Vec2f>& coords) : MetaObject(CID)
        {
            _mode = mode;
            _coordinates = coords;
        };

        MetaObject* clone() const override
        {
            return new KETSimpleObject(_mode, _coordinates);
        }

        enum
        {
            EKETEllipse,
            EKETRectangle,
            EKETLine
        };
        int _mode;
        std::pair<Vec2f, Vec2f> _coordinates;
    };

    class KETTextObject : public MetaObject
    {
    public:
        static const std::uint32_t CID = "KET text object"_hash;

        KETTextObject(const Vec3f& pos, const std::string& content) : MetaObject(CID)
        {
            _pos = pos;
            _content = content;
        }

        MetaObject* clone() const override
        {
            return new KETTextObject(_pos, _content);
        }

        struct KETTextStyle
        {
            int _offset;
            int _size;
            bool _italic;
            bool _bold;
            bool _subscript;
            bool _superscript;
            int _font_size;
        };

        struct KETTextLine
        {
            std::string _text;
            std::list<KETTextStyle> _inline_styles;
        };

        std::string _content;
        std::list<KETTextLine> _block;
        Vec3f _pos;
    };

    class KETReactionArrow : public MetaObject
    {
    public:
        static const std::uint32_t CID = "KET reaction arrow"_hash;
        enum
        {
            EOpenAngle,
            EFilledTriangle,
            EFilledBow,
            EDashedOpenAngle,
            EFailed,
            EBothEndsFilledTriangle,
            EEquilibriumFilledHalfBow,
            EEquilibriumFilledTriangle,
            EEquilibriumOpenAngle,
            EUnbalancedEquilibriumFilledHalfBow,
            EUnbalancedEquilibriumLargeFilledHalfBow,
            EUnbalancedEquilibriumFilleHalfTriangle
        };

        KETReactionArrow(int arrow_type, const Vec2f& begin, const Vec2f& end) : MetaObject(CID), _arrow_type(arrow_type), _begin(begin), _end(end){};

        MetaObject* clone() const override
        {
            return new KETReactionArrow(_arrow_type, _begin, _end);
        }

        int _arrow_type;
        Vec2f _begin;
        Vec2f _end;
    };

    class KETReactionPlus : public MetaObject
    {
    public:
        static const std::uint32_t CID = "KET reaction plus"_hash;
        KETReactionPlus(const Vec2f& pos) : MetaObject(CID), _pos(pos){};

        MetaObject* clone() const override
        {
            return new KETReactionPlus(_pos);
        }

        enum
        {
            EKETEllipse,
            EKETRectangle,
            EKETLine
        };
        Vec2f _pos;
    };

    struct MolSumm
    {
        MolSumm() : bbox(Vec2f(0, 0), Vec2f(0, 0)), role(BaseReaction::UNDEFINED){};
        MolSumm(const Rect2f& box) : bbox(box), role(BaseReaction::UNDEFINED){};

        Rect2f bbox;
        std::vector<int> indexes;
        int role;
        std::vector<int> arrows_to;
    };

    struct ReactionComponent
    {
        enum
        {
            MOLECULE = 0,
            PLUS,
            ARROW_BASIC,
            ARROW_FILLED_TRIANGLE,
            ARROW_FILLED_BOW,
            ARROW_DASHED,
            ARROW_FAILED,
            ARROW_BOTH_ENDS_FILLED_TRIANGLE,
            ARROW_EQUILIBRIUM_FILLED_HALF_BOW,
            ARROW_EQUILIBRIUM_FILLED_TRIANGLE,
            ARROW_EQUILIBRIUM_OPEN_ANGLE,
            ARROW_UNBALANCED_EQUILIBRIUM_FILLED_HALF_BOW,
            ARROW_UNBALANCED_EQUILIBRIUM_LARGE_FILLED_HALF_BOW,
            ARROW_UNBALANCED_EQUILIBRIUM_FILLED_HALF_TRIANGLE
        };

        enum
        {
            NOT_CONNECTED = -2,
            CONNECTED = -1
        };

        ReactionComponent(int ctype, const Rect2f& box, std::unique_ptr<BaseMolecule> mol)
            : component_type(ctype), bbox(box), molecule(std::move(mol)), summ_block_idx(NOT_CONNECTED){};

        int component_type;
        Rect2f bbox;
        std::unique_ptr<BaseMolecule> molecule;
        std::list<MolSumm>::iterator summ_block_it;
        int summ_block_idx;
        std::vector<Vec2f> coordinates;
    };

}
#endif
