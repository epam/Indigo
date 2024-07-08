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

#ifdef _MSC_VER
#pragma warning(push, 4)
#endif

#include "molecule/ket_commons.h"
#include "molecule/monomer_commons.h"

namespace indigo
{
    FONT_STYLE_SET& operator+=(FONT_STYLE_SET& lhs, const FONT_STYLE_SET& rhs)
    {
        for (const auto& fs : rhs)
        {
            if (fs.second)
            {
                if (!lhs.count(fs))
                    lhs.insert(fs);
            }
            else
            {
                auto it = lhs.find(std::make_pair(fs.first, true));
                if (it != lhs.end())
                    lhs.erase(it);
            }
        }
        return lhs;
    }

    uint8_t getPointSide(const Vec2f& point, const Vec2f& beg, const Vec2f& end)
    {
        uint8_t bit_mask = 0;
        Vec2f arrow_vec(beg);
        arrow_vec.sub(end);

        Vec2f slope1(point.x, point.y);
        Vec2f slope2(slope1);
        slope1.sub(beg);
        slope2.sub(end);
        auto dt1 = Vec2f::dot(slope1, arrow_vec);
        auto dt2 = Vec2f::dot(slope2, arrow_vec);

        if (std::signbit(dt1))
            bit_mask |= KETReagentUpArea;

        if (std::signbit(dt2))
            bit_mask |= KETReagentDownArea;

        return bit_mask;
    }

    bool isCIPSGroup(SGroup& sgroup)
    {
        if (sgroup.sgroup_type == SGroup::SG_DATA)
        {
            auto& dsg = (DataSGroup&)sgroup;
            return std::string(dsg.name.ptr()) == "INDIGO_CIP_DESC";
        }
        return false;
    }

    CIPDesc stringToCIP(const std::string& cip_str)
    {
        static const std::unordered_map<std::string, CIPDesc> KStringToCIP = {{"R", CIPDesc::R}, {"S", CIPDesc::S}, {"r", CIPDesc::r},
                                                                              {"s", CIPDesc::s}, {"E", CIPDesc::E}, {"Z", CIPDesc::Z}};
        auto cip_it = KStringToCIP.find(cip_str);
        if (cip_it != KStringToCIP.end())
            return cip_it->second;
        return CIPDesc::NONE;
    }

    std::string CIPToString(CIPDesc cip)
    {
        static const std::unordered_map<int, std::string> KCIPToString = {{(int)CIPDesc::R, "R"}, {(int)CIPDesc::S, "S"}, {(int)CIPDesc::r, "r"},
                                                                          {(int)CIPDesc::s, "s"}, {(int)CIPDesc::E, "E"}, {(int)CIPDesc::Z, "Z"}};
        auto cip_it = KCIPToString.find((int)cip);
        std::string res;
        if (cip_it != KCIPToString.end())
            res = cip_it->second;
        return res;
    }

    void getSGroupAtoms(BaseMolecule& mol, std::list<std::unordered_set<int>>& neighbors)
    {
        for (int i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
        {
            SGroup& sgroup = mol.sgroups.getSGroup(i);
            neighbors.push_back({});
            auto& sg_set = neighbors.back();
            for (auto atom_idx : sgroup.atoms)
                sg_set.insert(atom_idx);
        }
        if (mol.isQueryMolecule())
        {
            QueryMolecule& qmol = static_cast<QueryMolecule&>(mol);
            qmol.getComponentNeighbors(neighbors);
        }
    }

    std::string convertAPToHELM(const std::string& atp_id_str)
    {
        if (::isupper(atp_id_str[0]) && atp_id_str.size() == 2)
        {
            if (atp_id_str == kLeftAttachmentPoint)
                return kAttachmentPointR1;
            else if (atp_id_str == kRightAttachmentPoint)
                return kAttachmentPointR2;
            else if (atp_id_str[1] == 'x')
                return std::string("R") + std::to_string(atp_id_str[0] - 'A' + 1);
        }
        return atp_id_str;
    }

    std::string convertAPFromHELM(const std::string& atp_id_str)
    {
        int id = extract_id(atp_id_str, "R");
        if (id < 0)
            throw std::invalid_argument(std::string("convertAPFromHELM: prefix 'R' not found in :") + atp_id_str);
        char ap_symbol = static_cast<char>(id) + '@'; // convert number to ASCII letter
        std::string res(1, ap_symbol);
        switch (ap_symbol)
        {
        case 'A':
            res += 'l';
            break;
        case 'B':
            res += 'r';
            break;
        default:
            res += 'x';
            break;
        }
        return res;
    }

    const KETTextObject::TextAlignMap& KETTextObject::textAlignmentMap()
    {
        static TextAlignMap KTextAlignmentsMap{{KETAlignmentLeft, TextAlignment::ELeft},
                                               {KETAlignmentRight, TextAlignment::ERight},
                                               {KETAlignmentCenter, TextAlignment::ECenter},
                                               {KETAlignmentJustify, TextAlignment::EJustify}};
        return KTextAlignmentsMap;
    }

    const KETTextObject::FontStyleMap& KETTextObject::textStyleMapV1()
    {
        static const FontStyleMap KTextStylesMap{{KETFontBoldStrV1, KETFontStyle::FontStyle::EBold},
                                                 {KETFontItalicStrV1, KETFontStyle::FontStyle::EItalic},
                                                 {KETFontSuperscriptStrV1, KETFontStyle::FontStyle::ESuperScript},
                                                 {KETFontSubscriptStrV1, KETFontStyle::FontStyle::ESubScript}};
        return KTextStylesMap;
    }

    const KETTextObject::FontStyleMap& KETTextObject::textStyleMap()
    {
        static const FontStyleMap KTextFontStylesMap{{KETFontBoldStr, KETFontStyle::FontStyle::EBold},
                                                     {KETFontItalicStr, KETFontStyle::FontStyle::EItalic},
                                                     {KETFontSuperscriptStr, KETFontStyle::FontStyle::ESuperScript},
                                                     {KETFontSubscriptStr, KETFontStyle::FontStyle::ESubScript},
                                                     {KETFontFamilyStr, KETFontStyle::FontStyle::EFamily},
                                                     {KETFontSizeStr, KETFontStyle::FontStyle::ESize},
                                                     {KETFontColorStr, KETFontStyle::FontStyle::EColor}};
        return KTextFontStylesMap;
    }

    KETFontStyle::FontStyle KETTextObject::textStyleByName(const std::string& style_name)
    {
        auto style_it = textStyleMap().find(style_name);
        if (style_it != textStyleMap().end())
            return style_it->second;
        style_it = textStyleMapV1().find(style_name);
        if (style_it != textStyleMapV1().end())
            return style_it->second;
        return KETFontStyle::FontStyle::ENone;
    }

    KETTextObject::KETTextObject(const Rect2f& bbox, const std::string& content) : MetaObject(CID), _alignment{}, _indent{}, _font_styles{}
    {
        using namespace rapidjson;
        _bbox = bbox;
        _content = content;
        Document data;
        data.Parse(content.c_str());
        if (data.HasMember("blocks"))
        {
            Value& blocks = data["blocks"];
            for (rapidjson::SizeType i = 0; i < blocks.Size(); ++i)
            {
                KETTextParagraph text_line;
                if (blocks[i].HasMember("text"))
                {
                    text_line.text = blocks[i]["text"].GetString();
                    text_line.font_styles.emplace(0, std::initializer_list<std::pair<KETFontStyle, bool>>{{}});
                    text_line.font_styles.emplace(text_line.text.size(), std::initializer_list<std::pair<KETFontStyle, bool>>{{}});
                    if (blocks[i].HasMember("inlineStyleRanges"))
                    {
                        Value& style_ranges = blocks[i]["inlineStyleRanges"];
                        for (rapidjson::SizeType j = 0; j < style_ranges.Size(); ++j)
                        {
                            int style_begin = style_ranges[j]["offset"].GetInt();
                            int style_end = style_begin + style_ranges[j]["length"].GetInt();

                            std::string style_name = style_ranges[j]["style"].GetString();
                            KETFontStyle ket_fs;

                            auto style = textStyleByName(style_name);
                            if (style != KETFontStyle::FontStyle::ENone)
                                ket_fs.setFontStyle(style);
                            else
                            {
                                const std::string KCustomFontSize = "CUSTOM_FONT_SIZE_";
                                const std::string KCustomFontUnits = "px";
                                if (style_name.find(KCustomFontSize) == 0)
                                {
                                    ket_fs.setFontStyle(KETFontStyle::FontStyle::ESize);
                                    ket_fs.setValue(std::stoi(
                                        style_name.substr(KCustomFontSize.size(), style_name.size() - KCustomFontSize.size() - KCustomFontUnits.size())));
                                }
                            }

                            const auto it_begin = text_line.font_styles.find(style_begin);
                            const auto it_end = text_line.font_styles.find(style_end);

                            if (it_begin == text_line.font_styles.end())
                                text_line.font_styles.emplace(style_begin, std::initializer_list<std::pair<KETFontStyle, bool>>{{ket_fs, true}});
                            else
                                it_begin->second.emplace(ket_fs, true);

                            if (it_end == text_line.font_styles.end())
                                text_line.font_styles.emplace(style_end, std::initializer_list<std::pair<KETFontStyle, bool>>{{ket_fs, false}});
                            else
                                it_end->second.emplace(ket_fs, false);
                        }
                    }
                }
                _block.push_back(text_line);
            }
        }
    }

    KETTextObject::KETTextObject(const rapidjson::Value& text_obj) : MetaObject(CID), _alignment{}, _indent{}, _font_styles{}
    {
        using namespace rapidjson;

        auto bbox_lambda = [this](const std::string&, const Value& bbox_val) {
            Vec2f v1(bbox_val["x"].GetFloat(), bbox_val["y"].GetFloat());
            Vec2f v2(v1);
            v2.add(Vec2f(bbox_val["width"].GetFloat(), bbox_val["height"].GetFloat()));
            _bbox.copy(Rect2f(v1, v2));
        };

        auto paragraphs_lambda = [this](const std::string&, const Value& paragraphs_val) {
            for (const auto& paragraph : paragraphs_val.GetArray())
            {
                KETTextParagraph text_line;
                auto style_lambda = styleLambda(text_line.font_style);
                DispatchMapKVP paragraph_dispatcher = {{"alignment", alignLambda(text_line.alignment)},
                                                       {KETFontBoldStr, style_lambda},
                                                       {KETFontItalicStr, style_lambda},
                                                       {KETFontSubscriptStr, style_lambda},
                                                       {KETFontSuperscriptStr, style_lambda},
                                                       {"indent", indentLambda(text_line.indent)},
                                                       {"font", fontLambda(text_line.font_style)},
                                                       {"parts", partsLambda(text_line)}};

                applyDispatcher(paragraph, paragraph_dispatcher);
                _block.push_back(text_line);
            }
        };

        auto style_lambda = styleLambda(_font_styles);

        DispatchMapKVP text_obj_dispatcher = {{"bounding_box", bbox_lambda},     {"alignment", alignLambda(_alignment)}, {KETFontBoldStr, style_lambda},
                                              {KETFontItalicStr, style_lambda},  {KETFontSubscriptStr, style_lambda},    {KETFontSuperscriptStr, style_lambda},
                                              {"indent", indentLambda(_indent)}, {"font", fontLambda(_font_styles)},     {"paragraphs", paragraphs_lambda}};

        applyDispatcher(text_obj, text_obj_dispatcher);
    }

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
