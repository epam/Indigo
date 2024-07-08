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
#include <variant>

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
    const auto KETFontBoldStrV1 = "BOLD";
    const auto KETFontItalicStrV1 = "ITALIC";
    const auto KETFontSuperscriptStrV1 = "SUPERSCRIPT";
    const auto KETFontSubscriptStrV1 = "SUBSCRIPT";
    const auto KETFontCustomSizeStrV1 = "CUSTOM_FONT_SIZE";

    const auto KETFontBoldStr = "bold";
    const auto KETFontItalicStr = "italic";
    const auto KETFontSuperscriptStr = "superscript";
    const auto KETFontSubscriptStr = "subscript";
    const auto KETFontSizeStr = "size";
    const auto KETFontColorStr = "color";
    const auto KETFontFamilyStr = "family";

    const auto KETAlignmentLeft = "left";
    const auto KETAlignmentRight = "right";
    const auto KETAlignmentCenter = "center";
    const auto KETAlignmentJustify = "justify";

    const uint8_t KETReactantArea = 0;
    const uint8_t KETReagentUpArea = 1;
    const uint8_t KETReagentDownArea = 2;
    const uint8_t KETProductArea = 3;
    const Vec2f MIN_MOL_SIZE = {0.5, 0.5};

    struct KETVersion
    {
        int major;
        int minor;
        int patch;
    };

    const KETVersion KETVersion1 = {1, 0, 0};
    const KETVersion KETVersion2 = {2, 0, 0};

    enum class KETVersionIndex : int
    {
        EMajor,
        EMinor,
        EPatch
    };

    struct KETFontStyle
    {
        using KETFontVal = std::variant<std::monostate, std::string, uint32_t>;
        enum class FontStyle : int
        {
            ENone,
            EBold,
            EItalic,
            ESuperScript,
            ESubScript,
            EFamily,
            ESize,
            EColor
        };

        KETFontStyle(const FontStyle fs, KETFontVal val = std::monostate{}) : _font_style(fs), _val(val)
        {
        }

        KETFontStyle() : _font_style(FontStyle::ENone)
        {
        }

        KETFontStyle(const KETFontStyle& other) : _font_style(other._font_style), _val(other._val)
        {
        }

        operator int() const
        {
            return static_cast<int>(_font_style);
        }

        operator FontStyle() const
        {
            return _font_style;
        }

        FontStyle getFontStyle() const
        {
            return _font_style;
        }

        std::optional<std::string> getString() const
        {
            if (auto str = std::get_if<std::string>(&_val))
                return *str;
            return std::nullopt;
        }

        std::optional<std::uint32_t> getUInt() const
        {
            if (auto val = std::get_if<uint32_t>(&_val))
                return *val;
            return std::nullopt;
        }

        void setFontStyle(FontStyle fs)
        {
            _font_style = fs;
        }

        void setValue(const std::string& val)
        {
            _val = val;
        }

        void setValue(uint32_t val)
        {
            _val = val;
        }

        bool hasValue() const
        {
            return _val.index() != 0;
        }

    private:
        FontStyle _font_style;
        std::variant<std::monostate, std::string, uint32_t> _val;
    };

    struct compareFunction
    {
        bool operator()(const std::pair<KETFontStyle, bool>& a, const std::pair<KETFontStyle, bool>& b) const
        {
            return a.second == b.second ? static_cast<int>(a.first) < static_cast<int>(b.first) : a.second < b.second;
        }
    };

    using FONT_STYLE_SET = std::set<std::pair<KETFontStyle, bool>, compareFunction>;

    FONT_STYLE_SET& operator+=(FONT_STYLE_SET& lhs, const FONT_STYLE_SET& rhs);

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
        enum class TextAlignment : int
        {
            ELeft,
            ERight,
            ECenter,
            EJustify
        };

        using TextAlignMap = const std::unordered_map<std::string, TextAlignment>;
        using StrIntMap = const std::unordered_map<std::string, int>;
        using FontStyleMap = const std::unordered_map<std::string, KETFontStyle::FontStyle>;
        using DispatchMapKVP = std::unordered_map<std::string, std::function<void(const std::string&, const rapidjson::Value&)>>;
        using DispatchMapVal = std::unordered_map<std::string, std::function<void(const rapidjson::Value&)>>;

        static const TextAlignMap& textAlignmentMap();

        static const FontStyleMap& textStyleMapV1();

        static const FontStyleMap& textStyleMap();

        static KETFontStyle::FontStyle textStyleByName(const std::string& style_name);

        struct KETTextIndent
        {
            std::optional<float> left;
            std::optional<float> right;
            std::optional<float> first_line;
        };

        struct KETTextParagraph
        {
            KETTextParagraph()
            {
            }
            std::string text;
            FONT_STYLE_SET font_style;
            std::optional<TextAlignment> alignment;
            std::optional<KETTextIndent> indent;
            std::map<std::size_t, FONT_STYLE_SET> font_styles;
        };

        static const std::uint32_t CID = "KET text object"_hash;

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

        static auto floatLambda(std::optional<float>& val)
        {
            return [&val](const rapidjson::Value& float_val) { val = float_val.GetFloat(); };
        }

        static auto intLambda(std::optional<int>& val)
        {
            return [&val](const rapidjson::Value& int_val) { val = int_val.GetInt(); };
        }

        static auto strLambda(std::optional<std::string>& str)
        {
            return [&str](const rapidjson::Value& str_val) { str = str_val.GetString(); };
        }

        auto alignLambda(std::optional<TextAlignment>& alignment)
        {
            return [this, &alignment](const std::string&, const rapidjson::Value& align_val) {
                auto ta_it = textAlignmentMap().find(align_val.GetString());
                if (ta_it != textAlignmentMap().end())
                    alignment = ta_it->second;
            };
        }

        auto styleLambda(FONT_STYLE_SET& fs)
        {
            return [this, &fs](const std::string& key, const rapidjson::Value& style_val) {
                std::string style_name = key;
                std::transform(style_name.begin(), style_name.end(), style_name.begin(), [](unsigned char c) { return std::toupper(c); });
                auto style = textStyleByName(style_name);
                if (style != KETFontStyle::FontStyle::ENone)
                    fs.emplace(style, style_val.GetBool());
            };
        }

        static auto colorLambda(FONT_STYLE_SET& fs, bool bval)
        {
            return [&fs, bval](const rapidjson::Value& color_str) {
                std::string color_string = color_str.GetString();
                if (color_string.length() == 7 && color_string[0] == '#')
                {
                    fs.emplace(
                        std::piecewise_construct,
                        std::forward_as_tuple(KETFontStyle::FontStyle::EColor, KETFontStyle::KETFontVal(std::stoul(color_string.substr(1), nullptr, 16))),
                        std::forward_as_tuple(bval));
                }
            };
        }

        static auto fontFamilyLambda(FONT_STYLE_SET& fs, bool bval)
        {
            return [&fs, bval](const rapidjson::Value& val) {
                if (val.IsString())
                    fs.emplace(std::piecewise_construct, std::forward_as_tuple(KETFontStyle::FontStyle::EFamily, val.GetString()), std::forward_as_tuple(bval));
            };
        }

        static auto fontSizeLambda(FONT_STYLE_SET& fs, bool bval)
        {
            return [&fs, bval](const rapidjson::Value& val) {
                if (val.IsInt())
                    fs.emplace(std::piecewise_construct, std::forward_as_tuple(KETFontStyle::FontStyle::ESize, KETFontStyle::KETFontVal(val.GetUint())),
                               std::forward_as_tuple(bval));
            };
        }

        auto fontLambda(FONT_STYLE_SET& fs, bool bval = true)
        {
            return [&fs, bval](const std::string&, const rapidjson::Value& font_val) {
                DispatchMapVal font_dispatcher = {
                    {KETFontFamilyStr, fontFamilyLambda(fs, bval)}, {KETFontSizeStr, fontSizeLambda(fs, bval)}, {KETFontColorStr, colorLambda(fs, bval)}};
                applyDispatcher(font_val, font_dispatcher);
            };
        }

        static auto indentLambda(std::optional<KETTextIndent>& indent)
        {
            return [&indent](const std::string&, const rapidjson::Value& indent_val) {
                std::unordered_map<std::string, std::function<void(const rapidjson::Value&)>> indent_dispatcher = {
                    {"first_line", floatLambda(indent.value().first_line)},
                    {"left", floatLambda(indent.value().left)},
                    {"right", floatLambda(indent.value().right)}};

                for (auto indent_it = indent_val.MemberBegin(); indent_it != indent_val.MemberEnd(); ++indent_it)
                {
                    auto ind_it = indent_dispatcher.find(indent_it->name.GetString());
                    if (ind_it != indent_dispatcher.end())
                        ind_it->second(indent_it->value);
                }
            };
        }

        auto partsLambda(KETTextParagraph& paragraph)
        {
            return [this, &paragraph](const std::string&, const rapidjson::Value& parts) {
                for (const auto& part : parts.GetArray())
                {
                    std::string text;
                    FONT_STYLE_SET fss;
                    auto text_lambda = [&text](const std::string&, const rapidjson::Value& text_val) { text = text_val.GetString(); };
                    auto style_lambda = styleLambda(fss);
                    DispatchMapKVP paragraph_dispatcher = {{"text", text_lambda},
                                                           {KETFontBoldStr, style_lambda},
                                                           {KETFontItalicStr, style_lambda},
                                                           {KETFontSubscriptStr, style_lambda},
                                                           {KETFontSuperscriptStr, style_lambda},
                                                           {"font", fontLambda(fss)}};
                    applyDispatcher(part, paragraph_dispatcher);
                    if (text.size())
                    {
                        paragraph.font_styles.emplace(paragraph.text.size(), fss);
                        paragraph.text += text;
                    }
                }
            };
        }

        KETTextObject(const KETTextObject& other)
            : MetaObject(CID), _alignment(other._alignment), _indent(other._indent), _font_styles{other._font_styles}, _bbox(other._bbox),
              _content(other._content), _block(other._block)
        {
        }

        KETTextObject(const Rect2f& bbox, const std::string& content);

        KETTextObject(const rapidjson::Value& text_obj);

        MetaObject* clone() const override
        {
            return new KETTextObject(*this);
        }

        auto indent() const
        {
            return _indent;
        }

        auto alignment() const
        {
            return _alignment;
        }

        const auto& boundingBox() const
        {
            return _bbox;
        }

        const auto& block() const
        {
            return _block;
        }

        const auto& content() const
        {
            return _content;
        }

        const auto& fontStyles() const
        {
            return _font_styles;
        }

    private:
        std::string _content;
        std::list<KETTextParagraph> _block;
        Rect2f _bbox;
        std::optional<KETTextIndent> _indent;
        std::optional<TextAlignment> _alignment;
        FONT_STYLE_SET _font_styles;
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
            std::cout << "hash:" << c_val << std::endl;
            return c_val;
        }
    };

}
#endif
