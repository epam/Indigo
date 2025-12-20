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

#include "render_item_aux.h"
#include "base_cpp/output.h"
#include "layout/metalayout.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "render_context.h"
#include "render_internal.h"
#include <codecvt>
#include <regex>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#pragma warning(disable : 4251)
#endif

#include <lunasvg.h>

using namespace indigo;

IMPL_ERROR(RenderItemAuxiliary, "RenderItemAuxiliary");

RenderItemAuxiliary::RenderItemAuxiliary(RenderItemFactory& factory)
    : RenderItemBase(factory), arrowLength(_settings.arrowLength), scaleFactor(1.0), offset(0, 0), mol(nullptr), meta(nullptr), rLabelIdx(0),
      type(AUX_NOT_INITIALIZED), hasOffset(false)
{
}

RenderItemAuxiliary::~RenderItemAuxiliary()
{
}

void RenderItemAuxiliary::_drawText(TextItem& ti, bool idle)
{
    _rc.setTextItemSize(ti);
    _rc.drawTextItemText(ti, idle);
}

void RenderItemAuxiliary::_drawText(bool idle)
{
    TextItem ti;
    ti.text.copy(text);
    if (type == AUX_COMMENT)
    {
        ti.fontsize = FONT_SIZE_COMMENT;
        ti.ritype = RenderItem::RIT_COMMENT;
    }
    else if (type == AUX_TITLE)
    {
        ti.fontsize = FONT_SIZE_TITLE;
        ti.ritype = RenderItem::RIT_TITLE;
    }
    else
    {
        throw Error("Font size unknown");
    }
    _rc.setTextItemSize(ti);
    ti.bbp.set(0, 0);
    _rc.drawTextItemText(ti, idle);
}

void RenderItemAuxiliary::_drawRGroupLabel(bool idle)
{
    BaseMolecule& bm = *mol;
    MoleculeRGroups& rgs = bm.rgroups;
    RGroup& rg = rgs.getRGroup(rLabelIdx);

    TextItem tiR;
    tiR.fontsize = FONT_SIZE_LABEL;
    tiR.color = CWC_BASE;
    bprintf(tiR.text, "R%d=", rLabelIdx);
    _rc.setTextItemSize(tiR);
    referenceY = tiR.bbsz.y / 2;
    tiR.bbp.set(0, 0);
    if (hasOffset)
    {
        tiR.bbp.copy(offset);
        scale(tiR.bbp);
        tiR.bbp.x -= (tiR.bbsz.x + _settings.layoutMarginHorizontal);
        tiR.bbp.y -= referenceY;
    }

    _rc.drawTextItemText(tiR, idle);

    float ypos = tiR.bbp.y + tiR.bbsz.y + _settings.unit;
    float xpos = tiR.bbp.x;

    if (rg.occurrence.size() > 0)
    {
        TextItem tiOccurrence;
        tiOccurrence.fontsize = FONT_SIZE_RGROUP_LOGIC_INDEX;
        tiOccurrence.color = CWC_BASE;
        ArrayOutput output(tiOccurrence.text);
        for (int i = 0; i < rg.occurrence.size(); ++i)
        {
            int v = rg.occurrence[i];
            int a = (v >> 16) & 0xFFFF;
            int b = v & 0xFFFF;
            if (i > 0)
                output.printf(", ");
            if (a == b)
                output.printf("%d", a);
            else if (a == 0)
                output.printf("<%d", b + 1);
            else if (b == 0xFFFF)
                output.printf(">%d", a - 1);
            else
                output.printf("%d-%d", a, b);
        }
        output.writeByte(0);

        _rc.setTextItemSize(tiOccurrence);
        tiOccurrence.bbp.set(xpos, ypos);
        _rc.drawTextItemText(tiOccurrence, idle);

        ypos += tiOccurrence.bbsz.y + _settings.unit;
    }

    if (rg.rest_h > 0)
    {
        TextItem tiRestH;
        tiRestH.fontsize = FONT_SIZE_RGROUP_LOGIC_INDEX;
        tiRestH.color = CWC_BASE;
        bprintf(tiRestH.text, "RestH");
        _rc.setTextItemSize(tiRestH);
        tiRestH.bbp.set(xpos, ypos);
        _rc.drawTextItemText(tiRestH, idle);
    }
}

void RenderItemAuxiliary::_drawRIfThen(bool idle)
{
    BaseMolecule& bm = *mol;
    MoleculeRGroups& rgs = bm.rgroups;

    float ypos = 0;
    for (int i = 1; i <= rgs.getRGroupCount(); ++i)
    {
        const RGroup& rg = rgs.getRGroup(i);
        if (rg.if_then > 0)
        {
            TextItem tiIfThen;
            tiIfThen.fontsize = FONT_SIZE_RGROUP_LOGIC;
            tiIfThen.color = CWC_BASE;
            bprintf(tiIfThen.text, "IF R%d THEN R%d", i, rg.if_then);
            _rc.setTextItemSize(tiIfThen);
            tiIfThen.bbp.set(0, ypos);
            _rc.drawTextItemText(tiIfThen, idle);
            ypos += tiIfThen.bbsz.y + _settings.rGroupIfThenInterval;
        }
    }
}

void RenderItemAuxiliary::_drawPlus()
{
    _rc.setSingleSource(CWC_BASE);
    _rc.drawPlus(Vec2f(_settings.plusSize / 2, 0), _settings.metaLineWidth, _settings.plusSize);
}

void RenderItemAuxiliary::_drawArrow(const ReactionArrowObject& ar)
{
    _rc.setSingleSource(CWC_BASE);
    Vec2f beg = ar.getTail();
    Vec2f end = ar.getHead();
    scale(beg);
    scale(end);
    switch (ar.getArrowType())
    {
    case ReactionArrowObject::EOpenAngle:
        _rc.drawCustomArrow(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize, false, false);
        break;

    case ReactionArrowObject::EFilledBow:
        _rc.drawCustomArrow(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize, true, false);
        break;

    case ReactionArrowObject::EFailed:
        _rc.drawCustomArrow(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize, true, true);
        break;

    case ReactionArrowObject::EDashedOpenAngle:
        _rc.drawDashedArrow(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize);
        break;

    case ReactionArrowObject::EBothEndsFilledTriangle:
        _rc.drawBothEndsArrow(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize);
        break;

    case ReactionArrowObject::EEquilibriumFilledHalfBow:
        _rc.drawEquillibriumHalf(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize, RenderContext::ArrowType::EBowArray);
        break;

    case ReactionArrowObject::EEquilibriumFilledTriangle:
        _rc.drawEquillibriumFilledTriangle(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize);
        break;

    case ReactionArrowObject::EEquilibriumOpenAngle:
        _rc.drawEquillibriumHalf(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize);
        break;

    case ReactionArrowObject::EUnbalancedEquilibriumLargeFilledHalfBow:
        _rc.drawEquillibriumHalf(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize, RenderContext::ArrowType::EBowArray,
                                 true, true);
        break;

    case ReactionArrowObject::EUnbalancedEquilibriumFilledHalfBow:
        _rc.drawEquillibriumHalf(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize, RenderContext::ArrowType::EBowArray,
                                 false, true);
        break;

    case ReactionArrowObject::EUnbalancedEquilibriumOpenHalfAngle:
        _rc.drawEquillibriumHalf(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize, RenderContext::ArrowType::EOpenArrow,
                                 false, true);
        break;

    case ReactionArrowObject::EUnbalancedEquilibriumFilledHalfTriangle:
        _rc.drawEquillibriumHalf(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize, RenderContext::ArrowType::ETriangleArrow,
                                 false, true);
        break;

    case ReactionArrowObject::EEllipticalArcFilledBow:
        _rc.drawEllipticalArrow(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize, ar.getHeight(), ar.getArrowType());
        break;

    case ReactionArrowObject::EEllipticalArcFilledTriangle:
        _rc.drawEllipticalArrow(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize, ar.getHeight(), ar.getArrowType());
        break;

    case ReactionArrowObject::EEllipticalArcOpenAngle:
        _rc.drawEllipticalArrow(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize, ar.getHeight(), ar.getArrowType());
        break;

    case ReactionArrowObject::EEllipticalArcOpenHalfAngle:
        _rc.drawEllipticalArrow(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize, ar.getHeight(), ar.getArrowType());
        break;

    case ReactionArrowObject::ERetrosynthetic:
        _rc.drawRetroSynthArrow(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize);
        break;

    default:
        _rc.drawArrow(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize);
        break;
    }
}

void RenderItemAuxiliary::_drawArrow(const ReactionMultitailArrowObject& ar)
{
    _rc.setLineWidth(_settings.bondLineWidth);
    const float radius = ReactionMultitailArrowObject::TAIL_ARC_RADIUS;
    // In order to avoid a slight gap, that becomes apparent with the highest zoom.
    const float gap = .01f;

    Vec2f arrowBeg = {ar.getSpineBegin().x, ar.getHead().y}, arrowEnd = ar.getHead();
    scale(arrowBeg);
    scale(arrowEnd);
    _rc.drawArrow(arrowBeg, arrowEnd, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize);

    Vec2f spineBeg = ar.getSpineBegin(), spineEnd = ar.getSpineEnd();
    scale(spineBeg);
    scale(spineEnd);
    _rc.drawLine(spineBeg + Vec2f(.0, radius - gap), spineEnd + Vec2f(.0, -radius + gap));

    for (int i = 0; i < ar.getTails().size(); i++)
    {
        auto tail = ar.getTails().at(i);
        scale(tail);
        _rc.drawLine(tail, {spineBeg.x - (i == 0 || i == ar.getTails().size() - 1 ? radius - gap : 0), tail.y});
    }

    spineBeg += Vec2f(-radius, radius);
    _rc.drawArc(spineBeg, radius, -static_cast<float>(M_PI) / 2.f, .0f);
    spineEnd += Vec2f(-radius, -radius);
    _rc.drawArc(spineEnd, radius, .0f, static_cast<float>(M_PI) / 2.f);
}

void RenderItemAuxiliary::_drawArrow()
{
    _rc.setSingleSource(CWC_BASE);
    _rc.drawArrow(Vec2f(0, 0), Vec2f(arrowLength, 0), _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize);
}

void RenderItemAuxiliary::fillKETStyle(TextItem& ti, const FONT_STYLE_SET& style_set)
{
    for (const auto& text_style : style_set)
    {
        switch (static_cast<KETFontStyle::FontStyle>(text_style.first))
        {
        case KETFontStyle::FontStyle::EBold:
            ti.bold = text_style.second;
            break;
        case KETFontStyle::FontStyle::EItalic:
            ti.italic = text_style.second;
            break;
        case KETFontStyle::FontStyle::ESuperScript:
            ti.script_type = text_style.second ? 1 : 0;
            break;
        case KETFontStyle::FontStyle::ESubScript:
            ti.script_type = text_style.second ? 2 : 0;
            break;
        case KETFontStyle::FontStyle::ESize: {
            ti.size = KDefaultFontSize;
            auto sz_val = text_style.first.getUInt();
            if (text_style.second && sz_val.has_value())
                ti.size = sz_val.value();
            ti.size /= KFontScaleFactor;
        }
        break;
        case KETFontStyle::FontStyle::EColor: {
            auto color_val = text_style.first.getUInt();
            if (text_style.second && color_val.has_value())
            {
                ti.color = color_val.value();

                ti.rgb_color = ti.color == CWC_BASE
                                   ? Vec3f(0, 0, 0)
                                   : Vec3f((float)((ti.color >> 16) & 0xFF) / 255.0f, ti.rgb_color.y = (float)((ti.color >> 8) & 0xFF) / 255.0f,
                                           ti.rgb_color.z = (float)(ti.color & 0xFF) / 255.0f);
            }
            else
            {
                ti.color = CWC_BASE;
                ti.rgb_color = Vec3f(0, 0, 0);
            }
        }
        default:
            break;
        }
    }
}

void RenderItemAuxiliary::_drawMeta(bool idle)
{
    if (meta)
    {
        _rc.setSingleSource(CWC_BASE);
        const auto& md = meta->metaData();
        // images go first
        std::vector<int> order_indexes, back_indexes;
        for (int i = 0; i < md.size(); ++i)
        {
            const auto& mobj = *md[i];
            if (mobj._class_id == EmbeddedImageObject::CID)
                order_indexes.push_back(i);
            else
                back_indexes.push_back(i);
        }
        order_indexes.insert(order_indexes.end(), back_indexes.begin(), back_indexes.end());

        for (auto i : order_indexes)
        {
            const auto& mobj = *md[i];
            switch (mobj._class_id)
            {
            case SimpleGraphicsObject::CID: {
                const SimpleGraphicsObject& ko = static_cast<const SimpleGraphicsObject&>(mobj);
                _renderSimpleObject(ko);
            }
            break;
            case SimpleTextObject::CID: {
                const SimpleTextObject& ko = static_cast<const SimpleTextObject&>(mobj);
                double text_offset_y = 0;
                auto default_styles = ko.fontStyles();
                for (auto& text_item : ko.block())
                {
                    auto paragraph_style = default_styles;
                    paragraph_style += text_item.font_style;
                    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> utf8w;

                    auto text_wstr = utf8w.from_bytes(text_item.text);
                    float text_max_height = _getMaxHeight(text_item);
                    auto line_starts = text_item.line_starts;
                    int first_index = -1;
                    int second_index = -1;
                    float text_offset_x = 0;
                    auto indent = ko.indent();
                    if (text_item.indent.has_value())
                        indent = text_item.indent;

                    FONT_STYLE_SET current_styles;
                    ObjArray<ObjArray<TextItem>> ti_lines;
                    std::vector<std::pair<int, float>> spaces_widths;
                    std::pair<int, float> trailing_spaces{};
                    TextItem ti;
                    ti.size = KDefaultFontSize / KFontScaleFactor; // default size
                    ti.ritype = RenderItem::RIT_TITLE;
                    Vec2f text_origin(ko.boundingBox().left(), ko.boundingBox().top());
                    scale(text_origin);
                    float text_width = ko.boundingBox().width() * scaleFactor;
                    for (auto& kvp : text_item.font_styles)
                    {
                        if (first_index == -1)
                        {
                            first_index = static_cast<int>(kvp.first);
                            current_styles = paragraph_style;
                            current_styles += kvp.second;
                            continue;
                        }
                        second_index = static_cast<int>(kvp.first);

                        std::vector<std::string> styled_lines;
                        while (line_starts.has_value() && line_starts.value().size() && *line_starts.value().begin() <= second_index &&
                               *line_starts.value().begin() > first_index)
                        {
                            auto ls_index = *line_starts.value().begin();
                            line_starts.value().erase(line_starts.value().begin());
                            auto text_wsubstr = text_wstr.substr(first_index, ls_index - first_index);
                            if (text_wsubstr.back() != '\n' && text_wsubstr.back() != '\r')
                                text_wsubstr.push_back('\n');
                            styled_lines.push_back(utf8w.to_bytes(text_wsubstr));
                            first_index = ls_index;
                        }

                        if (second_index > first_index)
                            styled_lines.push_back(utf8w.to_bytes(text_wstr.substr(first_index, second_index - first_index)));

                        fillKETStyle(ti, current_styles);

                        for (auto& styled_text : styled_lines)
                        {
                            auto splitted = split_to_lines(styled_text);
                            for (auto line_it = splitted.begin(); line_it != splitted.end(); ++line_it)
                            {
                                if (ti_lines.size() == 0)
                                {
                                    ti_lines.push();
                                    spaces_widths.emplace_back(0, 0.0f);
                                }

                                auto& ti_line = ti_lines.top();
                                auto splitted_spaces = split_spaces(*line_it);

                                for (auto& line_str : splitted_spaces)
                                {
                                    auto spc_count = (int)std::count(line_str.begin(), line_str.end(), ' ');
                                    ti.text.readString(line_str.c_str(), true);
                                    _rc.setTextItemSize(ti);
                                    ti.bbp.x = static_cast<float>(text_origin.x - ti.relpos.x + text_offset_x);
                                    trailing_spaces.first =
                                        (int)std::distance(line_str.rbegin(),
                                                           std::find_if(line_str.rbegin(), line_str.rend(), [](char c) {
                                                               return c != ' ';
                                                           }));
                                    trailing_spaces.second = trailing_spaces.first ? _rc.getSpaceWidth() * trailing_spaces.first : 0.0f;

                                    if (spc_count)
                                    {
                                        text_offset_x += _rc.getSpaceWidth() * spc_count;
                                        spaces_widths.back().first += spc_count;
                                        spaces_widths.back().second += _rc.getSpaceWidth() * spc_count;
                                        ti.bbsz.x = 0;
                                        ti.text.readString(" ", true);
                                    }
                                    else
                                    {
                                        ti.bbp.y = static_cast<float>(text_origin.y - ti.relpos.y + text_max_height / 2 + text_offset_y);
                                        text_offset_x += ti.bbsz.x;
                                    }
                                    ti_line.push(ti);
                                }

                                if (splitted.size() > 1 && std::next(line_it) != splitted.end())
                                {
                                    text_offset_y += text_max_height + _settings.boundExtent;
                                    text_offset_x = 0;
                                    if (trailing_spaces.first)
                                    {
                                        spaces_widths.back().first -= trailing_spaces.first;
                                        spaces_widths.back().second -= trailing_spaces.second;
                                    }
                                    ti_lines.push();
                                    spaces_widths.emplace_back(0, 0.0f);
                                }
                            }
                        }
                        current_styles = kvp.second;
                        first_index = second_index;
                    }

                    if (ti_lines.size() && !ti_lines[ti_lines.size() - 1].size())
                    {
                        text_offset_y -= text_max_height + _settings.boundExtent;
                        ti_lines.remove(ti_lines.size() - 1);
                    }

                    for (int j = 0; j < ti_lines.size(); ++j)
                    {
                        auto& ti_line = ti_lines[j];
                        float align_offset = 0;
                        float indent_offset = (j == 0 && indent.has_value()) ? indent.value() : 0.0f;

                        // calculate alignment offsets
                        if (text_item.alignment.has_value() && text_item.alignment.value() != SimpleTextObject::TextAlignment::ELeft)
                        {
                            float line_width = 0;
                            for (int k = 0; k < ti_line.size(); ++k)
                            {
                                auto& ti_rc = ti_line[k];
                                line_width += ((k + 1) < ti_line.size() && text_item.alignment.value() != SimpleTextObject::TextAlignment::EFull)
                                                  ? (ti_line[k + 1].bbp.x - ti_rc.bbp.x)
                                                  : ti_rc.bbsz.x;
                            }

                            float space_width = text_width - line_width - indent_offset;

                            switch (text_item.alignment.value())
                            {
                            case SimpleTextObject::TextAlignment::ECenter:
                                align_offset = space_width / 2;
                                break;
                            case SimpleTextObject::TextAlignment::ERight:
                                align_offset = space_width;
                                break;
                            case SimpleTextObject::TextAlignment::EFull:
                                if (j < ti_lines.size() - 1)
                                {
                                    text_offset_x = 0;
                                    if (spaces_widths[j].first)
                                        space_width /= spaces_widths[j].first;
                                    // iterate line elements and calculate new positions
                                    for (int k = 0; k < ti_line.size(); ++k)
                                    {
                                        auto& ti_rc = ti_line[k];
                                        ti_rc.bbp.x = static_cast<float>(text_origin.x - ti_rc.relpos.x + text_offset_x);
                                        text_offset_x += ti_rc.bbsz.x;
                                        if (std::string(ti_rc.text.ptr()) == " ")
                                            text_offset_x += space_width;
                                    }
                                }
                                break;
                            }
                        }

                        // draw text
                        for (int k = 0; k < ti_line.size(); ++k)
                        {
                            auto& ti_rc = ti_line[k];
                            ti_rc.bbp.x += align_offset + indent_offset;
                            _rc.drawTextItemText(ti_rc, ti_rc.rgb_color, idle);
                        }
                    }
                    text_offset_y += text_max_height + _settings.boundExtent;
                }
            }
            break;
            case ReactionPlusObject::CID: {
                const ReactionPlusObject& rp = static_cast<const ReactionPlusObject&>(mobj);
                _rc.setSingleSource(CWC_BASE);
                Vec2f plus_pos = rp.getPos();
                scale(plus_pos);
                _rc.drawPlus(plus_pos, _settings.metaLineWidth, _settings.plusSize);
            }
            break;
            case ReactionArrowObject::CID: {
                const ReactionArrowObject& ar = static_cast<const ReactionArrowObject&>(mobj);
                _drawArrow(ar);
            }
            break;
            case ReactionMultitailArrowObject::CID: {
                const ReactionMultitailArrowObject& ar = static_cast<const ReactionMultitailArrowObject&>(mobj);
                _drawArrow(ar);
            }
            break;
            case EmbeddedImageObject::CID: {
                const EmbeddedImageObject& img = static_cast<const EmbeddedImageObject&>(mobj);
                _drawImage(img);
            }
            break;
            }
        }
    }
}

void lunasvgWrite(void* context, void* data, int size)
{
    static_cast<std::string*>(context)->assign(static_cast<const char*>(data), size);
}

void RenderItemAuxiliary::_drawImage(const EmbeddedImageObject& img)
{
    auto& bb = img.getBoundingBox();
    auto v1 = bb.leftBottom();
    auto v2 = bb.rightTop();
    scale(v1);
    scale(v2);
    if (img.getFormat() == EmbeddedImageObject::EKETPNG)
        _rc.drawPng(img.getData(), Rect2f(v1, v2));
    else if (img.getFormat() == EmbeddedImageObject::EKETSVG)
    {
        auto document = lunasvg::Document::loadFromData(img.getData());
        if (!document)
            throw Error("RenderItemAuxiliary::_drawImage: loadFromData error");

        auto bitmap = document->renderToBitmap();
        if (bitmap.isNull())
            throw Error("RenderItemAuxiliary::_drawImage: renderToBitmap error");

        std::string lunasvgClosure;
        if (!bitmap.writeToPng(lunasvgWrite, &lunasvgClosure))
            throw Error("RenderItemAuxiliary::_drawImage: writeToPng error");

        _rc.drawPng(lunasvgClosure, Rect2f(v1, v2));
    }
}

void RenderItemAuxiliary::_renderSimpleObject(const SimpleGraphicsObject& simple)
{
    _rc.setLineWidth(_settings.bondLineWidth);
    _rc.setSingleSource(CWC_BASE);

    auto v1 = simple._coordinates.first;
    auto v2 = simple._coordinates.second;
    scale(v1);
    scale(v2);
    Rect2f rc(v1, v2);

    switch (simple._mode)
    {
    case SimpleGraphicsObject::EEllipse:
        _rc.drawEllipse(v1, v2);
        break;

    case SimpleGraphicsObject::ERectangle: {
        Array<Vec2f> pts;
        pts.push() = rc.leftTop();
        pts.push() = rc.rightTop();
        pts.push() = rc.rightBottom();
        pts.push() = rc.leftBottom();
        pts.push() = rc.leftTop();
        _rc.drawPoly(pts);
    }
    break;

    case SimpleGraphicsObject::ELine: {
        Array<Vec2f> pts;
        auto& vec1 = pts.push();
        auto& vec2 = pts.push();
        vec1 = v1;
        vec2 = v2;
        _rc.drawPoly(pts);
    }
    break;
    }
}

void RenderItemAuxiliary::_renderIdle()
{
    _rc.initNullContext();
    Vec2f bbmin, bbmax;
    Vec2f pos;
    render(true);
    _rc.bbGetMin(bbmin);
    _rc.bbGetMax(bbmax);
    _rc.closeContext(true);
    size.diff(bbmax, bbmin);
    origin.copy(bbmin);
}

void RenderItemAuxiliary::render(bool idle)
{
    _rc.translate(-origin.x, -origin.y);
    switch (type)
    {
    case AUX_COMMENT:
    case AUX_TITLE:
        _drawText(idle);
        return;
    case AUX_RXN_PLUS:
        _drawPlus();
        return;
    case AUX_RXN_ARROW:
        _drawArrow();
        return;
    case AUX_RGROUP_LABEL:
        _drawRGroupLabel(idle);
        return;
    case AUX_RGROUP_IFTHEN:
        _drawRIfThen(idle);
        return;
    case AUX_META:
        _drawMeta(idle);
        return;
    default:
        throw Error("Item type not set or invalid");
    }
}

void RenderItemAuxiliary::init()
{
}

float RenderItemAuxiliary::_getMaxHeight(const SimpleTextObject::KETTextParagraph& tl)
{
    int first_index = -1;
    int second_index = -1;
    FONT_STYLE_SET current_styles;
    TextItem ti;
    ti.size = KDefaultFontSize / KFontScaleFactor; // default size
    ti.ritype = RenderItem::RIT_TITLE;
    _rc.setTextItemSize(ti);
    float sz = (float)ti.bbsz.y;
    for (auto& kvp : tl.font_styles)
    {
        if (first_index == -1)
        {
            first_index = static_cast<int>(kvp.first);
            current_styles = kvp.second;
            continue;
        }
        second_index = static_cast<int>(kvp.first);
        std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8w;
        auto sub_text = utf8w.to_bytes(utf8w.from_bytes(tl.text).substr(first_index, second_index - first_index));
        ti.text.readString(sub_text.c_str(), true);
        fillKETStyle(ti, current_styles);
        _rc.setTextItemSize(ti);
        sz = std::max(sz, (float)ti.bbsz.y);
        current_styles = kvp.second;
        first_index = second_index;
    }
    return sz;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
