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

#include <iomanip>
#include <memory>
#include <set>
#include <sstream>

#include "layout/molecule_layout.h"

#include "molecule/molecule.h"
#include "molecule/molecule_cip_calculator.h"
#include "molecule/molecule_json_saver.h"
#include "molecule/molecule_savers.h"
#include "molecule/molecule_substructure_matcher.h"
#include "molecule/monomer_commons.h"
#include "molecule/monomers_template_library.h"
#include "molecule/parse_utils.h"

#include "molecule/query_molecule.h"
#include "molecule/smiles_loader.h"
#include "molecule/smiles_saver.h"
#include "reaction/pathway_reaction.h"
#include "reaction/reaction_multistep_detector.h"

#include <base_cpp/scanner.h>

using namespace indigo;
using namespace rapidjson;

void MoleculeJsonSaver::saveTextV1(JsonWriter& writer, const SimpleTextObject& text_obj)
{
    std::string content = text_obj.content();
    if (content.empty() && text_obj.block().size())
    {
        SimpleTextObjectBuilder tob;
        auto default_fss = text_obj.fontStyles();
        for (const auto& paragraph : text_obj.block())
        {
            auto paragraph_fss = default_fss;
            paragraph_fss += paragraph.font_style;
            SimpleTextLine line;
            line.text = paragraph.text;
            std::replace(line.text.begin(), line.text.end(), '\r', '\n');
            std::string_view text_view = std::string_view(line.text);
            KETFontStatusMap style_status_map;
            if (paragraph.font_styles.size() > 1)
                for (auto it_fss_kvp = paragraph.font_styles.rbegin(); it_fss_kvp != paragraph.font_styles.rend(); ++it_fss_kvp)
                {
                    auto current_part_fss = paragraph_fss;
                    auto prev_part_fss = paragraph_fss;

                    current_part_fss += it_fss_kvp->second;

                    auto it_fss_kvp_prev = it_fss_kvp != paragraph.font_styles.rbegin() ? std::prev(it_fss_kvp) : it_fss_kvp;

                    if (it_fss_kvp != it_fss_kvp_prev)
                        prev_part_fss += it_fss_kvp_prev->second;

                    auto text_part = it_fss_kvp != it_fss_kvp_prev ? text_view.substr(it_fss_kvp->first, it_fss_kvp_prev->first - it_fss_kvp->first)
                                                                   : text_view.substr(it_fss_kvp->first);

                    for (auto& fss : current_part_fss)
                    {
                        auto fs = fss.first.getFontStyle();
                        auto fs_it = style_status_map.find(fs);
                        if (fs_it == style_status_map.end())
                        {
                            style_status_map.emplace(
                                std::piecewise_construct, std::forward_as_tuple(fs),
                                std::forward_as_tuple(std::initializer_list<KETFontStyleStatus>{{it_fss_kvp->first, text_part.size(), fss.first.getVal()}}));
                        }
                        else
                        {
                            if (fs_it->second.front().val != fss.first.getVal() || fs_it->second.front().offset != it_fss_kvp_prev->first)
                                fs_it->second.emplace_front(it_fss_kvp->first, text_part.size(), fss.first.getVal());
                            else
                            {
                                fs_it->second.front().offset = it_fss_kvp->first;
                                fs_it->second.front().size += text_part.size();
                            }
                        }
                    }
                }
            if (style_status_map.size())
            {
                for (auto& [fs, ss_queue] : style_status_map)
                {
                    for (auto& ss : ss_queue)
                    {
                        SimpleTextStyle sts;
                        sts.offset = ss.offset;
                        sts.size = ss.size;
                        if (fs == KETFontStyle::FontStyle::ESize)
                        {
                            if (auto pval = std::get_if<uint32_t>(&ss.val))
                                sts.styles.push_back(std::string(KFontCustomSizeStrV1) + "_" + std::to_string(*pval) + "px");
                        }
                        else
                        {
                            auto it_fs = SimpleTextObject::textStyleMapInvV1().find(fs);
                            if (it_fs != SimpleTextObject::textStyleMapInvV1().end())
                                sts.styles.push_back(it_fs->second);
                        }
                        line.text_styles.push_back(sts);
                    }
                }
            }
            tob.addLine(line);
        }
        if (tob.getLineCounter())
            tob.finalize();
        content = tob.getJsonString();
    }

    writer.Key("data");
    writer.StartObject();
    writer.Key("content");
    writer.String(content.c_str());
    writer.Key("position");
    writer.WritePoint(text_obj.boundingBox().leftTop());
    writer.Key("pos");
    writer.StartArray();
    writer.WritePoint(text_obj.boundingBox().leftTop());
    writer.WritePoint(text_obj.boundingBox().leftBottom());
    writer.WritePoint(text_obj.boundingBox().rightBottom());
    writer.WritePoint(text_obj.boundingBox().rightTop());
    writer.EndArray();
    writer.EndObject();
}

void MoleculeJsonSaver::saveTextV2(JsonWriter& writer, const SimpleTextObject& text_obj)
{
    writer.Key("boundingBox");
    writer.WriteRect(text_obj.boundingBox());
    if (text_obj.alignment().has_value())
        saveAlignment(writer, text_obj.alignment().value());
    if (text_obj.indent().has_value())
    {
        writer.Key("indent");
        writer.Double(text_obj.indent().value());
    }
    if (text_obj.fontStyles().size())
        saveFontStyles(writer, text_obj.fontStyles());
    if (text_obj.block().size())
        saveParagraphs(writer, text_obj);
}

void MoleculeJsonSaver::saveFontStyles(JsonWriter& writer, const FONT_STYLE_SET& fss)
{
    std::vector<std::reference_wrapper<const std::pair<KETFontStyle, bool>>> font_fields;
    for (auto& fs : fss)
    {
        switch (fs.first.getFontStyle())
        {
        case KETFontStyle::FontStyle::EBold:
            writer.Key(KFontBoldStr);
            writer.Bool(fs.second);
            break;
        case KETFontStyle::FontStyle::EItalic:
            writer.Key(KFontItalicStr);
            writer.Bool(fs.second);
            break;
        case KETFontStyle::FontStyle::ESubScript:
            writer.Key(KFontSubscriptStr);
            writer.Bool(fs.second);
            break;
        case KETFontStyle::FontStyle::ESuperScript:
            writer.Key(KFontSuperscriptStr);
            writer.Bool(fs.second);
            break;
        case KETFontStyle::FontStyle::ENone:
            break;
        default:
            if (fs.second)
                font_fields.push_back(std::ref(fs));
            break;
        }
    }

    if (font_fields.size())
    {
        writer.Key("font");
        writer.StartObject();
        for (auto& fs_ref : font_fields)
        {
            auto& fs_font = fs_ref.get();
            if (fs_font.second && fs_font.first.hasValue())
                switch (fs_font.first.getFontStyle())
                {
                case KETFontStyle::FontStyle::EColor: {
                    writer.Key(KFontColorStr);
                    std::stringstream ss;
                    ss << "#" << std::hex << fs_font.first.getUInt().value();
                    writer.String(ss.str());
                }
                break;
                case KETFontStyle::FontStyle::EFamily:
                    writer.Key(KFontFamilyStr);
                    writer.String(fs_font.first.getString().value());
                    break;
                case KETFontStyle::FontStyle::ESize:
                    writer.Key(KFontSizeStr);
                    writer.Uint(fs_font.first.getUInt().value());
                    break;
                }
        }
        writer.EndObject();
    }
}

void MoleculeJsonSaver::saveParagraphs(JsonWriter& writer, const SimpleTextObject& text_obj)
{
    const auto& paragraphs = text_obj.block();
    writer.Key("paragraphs");
    writer.StartArray();
    for (const auto& paragraph : paragraphs)
    {
        writer.StartObject();
        if (paragraph.alignment.has_value())
            saveAlignment(writer, paragraph.alignment.value());
        if (paragraph.indent.has_value())
        {
            writer.Key("indent");
            writer.Double(paragraph.indent.value());
        }

        auto def_fss = text_obj.fontStyles();
        def_fss &= paragraph.font_style;

        if (def_fss.size())
            saveFontStyles(writer, def_fss);

        if (paragraph.font_styles.size())
            saveParts(writer, paragraph, def_fss);
        if (paragraph.line_starts.has_value() && paragraph.line_starts.value().size())
        {
            writer.Key("lineStarts");
            writer.StartArray();
            for (auto ls : paragraph.line_starts.value())
                writer.Uint(ls);
            writer.EndArray();
        }
        writer.EndObject();
    }
    writer.EndArray();
}

void MoleculeJsonSaver::saveParts(JsonWriter& writer, const SimpleTextObject::KETTextParagraph& paragraph, const FONT_STYLE_SET& def_fss)
{
    if (paragraph.font_styles.size() > 1)
    {
        std::string_view text_view = std::string_view(paragraph.text);
        writer.Key("parts");
        writer.StartArray();
        for (auto it_fss_kvp = paragraph.font_styles.begin(); it_fss_kvp != std::prev(paragraph.font_styles.end()); ++it_fss_kvp)
        {
            writer.StartObject();
            auto next_it = std::next(it_fss_kvp);
            auto text_part = text_view.substr(it_fss_kvp->first, next_it->first - it_fss_kvp->first);
            writer.Key("text");
            writer.String(std::string(text_part).c_str());
            auto current_part_fss = def_fss;
            current_part_fss &= it_fss_kvp->second;
            if (current_part_fss.size())
                saveFontStyles(writer, current_part_fss);
            writer.EndObject();
        }
        writer.EndArray();
    }
}

void MoleculeJsonSaver::saveAlignment(JsonWriter& writer, SimpleTextObject::TextAlignment alignment)
{
    std::string alignment_str;
    switch (alignment)
    {
    case SimpleTextObject::TextAlignment::ELeft:
        alignment_str = KAlignmentLeft;
        break;
    case SimpleTextObject::TextAlignment::ERight:
        alignment_str = KAlignmentRight;
        break;
    case SimpleTextObject::TextAlignment::ECenter:
        alignment_str = KAlignmentCenter;
        break;
    case SimpleTextObject::TextAlignment::EFull:
        alignment_str = KAlignmentFull;
        break;
    }
    writer.Key("alignment");
    writer.String(alignment_str.c_str());
}
