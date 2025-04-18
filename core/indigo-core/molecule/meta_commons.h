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
#include <variant>

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
    const auto KFontBoldStrV1 = "BOLD";
    const auto KFontItalicStrV1 = "ITALIC";
    const auto KFontSuperscriptStrV1 = "SUPERSCRIPT";
    const auto KFontSubscriptStrV1 = "SUBSCRIPT";
    const auto KFontCustomSizeStrV1 = "CUSTOM_FONT_SIZE";
    const auto KImagePNG = "image/png";
    const auto KImageSVG = "image/svg+xml";

    const auto KFontBoldStr = "bold";
    const auto KFontItalicStr = "italic";
    const auto KFontSuperscriptStr = "superscript";
    const auto KFontSubscriptStr = "subscript";
    const auto KFontSizeStr = "size";
    const auto KFontColorStr = "color";
    const auto KFontFamilyStr = "family";

    const auto KAlignmentLeft = "left";
    const auto KAlignmentRight = "right";
    const auto KAlignmentCenter = "center";
    const auto KAlignmentFull = "full";

    const uint8_t KReactantArea = 0;
    const uint8_t KReagentUpArea = 1;
    const uint8_t KReagentDownArea = 2;
    const uint8_t KProductArea = 3;
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

    using KETFontVal = std::variant<std::monostate, std::string, uint32_t>;

    struct KETFontStyleStatus
    {
        KETFontStyleStatus() : offset(0), size(0), val(std::monostate{})
        {
        }

        KETFontStyleStatus(std::size_t off, std::size_t sz, KETFontVal v) : offset(off), size(sz), val(v)
        {
        }

        std::size_t offset;
        std::size_t size;
        KETFontVal val;
    };

    struct KETFontStyle
    {
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

        int intFontStyle() const
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

        const KETFontVal& getVal() const
        {
            return _val;
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

        void copyValue(const KETFontStyle& other)
        {
            _val = other._val;
        }

        void setValue(uint32_t val)
        {
            _val = val;
        }

        void setValue(const std::variant<std::monostate, std::string, uint32_t>& val)
        {
            _val = val;
        }

        bool hasValue() const
        {
            return _val.index() != 0;
        }

    private:
        FontStyle _font_style;
        KETFontVal _val;
    };

    struct compareFunction
    {
        bool operator()(const std::pair<KETFontStyle, bool>& a, const std::pair<KETFontStyle, bool>& b) const
        {
            return a.second == b.second ? a.first.intFontStyle() < b.first.intFontStyle() : a.second < b.second;
        }
    };

    using FONT_STYLE_SET = std::set<std::pair<KETFontStyle, bool>, compareFunction>;
    using KETFontStatusMap = std::map<KETFontStyle::FontStyle, std::deque<KETFontStyleStatus>>;

    FONT_STYLE_SET& operator&=(FONT_STYLE_SET& lhs, const FONT_STYLE_SET& rhs);
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
        enum class TextAlignment : int
        {
            ELeft,
            ERight,
            ECenter,
            EFull
        };

        using TextAlignMap = const std::unordered_map<std::string, TextAlignment>;
        using StrIntMap = const std::unordered_map<std::string, int>;
        using FontStyleMap = const std::unordered_map<std::string, KETFontStyle::FontStyle>;
        using FontStyleMapInv = const std::unordered_map<KETFontStyle::FontStyle, std::string>;

        using DispatchMapKVP = std::unordered_map<std::string, std::function<void(const std::string&, const rapidjson::Value&)>>;
        using DispatchMapVal = std::unordered_map<std::string, std::function<void(const rapidjson::Value&)>>;

        static const TextAlignMap& textAlignmentMap();

        static const FontStyleMap& textStyleMapV1();
        static const FontStyleMapInv& textStyleMapInvV1();
        static const FontStyleMap& textStyleMap();

        static KETFontStyle::FontStyle textStyleByName(const std::string& style_name);

        struct KETTextParagraph
        {
            KETTextParagraph() : font_styles{}, alignment{}, indent{}, line_starts{}
            {
            }
            std::string text;
            FONT_STYLE_SET font_style;
            std::optional<TextAlignment> alignment;
            std::optional<float> indent;
            std::optional<std::set<int>> line_starts;
            std::map<std::size_t, FONT_STYLE_SET> font_styles;
        };

        static const std::uint32_t CID = "Simple text object"_hash;

        static void convertToSimpleTextStyle(const FONT_STYLE_SET& fss, SimpleTextStyle& sts)
        {
            for (auto& fs : fss)
            {
                switch (fs.first.getFontStyle())
                {
                case KETFontStyle::FontStyle::EBold:
                    sts.styles.push_back(KFontBoldStrV1);
                    break;
                case KETFontStyle::FontStyle::EItalic:
                    sts.styles.push_back(KFontItalicStrV1);
                    break;
                case KETFontStyle::FontStyle::ESubScript:
                    sts.styles.push_back(KFontSubscriptStrV1);
                    break;
                case KETFontStyle::FontStyle::ESuperScript:
                    sts.styles.push_back(KFontSuperscriptStrV1);
                    break;
                case KETFontStyle::FontStyle::ESize: {
                    auto fs_val = fs.first.getUInt();
                    if (fs_val.has_value())
                        sts.styles.push_back(std::string(KFontCustomSizeStrV1) + "_" + std::to_string(fs_val.value()) + "px");
                    break;
                }
                default:
                    break;
                }
            }
        }

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
            return [&val](const std::string&, const rapidjson::Value& float_val) {
                if (float_val.IsFloat())
                    val = float_val.GetFloat();
            };
        }

        static auto intLambda(std::optional<int>& val)
        {
            return [&val](const rapidjson::Value& int_val) {
                if (int_val.IsInt())
                    val = int_val.GetInt();
            };
        }

        static auto boolLambda(bool& val)
        {
            return [&val](const rapidjson::Value& bool_val) {
                if (bool_val.IsBool())
                    val = bool_val.GetBool();
            };
        }

        static auto strLambda(std::optional<std::string>& str)
        {
            return [&str](const rapidjson::Value& str_val) {
                if (str_val.IsString())
                    str = str_val.GetString();
            };
        }

        auto alignLambda(std::optional<TextAlignment>& alignment)
        {
            return [this, &alignment](const std::string&, const rapidjson::Value& align_val) {
                auto ta_it = textAlignmentMap().find(align_val.GetString());
                if (ta_it != textAlignmentMap().end())
                    alignment = ta_it->second;
            };
        }

        auto styleLambda(FONT_STYLE_SET& fss)
        {
            return [this, &fss](const std::string& key, const rapidjson::Value& style_val) {
                std::string style_name = key;
                std::transform(style_name.begin(), style_name.end(), style_name.begin(), [](unsigned char c) { return std::toupper(c); });
                auto style = textStyleByName(style_name);
                if (style != KETFontStyle::FontStyle::ENone)
                    fss.emplace(style, style_val.GetBool());
            };
        }

        static auto colorLambda(FONT_STYLE_SET& fss, bool bval)
        {
            return [&fss, bval](const rapidjson::Value& color_str) {
                std::string color_string = color_str.GetString();
                if (color_string.size() && color_string[0] == '#')
                {
                    fss.emplace(std::piecewise_construct,
                                std::forward_as_tuple(KETFontStyle::FontStyle::EColor, static_cast<uint32_t>(std::stoul(color_string.substr(1), nullptr, 16))),
                                std::forward_as_tuple(bval));
                }
            };
        }

        static auto fontFamilyLambda(FONT_STYLE_SET& fss, bool bval)
        {
            return [&fss, bval](const rapidjson::Value& val) {
                if (val.IsString())
                    fss.emplace(std::piecewise_construct, std::forward_as_tuple(KETFontStyle::FontStyle::EFamily, val.GetString()),
                                std::forward_as_tuple(bval));
            };
        }

        static auto fontSizeLambda(FONT_STYLE_SET& fss, bool bval)
        {
            return [&fss, bval](const rapidjson::Value& val) {
                if (val.IsInt())
                    fss.emplace(std::piecewise_construct, std::forward_as_tuple(KETFontStyle::FontStyle::ESize, static_cast<uint32_t>(val.GetUint())),
                                std::forward_as_tuple(bval));
            };
        }

        auto fontLambda(FONT_STYLE_SET& fss, bool bval = true)
        {
            return [&fss, bval](const std::string&, const rapidjson::Value& font_val) {
                DispatchMapVal font_dispatcher = {
                    {KFontFamilyStr, fontFamilyLambda(fss, bval)}, {KFontSizeStr, fontSizeLambda(fss, bval)}, {KFontColorStr, colorLambda(fss, bval)}};
                applyDispatcher(font_val, font_dispatcher);
            };
        }

        auto lineStartsLambda(KETTextParagraph& paragraph)
        {
            return [&paragraph](const std::string&, const rapidjson::Value& line_starts) {
                for (const auto& line_start : line_starts.GetArray())
                {
                    if (!paragraph.line_starts.has_value())
                        paragraph.line_starts = std::set<int>();
                    paragraph.line_starts.value().insert(line_start.GetInt());
                }
            };
        }

        auto partsLambda(KETTextParagraph& paragraph)
        {
            return [this, &paragraph](const std::string&, const rapidjson::Value& parts) {
                for (const auto& part_val : parts.GetArray())
                {
                    std::string part;
                    FONT_STYLE_SET fss;
                    auto text_lambda = [&part](const std::string&, const rapidjson::Value& text_val) { part = text_val.GetString(); };
                    auto style_lambda = styleLambda(fss);
                    DispatchMapKVP paragraph_dispatcher = {{"text", text_lambda},
                                                           {KFontBoldStr, style_lambda},
                                                           {KFontItalicStr, style_lambda},
                                                           {KFontSubscriptStr, style_lambda},
                                                           {KFontSuperscriptStr, style_lambda},
                                                           {"font", fontLambda(fss)}};
                    // all styles collected in fss
                    applyDispatcher(part_val, paragraph_dispatcher);
                    if (part.size())
                    {
                        auto prev_it = paragraph.font_styles.find(paragraph.text.size());
                        if (prev_it != paragraph.font_styles.end())
                            prev_it->second += fss;
                        else
                            paragraph.font_styles.emplace(paragraph.text.size(), fss);

                        paragraph.text += part;
                        FONT_STYLE_SET fss_off;
                        std::transform(fss.begin(), fss.end(), std::inserter(fss_off, fss_off.end()),
                                       [](const auto& entry) { return std::make_pair(entry.first, false); });
                        fss = std::move(fss_off);
                        paragraph.font_styles.emplace(paragraph.text.size(), fss);
                    }
                }
            };
        }

        SimpleTextObject(const SimpleTextObject& other)
            : MetaObject(CID), _alignment(other._alignment), _indent(other._indent), _font_styles{other._font_styles}, _bbox(other._bbox),
              _content(other._content), _block(other._block)
        {
        }

        SimpleTextObject(const Rect2f& bbox, const std::string& content);

        SimpleTextObject(const rapidjson::Value& text_obj);
        SimpleTextObject();

        MetaObject* clone() const override
        {
            return new SimpleTextObject(*this);
        }

        auto& indent()
        {
            return _indent;
        }

        const auto& indent() const
        {
            return _indent;
        }

        auto& alignment()
        {
            return _alignment;
        }

        const auto& alignment() const
        {
            return _alignment;
        }

        auto& boundingBox()
        {
            return _bbox;
        }

        const auto& boundingBox() const
        {
            return _bbox;
        }

        auto& block()
        {
            return _block;
        }

        const auto& block() const
        {
            return _block;
        }

        const auto& content() const
        {
            return _content;
        }

        auto& fontStyles()
        {
            return _font_styles;
        }

        const auto& fontStyles() const
        {
            return _font_styles;
        }

        void getBoundingBox(Rect2f& bbox) const override
        {
            bbox = _bbox;
        }

        const auto& getLines() const
        {
            return _block;
        }

    private:
        void offset(const Vec2f& offset) override
        {
            _bbox.offset(offset);
        }

        std::string _content;
        std::list<KETTextParagraph> _block;
        Rect2f _bbox;
        std::optional<float> _indent;
        std::optional<TextAlignment> _alignment;
        FONT_STYLE_SET _font_styles;
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
