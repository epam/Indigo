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

#include "render_common.h"
#include "base_cpp/array.h"
#include "base_cpp/obj_array.h"
#include "math/algebra.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction.h"

using namespace indigo;

namespace indigo
{
    // cos(a) to cos(a/2)
    double cos2c(const double cs)
    {
        return sqrt((1 + cs) / 2);
    }

    // cos(a) to sin(a/2)
    double sin2c(const double cs)
    {
        return sqrt((1 - cs) / 2);
    }

    // cos(a) to tg(a/2)
    double tg2c(const double cs)
    {
        return sqrt((1 - cs) / (1 + cs));
    }

    // cos(a) to ctg(a/2)
    double ctg2c(const double cs)
    {
        return sqrt((1 + cs) / (1 - cs));
    }
} // namespace indigo

RenderItem::RenderItem()
{
    clear();
}

RenderItem::RenderItem(const RenderItem& ri)
{
    ritype = ri.ritype;
    bbp.copy(ri.bbp);
    bbsz.copy(ri.bbsz);
    relpos.copy(ri.relpos);
    color = ri.color;
    highlighted = ri.highlighted;
    noBondOffset = ri.noBondOffset;
}

void RenderItem::clear()
{
    ritype = RIT_NULL;
    bbp.set(0, 0);
    bbsz.set(0, 0);
    relpos.set(0, 0);
    color = CWC_BASE;
    highlighted = false;
    noBondOffset = false;
}

void TextItem::clear()
{
    RenderItem::clear();
    text.clear();
    size = -0.0;
    bold = false;
    italic = false;
    script_type = 0;
}

void GraphItem::clear()
{
    RenderItem::clear();
}

void RenderItemAttachmentPoint::clear()
{
    RenderItem::clear();
    number = -1;
}

void RenderItemBracket::clear()
{
    p0.set(0, 0);
    p1.set(0, 0);
    q0.set(0, 0);
    q1.set(0, 0);
    d.set(0, 0);
    n.set(0, 0);
    length = 0;
    width = 0;
}

void RenderItemRSiteAttachmentIndex::clear()
{
    RenderItem::clear();
    number = -1;
    radius = 0;
}

AtomDesc::AtomDesc()
{
    clear();
}

void AtomDesc::clear()
{
    showLabel = showHydro = true;
    tibegin = gibegin = -1;
    ticount = gicount = 0;
    attachmentPointBegin = -1;
    attachmentPointCount = 0;
    rSiteAttachmentIndexBegin = -1;
    rSiteAttachmentIndexCount = 0;
    stereoGroupType = stereoGroupNumber = -1;
    isRGroupAttachmentPoint = false;
    pseudoAtomStringVerbose = false;
    hcolorSet = false;
    fixed = false;
    exactChange = false;
    color = CWC_BASE;
    implicit_h = 0;
    hydroPos = HYDRO_POS_RIGHT;
    aam = -1;
    inversion = STEREO_UNMARKED;
    nearbyAtoms.clear();
    list.clear();
    pseudo.clear();
    memset(implHPosWeights, 0, sizeof(implHPosWeights));
    upperSin = lowerSin = rightSin = leftSin = 0;
    pos.set(0, 0);
    boundBoxMin.set(0, 0);
    boundBoxMax.set(0, 0);
    type = 0;
    label = queryLabel = 0;
    leftMargin = rightMargin = 0;
    ypos = 0;
    height = 0;
}

Sgroup::Sgroup()
{
    clear();
}

void Sgroup::clear()
{
    tibegin = gibegin = bibegin = -1;
    ticount = gicount = bicount = 0;
    hide_brackets = false;
}

BondEnd::BondEnd()
{
    clear();
}

void BondEnd::clear()
{
    lRing = next = -1;
    centered = false;
    prolong = false;
    lang = (float)(2 * M_PI);
    rang = (float)(2 * M_PI);
    rcos = lcos = -1;
    rsin = lsin = 0;
    rnei = lnei = -1;
    offset = 0;
    width = 0;
    aid = 0;
    bid = 0;
    rnei = lnei = -1;
    dir = lnorm = p = {0, 0};
}

IMPL_ERROR(BondDescr, "molrender bond description");

BondDescr::BondDescr()
{
    clear();
}

void BondDescr::clear()
{
    be2 = be1 = -1;
    type = -1;
    queryType = -1;
    inRing = false;
    aromRing = false;
    stereoCare = false;
    thickness = 0.0f;
    stereodir = 0;
    cistrans = false;
    centered = false;
    extP = extN = 0;
    bahs = eahs = 0;
    tiTopology = -1;
    topology = 0;
    reactingCenter = RC_UNMARKED;
    lineOnTheRight = false;
    isShort = false;
    length = 0;
    norm = dir = vb = ve = center = {0, 0};
}

int BondDescr::getBondEnd(int aid) const
{
    if (aid == beg)
        return be1;
    if (aid == end)
        return be2;
    throw Error("atom given is not adjacent to the bond");
}

Ring::Ring()
{
    clear();
}

void Ring::clear()
{
    bondEnds.clear();
    angles.clear();
    dblBondCount = 0;
    aromatic = true;
    center.set(0, 0);
    radius = 0;
}

MoleculeRenderData::MoleculeRenderData()
{
    clear();
}

void MoleculeRenderData::clear()
{
    atoms.clear();
    bonds.clear();
    bondends.clear();
    brackets.clear();
    rSiteAttachmentIndices.clear();
    attachmentPoints.clear();
    graphitems.clear();
    rings.clear();
    textitems.clear();
    aam.clear();
    reactingCenters.clear();
    inversions.clear();
    exactChanges.clear();
    sgroups.clear();
}

CP_DEF(RenderSettings);

RenderSettings::RenderSettings()
    : CP_INIT, TL_CP_GET(bondDashAromatic), TL_CP_GET(bondDashAny), TL_CP_GET(bondDashSingleOrAromatic), TL_CP_GET(bondDashDoubleOrAromatic),
      TL_CP_GET(bondDashHydro)
{
    init(1.0f, 1.0f);
}

void RenderSettings::init(float sf, float lwf)
{
    unit = sf / 30;
    bondLineWidth = lwf * unit;
    bondSpace = 2.5f * unit;

    fzz[FONT_SIZE_LABEL] = unit * 12;
    fzz[FONT_SIZE_ATTR] = unit * 8;
    fzz[FONT_SIZE_RGROUP_LOGIC] = unit * 12;
    fzz[FONT_SIZE_RGROUP_LOGIC_INDEX] = unit * 8;
    fzz[FONT_SIZE_INDICES] = unit * 6;
    fzz[FONT_SIZE_ATTACHMENT_POINT_INDEX] = unit * 6;
    fzz[FONT_SIZE_RSITE_ATTACHMENT_INDEX] = unit * 6;
    fzz[FONT_SIZE_COMMENT] = 0; // not used, value taken from RenderOptions.commentFontFactor
    fzz[FONT_SIZE_TITLE] = 0;   // not used, value taken from RenderOptions.titleFontFactor
    fzz[FONT_SIZE_DATA_SGROUP] = unit * 8;

    upperIndexShift = -0.4f;
    lowerIndexShift = 0.4f;
    boundExtent = 1.3f * unit;
    labelInternalOffset = unit;
    stereoGroupLabelOffset = 2 * unit;
    radicalRightOffset = unit / 2;
    radicalRightVertShift = -0.2f;
    radicalTopOffset = 0.8f * unit;
    radicalTopDistDot = unit;
    radicalTopDistCap = unit / 2;
    dashUnit = unit;
    eps = 1e-4f;
    cosineTreshold = 0.98f;
    prolongAdjSinTreshold = 0.2f;
    stereoCareBoxSize = bondSpace * 3 + unit * 3;
    minBondLength = unit * 5;

    graphItemDotRadius = unit;
    graphItemCapSlope = 2;
    graphItemCapBase = 0.7f * unit;
    graphItemCapWidth = 1.2f * unit;
    graphItemDigitWidth = 4.5f * unit;
    graphItemDigitHeight = 6.5f * unit;
    graphItemSignLineWidth = 0.8f * unit;
    graphItemPlusEdge = (graphItemDigitWidth - graphItemSignLineWidth) / 2;

    const int dashDot[] = {5, 2, 1, 2};
    const int dash[] = {3, 2};

    bondDashSingleOrAromatic.clear();
    bondDashDoubleOrAromatic.clear();
    bondDashAny.clear();
    bondDashAromatic.clear();
    for (int i = 0; i < NELEM(dashDot); ++i)
    {
        double val = dashDot[i] * dashUnit;
        bondDashSingleOrAromatic.push(val);
        bondDashDoubleOrAromatic.push(val);
    }
    for (int i = 0; i < NELEM(dash); ++i)
    {
        double val = dash[i] * dashUnit;
        bondDashAny.push(val);
        bondDashAromatic.push(val);
    }

    bondDashHydro.push(dashUnit);

    layoutMarginHorizontal = 0.4f;
    layoutMarginVertical = 0.6f;
    plusSize = 0.5;
    metaLineWidth = 1.0 / 16;
    arrowLength = 3 * plusSize;
    arrowHeadWidth = plusSize / 2;
    arrowHeadSize = plusSize / 2;
    equalityInterval = plusSize / 2;
    rGroupIfThenInterval = unit * 4;
    neighboringLabelTolerance = 1.3f;
    minSin = 0.49f;
    neighboringAtomDistanceTresholdA = 0.8f;
    neighboringAtomDistanceTresholdB = 0.5f;
}

CanvasOptions::CanvasOptions()
{
    clear();
}

void CanvasOptions::clear()
{
    width = height = -1;
    maxWidth = maxHeight = -1;
    xOffset = yOffset = 0;
    bondLength = -1;
    gridMarginX = gridMarginY = 0;
    marginX = marginY = 0;
    commentOffset = 0;
    commentPos = COMMENT_POS_BOTTOM;
    commentAlign.clear();
    titleAlign.clear();
    titleOffset = 0;
    gridColumnNumber = 1;
    comment.clear();
    titleProp.clear();
    titleProp.appendString("^NAME", true);
}

//
// MultilineTextLayout
//

MultilineTextLayout::MultilineTextLayout()
{
    clear();
}

MultilineTextLayout::MultilineTextLayout(Alignment bbox, Alignment inbox) : bbox_alignment(bbox), inbox_alignment(inbox)
{
}

float MultilineTextLayout::getRelativeOffset(Alignment alignment)
{
    if (alignment == Center)
        return 0.5f;
    if (alignment == Left)
        return 0.0f;

    // alignment == Right
    return 1.0f;
}

float MultilineTextLayout::getBboxRelativeOffset() const
{
    return getRelativeOffset(bbox_alignment);
}

float MultilineTextLayout::getInboxRelativeOffset() const
{
    return getRelativeOffset(inbox_alignment);
}

void MultilineTextLayout::clear()
{
    bbox_alignment = Center;
    inbox_alignment = Center;
}

float MultilineTextLayout::getAnchorPoint(float area_x, float area_width, float text_width)
{
    float bbox_x = area_x + (area_width - text_width) * getBboxRelativeOffset();
    return bbox_x + text_width * getInboxRelativeOffset();
}
