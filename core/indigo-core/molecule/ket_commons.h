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
    const auto KETAlignmentLeft = "left";
    const auto KETAlignmentRight = "right";
    const auto KETAlignmentCenter = "center";
    const auto KETAlignmentJustify = "justify";

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

        enum class TextAlignment : int
        {
            ELeft,
            ERight,
            ECenter,
            EJustify
        };

        using TextAlignMap = const std::unordered_map<std::string, TextAlignment>;
        using StrIntMap = const std::unordered_map<std::string, int>;
        using DispatchMapKVP = std::unordered_map<std::string, std::function<void(const std::string&, const rapidjson::Value&)>>;
        using DispatchMapVal = std::unordered_map<std::string, std::function<void(const rapidjson::Value&)>>;

        TextAlignMap KTextAlignmentsMap{{KETAlignmentLeft, TextAlignment::ELeft},
                                        {KETAlignmentRight, TextAlignment::ERight},
                                        {KETAlignmentCenter, TextAlignment::ECenter},
                                        {KETAlignmentJustify, TextAlignment::EJustify}};

        StrIntMap KTextStylesMap{
            {KETFontBoldStr, EBold}, {KETFontItalicStr, EItalic}, {KETFontSuperscriptStr, ESuperScript}, {KETFontSubscriptStr, ESubScript}};

        struct KETTextIndent
        {
            float left;
            float right;
            float first_line;
        };

        struct KETTextFont
        {
            KETTextFont() : size(0), color(0)
            {
            }
            std::string family;
            int size;
            uint32_t color;
        };

        struct KETTextStyle
        {
            KETTextStyle() : bold(false), italic(false), subscript(false), superscript(false)
            {
            }
            bool bold;
            bool italic;
            bool subscript;
            bool superscript;
        };

        struct KETTextParagraph
        {
            std::string text;
            KETTextStyle style;
            KETTextFont font;
            TextAlignment alignment;
            KETTextIndent indent;
            std::map<std::size_t, FONT_STYLE_SET> styles;
        };

        static const std::uint32_t CID = "KET text object"_hash;

        std::string _content;
        std::list<KETTextParagraph> _block;
        Rect2f _bbox;
        KETTextIndent _indent;
        KETTextFont _font;
        TextAlignment _alignment;
        KETTextStyle _style;

        static void applyDispatcher(const rapidjson::Value& val, const DispatchMapKVP& disp_map)
        {
            for (auto kvp_it = val.MemberBegin(); kvp_it != val.MemberEnd(); ++kvp_it)
            {
                auto disp_it = disp_map.find(kvp_it->name.GetString());
                if (disp_it != disp_map.end())
                    disp_it->second(kvp_it->name.GetString(), kvp_it->value);
            }
        }

        static void applyDispatcher(const rapidjson::Value& val, const DispatchMapVal& disp_map)
        {
            for (auto kvp_it = val.MemberBegin(); kvp_it != val.MemberEnd(); ++kvp_it)
            {
                auto disp_it = disp_map.find(kvp_it->name.GetString());
                if (disp_it != disp_map.end())
                    disp_it->second(kvp_it->value);
            }
        }

        static auto floatLambda(float& val)
        {
            return [&val](const rapidjson::Value& float_val) { val = float_val.GetFloat(); };
        }

        static auto intLambda(int& val)
        {
            return [&val](const rapidjson::Value& int_val) { val = int_val.GetInt(); };
        }

        static auto strLambda(std::string& str)
        {
            return [&str](const rapidjson::Value& str_val) { str = str_val.GetString(); };
        }

        static auto colorLambda(uint32_t& color)
        {
            return [&color](const rapidjson::Value& color_str) {
                std::string color_string = color_str.GetString();
                if (color_string.length() == 7 && color_string[0] == '#')
                    color = std::stoul(color_string.substr(1), nullptr, 16);
            };
        }

        auto alignLambda(TextAlignment& alignment)
        {
            return [this, &alignment](const std::string&, const rapidjson::Value& align_val) {
                auto ta_it = KTextAlignmentsMap.find(align_val.GetString());
                if (ta_it != KTextAlignmentsMap.end())
                    alignment = ta_it->second;
            };
        }

        auto styleLambda(KETTextStyle& style)
        {
            return [this, &style](const std::string& key, const rapidjson::Value& style_val) {
                std::string style_name = key;
                std::transform(style_name.begin(), style_name.end(), style_name.begin(), [](unsigned char c) { return std::toupper(c); });
                auto ta_it = KTextStylesMap.find(style_name);
                if (ta_it != KTextStylesMap.end())
                {
                    bool bval = style_val.GetBool();
                    switch (ta_it->second)
                    {
                    case EBold:
                        style.bold = bval;
                        break;
                    case EItalic:
                        style.italic = bval;
                        break;
                    case ESubScript:
                        style.subscript = bval;
                        break;
                    case ESuperScript:
                        style.superscript = bval;
                        break;
                    default:
                        break;
                    }
                }
            };
        }

        auto styleLambda(FONT_STYLE_SET& style)
        {
            return [this, &style](const std::string& key, const rapidjson::Value& style_val) {
                std::string style_name = key;
                std::transform(style_name.begin(), style_name.end(), style_name.begin(), [](unsigned char c) { return std::toupper(c); });
                auto ta_it = KTextStylesMap.find(style_name);
                if (ta_it != KTextStylesMap.end())
                    style.emplace(ta_it->second, style_val.GetBool());
            };
        }

        static auto indentLambda(KETTextIndent& indent)
        {
            return [&indent](const std::string&, const rapidjson::Value& indent_val) {
                std::unordered_map<std::string, std::function<void(const rapidjson::Value&)>> indent_dispatcher = {
                    {"first_line", floatLambda(indent.first_line)}, {"left", floatLambda(indent.left)}, {"right", floatLambda(indent.right)}};

                for (auto indent_it = indent_val.MemberBegin(); indent_it != indent_val.MemberEnd(); ++indent_it)
                {
                    auto ind_it = indent_dispatcher.find(indent_it->name.GetString());
                    if (ind_it != indent_dispatcher.end())
                        ind_it->second(indent_it->value);
                }
            };
        }

        static auto fontLambda(KETTextFont& font)
        {
            return [&font](const std::string&, const rapidjson::Value& font_val) {
                DispatchMapVal font_dispatcher = {{"family", strLambda(font.family)}, {"size", intLambda(font.size)}, {"color", colorLambda(font.color)}};
                applyDispatcher(font_val, font_dispatcher);
            };
        }

        auto partsLambda(KETTextParagraph& paragraph)
        {
            return [this, &paragraph](const std::string&, const rapidjson::Value& parts) {
                auto text_lambda = [&paragraph](const std::string&, const rapidjson::Value& text_val) { paragraph.text += text_val.GetString(); };
                auto style_lambda = [this, &paragraph](const std::string& key, const rapidjson::Value& style_val) 
                {
                    std::string style_name = key;
                    std::transform(style_name.begin(), style_name.end(), style_name.begin(), [](unsigned char c) { return std::toupper(c); });
                    auto ta_it = KTextStylesMap.find(style_name);
                    if (ta_it != KTextStylesMap.end())
                    {
                        bool bval = style_val.GetBool();
                        switch (ta_it->second)
                        {
                        case EBold:
                            break;
                        case EItalic:
                            break;
                        case ESubScript:
                            break;
                        case ESuperScript:
                            break;
                        default:
                            break;
                        }
                    }
                };
                for (const auto& part : parts.GetArray())
                {

                }
            };
        }

        KETTextObject(const KETTextObject& other)
            : MetaObject(CID), _alignment(other._alignment), _indent(other._indent), _font{other._font}, _bbox(other._bbox), _content(other._content),
              _block(other._block)
        {
        }

        KETTextObject(const Rect2f& bbox, const std::string& content);

        KETTextObject(const rapidjson::Value& text_obj);

        MetaObject* clone() const override
        {
            return new KETTextObject(*this);
        }
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
