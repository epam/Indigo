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

#include <exception>
#include <functional>
#include <rapidjson/document.h>
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
    const double KETDefaultFontSize = 13;
    const double KETFontScaleFactor = 47;
    const auto KETFontBoldStr = "BOLD";
    const auto KETFontItalicStr = "ITALIC";
    const auto KETFontSuperscriptStr = "SUPERSCRIPT";
    const auto KETFontSubscriptStr = "SUBSCRIPT";
    const auto KETFontCustomSizeStr = "CUSTOM_FONT_SIZE";
    const uint8_t KETReactantArea = 0;
    const uint8_t KETReagentUpArea = 1;
    const uint8_t KETReagentDownArea = 2;
    const uint8_t KETProductArea = 3;
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
            {KETFontBoldStr, EBold}, {KETFontItalicStr, EItalic}, {KETFontSuperscriptStr, ESuperScript}, {KETFontSubscriptStr, ESubScript}};

        struct KETTextLine
        {
            std::string text;
            std::map<std::size_t, FONT_STYLE_SET> styles;
        };

        static const std::uint32_t CID = "KET text object"_hash;

        KETTextObject(const Vec3f& pos, const std::string& content) : MetaObject(CID)
        {
            using namespace rapidjson;
            _pos = pos;
            _content = content;
            Document data;
            data.Parse(content.c_str());
            if (data.HasMember("blocks"))
            {
                Value& blocks = data["blocks"];
                for (rapidjson::SizeType i = 0; i < blocks.Size(); ++i)
                {
                    KETTextLine text_line;
                    if (blocks[i].HasMember("text"))
                    {
                        text_line.text = blocks[i]["text"].GetString();
                        text_line.styles.emplace(0, std::initializer_list<std::pair<int, bool>>{});
                        text_line.styles.emplace(text_line.text.size(), std::initializer_list<std::pair<int, bool>>{});
                        if (blocks[i].HasMember("inlineStyleRanges"))
                        {
                            Value& style_ranges = blocks[i]["inlineStyleRanges"];
                            for (rapidjson::SizeType j = 0; j < style_ranges.Size(); ++j)
                            {
                                int style_begin = style_ranges[j]["offset"].GetInt();
                                int style_end = style_begin + style_ranges[j]["length"].GetInt();
                                int style_code = -1;

                                std::string style = style_ranges[j]["style"].GetString();
                                auto it = KTextStylesMap.find(style);
                                if (it != KTextStylesMap.end())
                                {
                                    style_code = it->second;
                                }
                                else
                                {
                                    const std::string KCustomFontSize = "CUSTOM_FONT_SIZE_";
                                    const std::string KCustomFontUnits = "px";
                                    if (style.find(KCustomFontSize) == 0)
                                    {
                                        style_code =
                                            std::stoi(style.substr(KCustomFontSize.size(), style.size() - KCustomFontSize.size() - KCustomFontUnits.size()));
                                    }
                                }
                                const auto it_begin = text_line.styles.find(style_begin);
                                const auto it_end = text_line.styles.find(style_end);

                                if (it_begin == text_line.styles.end())
                                    text_line.styles.emplace(style_begin, std::initializer_list<std::pair<int, bool>>{{style_code, true}});
                                else
                                {
                                    it_begin->second.emplace(style_code, true);
                                }

                                if (it_end == text_line.styles.end())
                                    text_line.styles.emplace(style_end, std::initializer_list<std::pair<int, bool>>{{style_code, false}});
                                else
                                {
                                    it_end->second.emplace(style_code, false);
                                }
                            }
                        }
                    }
                    _block.push_back(text_line);
                }
            }
        }

        MetaObject* clone() const override
        {
            return new KETTextObject(_pos, _content);
        }

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
        };

        KETReactionArrow(int arrow_type, const Vec2f& begin, const Vec2f& end, float height = 0)
            : MetaObject(CID), _arrow_type(arrow_type), _begin(begin), _end(end), _height(height){};

        MetaObject* clone() const override
        {
            return new KETReactionArrow(_arrow_type, _begin, _end, _height);
        }

        int _arrow_type;
        float _height;
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
            ARROW_UNBALANCED_EQUILIBRIUM_OPEN_HALF_ANGLE,
            ARROW_UNBALANCED_EQUILIBRIUM_FILLED_HALF_TRIANGLE,
            ARROW_ELLIPTICAL_ARC_FILLED_BOW,
            ARROW_ELLIPTICAL_ARC_FILLED_TRIANGLE,
            ARROW_ELLIPTICAL_ARC_OPEN_ANGLE,
            ARROW_ELLIPTICAL_ARC_OPEN_HALF_ANGLE
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
}
#endif
