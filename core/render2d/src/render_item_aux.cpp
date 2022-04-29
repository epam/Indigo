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

using namespace indigo;

IMPL_ERROR(RenderItemAuxiliary, "RenderItemAuxiliary");

RenderItemAuxiliary::RenderItemAuxiliary(RenderItemFactory& factory)
    : RenderItemBase(factory), arrowLength(_settings.arrowLength), scaleFactor(1.0), mol(nullptr), meta(nullptr)
{
}

RenderItemAuxiliary::~RenderItemAuxiliary()
{
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
    _rc.drawTextItemText(tiR, idle);

    float ypos = tiR.bbp.y + tiR.bbsz.y + _settings.unit;

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
        tiOccurrence.bbp.set(0, ypos);
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
        tiRestH.bbp.set(0, ypos);
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

void RenderItemAuxiliary::_drawArrow()
{
    _rc.setSingleSource(CWC_BASE);
    _rc.drawArrow(Vec2f(0, 0), Vec2f(arrowLength, 0), _settings.metaLineWidth, _settings.arrowHeadWidth, _settings.arrowHeadSize);
}

void RenderItemAuxiliary::_drawMeta(bool idle)
{
    if (meta)
    {
        _rc.setSingleSource(CWC_BASE);
        const auto& md = meta->metaData();
        for (int i = 0; i < md.size(); ++i)
        {
            const auto& simple = *md[i];
            switch (simple._class_id)
            {
            case KETSimpleObject::CID: {
                const KETSimpleObject& ko = static_cast<const KETSimpleObject&>(simple);
                _renderSimpleObject(ko);
            }
            break;
            case KETTextObject::CID: {
                const KETTextObject& ko = static_cast<const KETTextObject&>(simple);
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
    v1.x -= _min.x;
    v2.x -= _min.x;
    v1 *= scaleFactor;
    v2 *= scaleFactor;
    std::tie(v1.y, v2.y) = std::make_pair(_max.y - v1.y, _max.y - v2.y);
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
    if (type == AUX_META && meta)
    {
        const auto& md = meta->metaData();
        for (int i = 0; i < md.size(); ++i)
        {
            const auto& simple = *md[i];
            std::pair<Vec2f, Vec2f> coords;
            Rect2f bbox;
            switch (simple._class_id)
            {
            case KETSimpleObject::CID: {
                auto& obj = (KETSimpleObject&)simple;
                coords = obj._coordinates;
                bbox = Rect2f(coords.first, coords.second);
            }
            break;
            case KETTextObject::CID:
                break;
            case KETReactionArrow::CID:
                break;
            case KETReactionPlus::CID:
                break;
            default:
                break;
            }
            if (i)
            {
                _min.min(bbox.leftBottom());
                _max.max(bbox.rightTop());
            }
            else
            {
                _min.copy(bbox.leftBottom());
                _max.copy(bbox.rightTop());
            }
        }
    }
}
