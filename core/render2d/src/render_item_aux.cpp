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

void RenderItemAuxiliary::_drawArrow(const KETReactionArrow& ar)
{
    _rc.setSingleSource(CWC_BASE);
    auto beg = ar._begin;
    auto end = ar._end;
    scale(beg);
    scale(end);
    switch (ar._arrow_type)
    {
    case KETReactionArrow::EOpenAngle:
        _rc.drawCustomArrow(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize, false, false);
        break;

    case KETReactionArrow::EFilledBow:
        _rc.drawCustomArrow(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize, true, false);
        break;

    case KETReactionArrow::EFailed:
        _rc.drawCustomArrow(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize, true, true);
        break;

    case KETReactionArrow::EDashedOpenAngle:
        _rc.drawDashedArrow(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize);
        break;

    case KETReactionArrow::EBothEndsFilledTriangle:
        _rc.drawBothEndsArrow(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize);
        break;

    case KETReactionArrow::EEquilibriumFilledHalfBow:
        _rc.drawEquillibriumHalf(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize, RenderContext::ArrowType::EBowArray);
        break;

    case KETReactionArrow::EEquilibriumFilledTriangle:
        _rc.drawEquillibriumFilledTriangle(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize);
        break;

    case KETReactionArrow::EEquilibriumOpenAngle:
        _rc.drawEquillibriumHalf(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize);
        break;

    case KETReactionArrow::EUnbalancedEquilibriumLargeFilledHalfBow:
        _rc.drawEquillibriumHalf(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize, RenderContext::ArrowType::EBowArray,
                                 true, true);
        break;

    case KETReactionArrow::EUnbalancedEquilibriumFilledHalfBow:
        _rc.drawEquillibriumHalf(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize, RenderContext::ArrowType::EBowArray,
                                 false, true);
        break;

    case KETReactionArrow::EUnbalancedEquilibriumOpenHalfAngle:
        _rc.drawEquillibriumHalf(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize, RenderContext::ArrowType::EOpenArrow,
                                 false, true);
        break;

    case KETReactionArrow::EUnbalancedEquilibriumFilledHalfTriangle:
        _rc.drawEquillibriumHalf(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize, RenderContext::ArrowType::ETriangleArrow,
                                 false, true);
        break;

    case KETReactionArrow::EEllipticalArcFilledBow:
        _rc.drawEllipticalArrow(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize, ar._height, ar._arrow_type);
        break;

    case KETReactionArrow::EEllipticalArcFilledTriangle:
        _rc.drawEllipticalArrow(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize, ar._height, ar._arrow_type);
        break;

    case KETReactionArrow::EEllipticalArcOpenAngle:
        _rc.drawEllipticalArrow(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize, ar._height, ar._arrow_type);
        break;

    case KETReactionArrow::EEllipticalArcOpenHalfAngle:
        _rc.drawEllipticalArrow(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize, ar._height, ar._arrow_type);
        break;

    default:
        _rc.drawArrow(beg, end, _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize);
        break;
    }
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
        switch (text_style.first)
        {
        case KETTextObject::EBold:
            ti.bold = text_style.second;
            break;
        case KETTextObject::EItalic:
            ti.italic = text_style.second;
            break;
        case KETTextObject::ESuperScript:
            ti.script_type = text_style.second ? 1 : 0;
            break;
        case KETTextObject::ESubScript:
            ti.script_type = text_style.second ? 2 : 0;
            break;
        default:
            ti.size = text_style.second ? text_style.first : KETDefaultFontSize;
            ti.size /= KETFontScaleFactor;
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
        for (int i = 0; i < md.size(); ++i)
        {
            const auto& mobj = *md[i];
            switch (mobj._class_id)
            {
            case KETSimpleObject::CID: {
                const KETSimpleObject& ko = static_cast<const KETSimpleObject&>(mobj);
                _renderSimpleObject(ko);
            }
            break;
            case KETTextObject::CID: {
                const KETTextObject& ko = static_cast<const KETTextObject&>(mobj);
                double text_offset_y = 0;
                for (auto& text_item : ko._block)
                {
                    float text_max_height = _getMaxHeight(text_item);
                    int first_index = -1;
                    int second_index = -1;
                    double text_offset_x = 0;
                    FONT_STYLE_SET current_styles;
                    TextItem ti;
                    ti.size = KETDefaultFontSize / KETFontScaleFactor; // default size
                    ti.ritype = RenderItem::RIT_TITLE;
                    Vec2f text_origin(ko._pos.x, ko._pos.y);
                    scale(text_origin);
                    for (auto& kvp : text_item.styles)
                    {
                        if (first_index == -1)
                        {
                            first_index = kvp.first;
                            current_styles = kvp.second;
                            continue;
                        }
                        second_index = kvp.first;

                        std::wstring_convert<std::codecvt_utf8<wchar_t>> utf82w;
                        std::wstring_convert<std::codecvt_utf8<wchar_t>> w2utf8;

                        auto sub_text = w2utf8.to_bytes(utf82w.from_bytes(text_item.text).substr(first_index, second_index - first_index));
                        ti.text.readString(sub_text.c_str(), true);
                        fillKETStyle(ti, current_styles);
                        _rc.setTextItemSize(ti);
                        ti.bbp.x = text_origin.x - ti.relpos.x + text_offset_x;
                        ti.bbp.y = text_origin.y - ti.relpos.y + text_max_height / 2 + text_offset_y;
                        _rc.drawTextItemText(ti, Vec3f(0, 0, 0), idle);

                        text_offset_x += ti.bbsz.x + _settings.boundExtent;
                        current_styles = kvp.second;
                        first_index = second_index;
                    }
                    text_offset_y += text_max_height + _settings.boundExtent;
                    text_offset_x = 0;
                }
            }
            break;
            case KETReactionPlus::CID: {
                const KETReactionPlus& rp = static_cast<const KETReactionPlus&>(mobj);
                _rc.setSingleSource(CWC_BASE);
                auto plus_pos = rp._pos;
                scale(plus_pos);
                _rc.drawPlus(plus_pos, _settings.metaLineWidth, _settings.plusSize);
            }
            break;
            case KETReactionArrow::CID: {
                const KETReactionArrow& ar = static_cast<const KETReactionArrow&>(mobj);
                _drawArrow(ar);
            }
            break;
            }
        }
    }
}

void RenderItemAuxiliary::_renderSimpleObject(const KETSimpleObject& simple)
{
    _rc.setLineWidth(_settings.bondLineWidth);

    auto v1 = simple._coordinates.first;
    auto v2 = simple._coordinates.second;
    scale(v1);
    scale(v2);
    Rect2f rc(v1, v2);

    switch (simple._mode)
    {
    case KETSimpleObject::EKETEllipse:
        _rc.drawEllipse(v1, v2);
        break;

    case KETSimpleObject::EKETRectangle: {
        Array<Vec2f> pts;
        pts.push() = rc.leftTop();
        pts.push() = rc.rightTop();
        pts.push() = rc.rightBottom();
        pts.push() = rc.leftBottom();
        pts.push() = rc.leftTop();
        _rc.drawPoly(pts);
    }
    break;

    case KETSimpleObject::EKETLine: {
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

float RenderItemAuxiliary::_getMaxHeight(const KETTextObject::KETTextLine& tl)
{
    int first_index = -1;
    int second_index = -1;
    FONT_STYLE_SET current_styles;
    float sz = 0;
    TextItem ti;
    ti.size = KETDefaultFontSize / KETFontScaleFactor; // default size
    ti.ritype = RenderItem::RIT_TITLE;
    for (auto& kvp : tl.styles)
    {
        if (first_index == -1)
        {
            first_index = kvp.first;
            current_styles = kvp.second;
            continue;
        }
        second_index = kvp.first;

        std::wstring_convert<std::codecvt_utf8<wchar_t>> utf82w;
        std::wstring_convert<std::codecvt_utf8<wchar_t>> w2utf8;
        auto sub_text = w2utf8.to_bytes(utf82w.from_bytes(tl.text).substr(first_index, second_index - first_index));

        ti.text.readString(sub_text.c_str(), true);
        fillKETStyle(ti, current_styles);
        _rc.setTextItemSize(ti);
        sz = std::max(sz, (float)ti.bbsz.y);
        current_styles = kvp.second;
        first_index = second_index;
    }
    return sz;
}
