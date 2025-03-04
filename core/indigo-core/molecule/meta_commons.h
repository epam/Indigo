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

#ifndef __meta_commons_h__
#define __meta_commons_h__

#include <exception>
#include <functional>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <string>
#include <unordered_map>

#include "common/math/algebra.h"
#include "graph/graph.h"
#include "molecule/molecule_cip_calculator.h"
#include "molecule/parse_utils.h"
#include "molecule/query_molecule.h"
#include "reaction/base_reaction.h"

namespace indigo
{
    const double KDefaultFontSize = 13;
    const double KFontScaleFactor = 47;
    const auto KFontBoldStr = "BOLD";
    const auto KFontItalicStr = "ITALIC";
    const auto KFontSuperscriptStr = "SUPERSCRIPT";
    const auto KFontSubscriptStr = "SUBSCRIPT";
    const auto KFontCustomSizeStr = "CUSTOM_FONT_SIZE";
    const auto KImagePNG = "image/png";
    const auto KImageSVG = "image/svg+xml";

    const uint8_t KReactantArea = 0;
    const uint8_t KReagentUpArea = 1;
    const uint8_t KReagentDownArea = 2;
    const uint8_t KProductArea = 3;
    const Vec2f MIN_MOL_SIZE = {0.5, 0.5};

    struct compareFunction
    {
        bool operator()(const std::pair<int, bool>& a, const std::pair<int, bool>& b) const
        {
            return a.second == b.second ? a.first < b.first : a.second < b.second;
        }
    };

    using FONT_STYLE_SET = std::set<std::pair<int, bool>, compareFunction>;

    constexpr std::uint32_t string_hash(char const* s, std::size_t count)
    {
        return ((count ? string_hash(s, count - 1) : 2166136261u) ^ s[count]) * 16777619u;
    }

    constexpr std::uint32_t operator"" _hash(char const* s, std::size_t count)
    {
        return string_hash(s, count);
    }

    uint8_t getPointSide(const Vec2f& point, const Vec2f& beg, const Vec2f& end);

    CIPDesc stringToCIP(const std::string& cip_str);

    std::string CIPToString(CIPDesc cip);

    bool isCIPSGroup(SGroup& sgroup);

    void getSGroupAtoms(BaseMolecule& mol, std::list<std::unordered_set<int>>& neighbors);

    std::string convertAPToHELM(const std::string& atp_id_str);

    std::string convertAPFromHELM(const std::string& atp_id_str);

    class SimpleGraphicsObject : public MetaObject
    {
    public:
        static const std::uint32_t CID = "Metadata simple object"_hash;
        SimpleGraphicsObject(int mode, const std::pair<Vec2f, Vec2f>& coords) : MetaObject(CID)
        {
            _mode = mode;
            _coordinates = coords;
        };

        void getBoundingBox(Rect2f& bbox) const override
        {
            bbox = Rect2f(_coordinates.first, _coordinates.second);
        }

        MetaObject* clone() const override
        {
            return new SimpleGraphicsObject(_mode, _coordinates);
        }

        void offset(const Vec2f& offset) override
        {
            _coordinates.first += offset;
            _coordinates.second += offset;
        }

        enum
        {
            EEllipse,
            ERectangle,
            ELine
        };
        int _mode;
        std::pair<Vec2f, Vec2f> _coordinates;
    };

    struct SimpleTextStyle
    {
        std::size_t offset;
        std::size_t size;
        std::list<std::string> styles;
    };

    struct SimpleTextLine
    {
        std::string text;
        std::list<SimpleTextStyle> text_styles;
    };

    class SimpleTextObject : public MetaObject
    {
    public:
        enum
        {
            EPlain = 0,
            EBold = 1,
            EItalic = 2,
            ESuperScript = 3,
            ESubScript = 4,
            EFontSize = 5
        };

        const std::unordered_map<std::string, int> KTextStylesMap{
            {KFontBoldStr, EBold}, {KFontItalicStr, EItalic}, {KFontSuperscriptStr, ESuperScript}, {KFontSubscriptStr, ESubScript}};

        struct SimpleTextLine
        {
            std::string text;
            std::map<std::size_t, FONT_STYLE_SET> styles;
        };

        static const std::uint32_t CID = "Simple text object"_hash;

        SimpleTextObject(const Vec3f& pos, const Vec2f& sz, const std::string& content);

        MetaObject* clone() const override
        {
            return new SimpleTextObject(_pos, _size, _content);
        }

        void getBoundingBox(Rect2f& bbox) const override
        {
            bbox = Rect2f(Vec2f(_pos.x, _pos.y), Vec2f(_pos.x + _size.x, _pos.y - _size.y));
        }

        void offset(const Vec2f& offset) override
        {
            _pos.x += offset.x;
            _pos.y += offset.y;
        }

        const auto& getLines() const
        {
            return _block;
        }

        std::string _content;
        std::list<SimpleTextLine> _block;
        Vec3f _pos;
        Vec2f _size;
    };

    class SimpleTextObjectBuilder
    {
    public:
        SimpleTextObjectBuilder();
        void addLine(const SimpleTextLine& line);
        void finalize();
        std::string getJsonString() const;
        int getLineCounter() const;

    private:
        rapidjson::Writer<rapidjson::StringBuffer> _writer;
        rapidjson::StringBuffer _buffer;
        int _line_counter;
    };

    class ReactionArrowObject : public MetaObject
    {
    public:
        static const std::uint32_t CID = "Reaction arrow object"_hash;
        enum
        {
            EOpenAngle = 2,
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
            EUnbalancedEquilibriumOpenHalfAngle,
            EUnbalancedEquilibriumFilledHalfTriangle,
            EEllipticalArcFilledBow,
            EEllipticalArcFilledTriangle,
            EEllipticalArcOpenAngle,
            EEllipticalArcOpenHalfAngle,
            ERetrosynthetic
        };

        ReactionArrowObject(int arrow_type, const Vec2f& begin, const Vec2f& end, float height = 0)
            : MetaObject(CID), _arrow_type(arrow_type), _begin(begin), _end(end), _height(height){};

        MetaObject* clone() const override
        {
            return new ReactionArrowObject(_arrow_type, _begin, _end, _height);
        }

        void getBoundingBox(Rect2f& bbox) const override
        {
            bbox = Rect2f(_begin, _end);
        }

        int getArrowType() const
        {
            return _arrow_type;
        }

        float getHeight() const
        {
            return _height;
        }

        const auto& getHead() const
        {
            return _end;
        }

        const auto& getTail() const
        {
            return _begin;
        }

        void offset(const Vec2f& offset) override
        {
            _begin += offset;
            _end += offset;
        }

    private:
        int _arrow_type;
        float _height;
        Vec2f _begin;
        Vec2f _end;
    };

    class ReactionMultitailArrowObject : public MetaObject
    {
        static const int CORRECT_CONSTRUCTOR_PARAMETERS_SIZE = 5;
        static const int CORRECT_HEAD_SIZE = 1;
        static const int CORRECT_TAIL_SIZE = 2;

    public:
        static const std::uint32_t CID = "Reaction multitail arrow object"_hash;

        static constexpr float TAIL_ARC_RADIUS = .15f;

        template <typename Iterator>
        ReactionMultitailArrowObject(Iterator&& begin, Iterator&& end) : MetaObject(CID)
        {
            auto distanceBetweenBeginAndEnd = std::distance(begin, end);
            if (distanceBetweenBeginAndEnd < CORRECT_CONSTRUCTOR_PARAMETERS_SIZE)
                throw Exception("ReactionMultitailArrowObject: invalid arguments");

            _head = *begin++;
            _tails.reserve(static_cast<int>(distanceBetweenBeginAndEnd) - CORRECT_HEAD_SIZE - CORRECT_TAIL_SIZE);
            while (begin != end)
                _tails.push(*begin++);
            _spine_begin = _tails.pop();
            _spine_end = _tails.pop();
        }

        ReactionMultitailArrowObject(Vec2f head, const Array<Vec2f>& tails, Vec2f spine_begin, Vec2f spine_end)
            : MetaObject(CID), _head(head), _spine_begin(spine_begin), _spine_end(spine_end)
        {
            if (tails.size() < CORRECT_TAIL_SIZE)
                throw Exception("ReactionMultitailArrowObject: invalid arguments");
            _tails.copy(tails);
        }

        MetaObject* clone() const override
        {
            return new ReactionMultitailArrowObject(_head, _tails, _spine_begin, _spine_end);
        }

        auto getHead() const
        {
            return _head;
        }

        auto& getTails() const
        {
            return _tails;
        }

        auto getSpineBegin() const
        {
            return _spine_begin;
        }

        auto getSpineEnd() const
        {
            return _spine_end;
        }

        void getBoundingBox(Rect2f& bbox) const override
        {
            float min_left = 0;
            for (int i = 0; i < _tails.size(); ++i)
            {
                if (i == 0)
                    min_left = _tails[i].x;
                else
                    min_left = std::min(min_left, _tails[i].x);
            }

            float max_top = _spine_begin.y;
            float min_bottom = _spine_end.y;
            float max_right = _head.x;
            bbox = Rect2f(Vec2f(min_left, max_top), Vec2f(max_right, min_bottom));
        }

        void offset(const Vec2f& offset) override
        {
            _head += offset;
            _spine_begin += offset;
            _spine_end += offset;
            for (auto& tail : _tails)
                tail += offset;
        }

    private:
        Vec2f _head;
        Array<Vec2f> _tails;
        Vec2f _spine_begin, _spine_end;
    };

    class ReactionPlusObject : public MetaObject
    {
    public:
        static const std::uint32_t CID = "Reaction plus object"_hash;
        ReactionPlusObject(const Vec2f& pos) : MetaObject(CID), _pos(pos){};

        MetaObject* clone() const override
        {
            return new ReactionPlusObject(_pos);
        }

        enum
        {
            EKETEllipse,
            EKETRectangle,
            EKETLine
        };
        const auto& getPos() const
        {
            return _pos;
        }

        void getBoundingBox(Rect2f& bbox) const override
        {
            bbox = Rect2f(_pos, _pos);
        }

        void offset(const Vec2f& offset) override
        {
            _pos += offset;
        }

    private:
        Vec2f _pos;
    };

    class EmbeddedImageObject : public MetaObject
    {
    public:
        enum ImageFormat
        {
            EKETPNG,
            EKETSVG
        };

        static const std::uint32_t CID = "Embedded image object"_hash;
        EmbeddedImageObject(const Rect2f& bbox, EmbeddedImageObject::ImageFormat format, const std::string& data, bool is_base64 = true);

        MetaObject* clone() const override
        {
            return new EmbeddedImageObject(_bbox, _image_format, getBase64());
        }

        auto& getBoundingBox() const
        {
            return _bbox;
        }

        std::string getBase64() const;

        const std::string& getData() const
        {
            return _image_data;
        }

        ImageFormat getFormat() const
        {
            return _image_format;
        }

        void getBoundingBox(Rect2f& bbox) const override
        {
            bbox = _bbox;
        }

        void offset(const Vec2f& offset) override
        {
            _bbox = Rect2f(_bbox.leftBottom() + offset, _bbox.rightTop() + offset);
        }

    private:
        Rect2f _bbox;
        std::string _image_data;
        ImageFormat _image_format;
    };

    struct MolSumm
    {
        MolSumm() : bbox(Vec2f(0, 0), Vec2f(0, 0)), role(BaseReaction::UNDEFINED), reaction_idx(-1){};
        MolSumm(const Rect2f& box) : bbox(box), role(BaseReaction::UNDEFINED), reaction_idx(-1){};

        Rect2f bbox;
        std::vector<int> indexes;
        int role;
        int reaction_idx;
        std::vector<int> arrows_to;
        std::vector<int> arrows_from;
    };

    struct PathwayComponent
    {
        int product_csb_idx;
        std::vector<int> reactant_csb_indexes;
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
            ARROW_UNBALANCED_EQUILIBRIUM_OPEN_HALF_ANGLE,
            ARROW_UNBALANCED_EQUILIBRIUM_FILLED_HALF_TRIANGLE,
            ARROW_ELLIPTICAL_ARC_FILLED_BOW,
            ARROW_ELLIPTICAL_ARC_FILLED_TRIANGLE,
            ARROW_ELLIPTICAL_ARC_OPEN_ANGLE,
            ARROW_ELLIPTICAL_ARC_OPEN_HALF_ANGLE,
            ARROW_RETROSYNTHETIC,
            ARROW_MULTITAIL,
            TEXT
        };

        enum
        {
            NOT_CONNECTED = -2,
            CONNECTED = -1
        };

        ReactionComponent(int ctype, const Rect2f& box, int idx, std::unique_ptr<BaseMolecule> mol)
            : component_type(ctype), bbox(box), molecule(std::move(mol)), summ_block_idx(NOT_CONNECTED), index(idx){};

        int component_type;
        Rect2f bbox;
        std::unique_ptr<BaseMolecule> molecule;
        std::list<MolSumm>::iterator summ_block_it;
        int summ_block_idx;
        std::vector<Vec2f> coordinates;
        int index;
    };

    // hash for pairs taken from boost library
    struct pair_int_hash
    {
    private:
        const std::hash<int> ah;
        const std::hash<int> bh;

    public:
        pair_int_hash() : ah(), bh()
        {
        }
        size_t operator()(const std::pair<int, int>& p) const
        {
            size_t seed = ah(p.first);
            return bh(p.second) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
    };

    struct commutative_pair_int_hash
    {
        pair_int_hash pih;

    public:
        size_t operator()(const std::pair<int, int>& p) const
        {
            std::pair<int, int> sorted_pair(p);
            if (sorted_pair.first > sorted_pair.second)
                std::swap(sorted_pair.first, sorted_pair.second);
            auto c_val = pih(sorted_pair);
            return c_val;
        }
    };

    std::string getDebugSmiles(BaseMolecule& mol);
}
#endif
