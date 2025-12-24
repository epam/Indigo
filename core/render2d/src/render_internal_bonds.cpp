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

#include "render_internal.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "base_cpp/tree.h"
#include "molecule/molecule.h"
#include "molecule/molecule_sgroups.h"
#include "molecule/query_molecule.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction.h"
#include "render_context.h"
#include <regex>

#ifdef _WIN32
#pragma warning(push, 4)
#endif

using namespace indigo;

#define BOND_STEREO_BOLD 10001

void MoleculeRenderInternal::_drawBond(int b)
{
    BondDescr& bd = _bd(b);
    const BondEnd& be1 = _be(bd.be1);
    const BondEnd& be2 = _be(bd.be2);
    const AtomDesc& ad1 = _ad(be1.aid);
    const AtomDesc& ad2 = _ad(be2.aid);

    _cw.setLineWidth(_settings.bondLineWidth);

    _cw.setSingleSource(CWC_BASE);
    if (_edgeIsHighlighted(b))
        _cw.setHighlight();
    else if (ad1.hcolorSet || ad2.hcolorSet)
    {
        Vec3f color1, color2;
        _cw.getColorVec(color1, CWC_BASE);
        _cw.getColorVec(color2, CWC_BASE);
        if (ad1.hcolorSet)
            color1.copy(ad1.hcolor);
        if (ad2.hcolorSet)
            color2.copy(ad2.hcolor);
        if (color1.x == color2.x && color1.y == color2.y && color1.z == color2.z)
            _cw.setSingleSource(color1);
        else
            _cw.setGradientSource(color1, color2, ad1.pos, ad2.pos);
    }

    switch (bd.type)
    {
    case BOND_SINGLE:
        _bondSingle(bd, be1, be2);
        break;
    case BOND_DOUBLE:
        _bondDouble(bd, be1, be2);
        break;
    case BOND_TRIPLE:
        _bondTriple(bd, be1, be2);
        break;
    case BOND_AROMATIC:
        _bondAromatic(bd, be1, be2);
        break;
    case _BOND_HYDROGEN:
        _bondHydrogen(bd, be1, be2);
        break;
    case _BOND_COORDINATION:
        _bondCoordination(bd, be1, be2);
        break;

    default:
        switch (bd.queryType)
        {
        case _BOND_ANY:
            _bondAny(bd, be1, be2);
            break;
        case _BOND_SINGLE_OR_DOUBLE:
            _bondSingleOrDouble(bd, be1, be2);
            break;
        case _BOND_DOUBLE_OR_AROMATIC:
            _bondDoubleOrAromatic(bd, be1, be2);
            break;
        case _BOND_SINGLE_OR_AROMATIC:
            _bondSingleOrAromatic(bd, be1, be2);
            break;
        default:
            throw Error("Unknown type");
        }
    }

    _cw.resetHighlightThickness();

    if (bd.reactingCenter != RC_UNMARKED)
    {
        int rc = bd.reactingCenter;
        if (rc == RC_NOT_CENTER)
            _drawReactingCenter(bd, RC_NOT_CENTER);
        else
        {
            if (rc & RC_CENTER)
                _drawReactingCenter(bd, RC_CENTER);
            if ((rc & RC_UNCHANGED) && (_opt.showReactingCenterUnchanged || (rc & (~RC_UNCHANGED))))
                _drawReactingCenter(bd, RC_UNCHANGED);
            if (rc & RC_MADE_OR_BROKEN)
                _drawReactingCenter(bd, RC_MADE_OR_BROKEN);
            if (rc & RC_ORDER_CHANGED)
                _drawReactingCenter(bd, RC_ORDER_CHANGED);
        }
    }

    _cw.resetHighlight();
    _cw.clearPattern(); // destroy the linear gradient pattern if one was used

    if (bd.topology > 0)
        _drawTopology(bd);
}

void MoleculeRenderInternal::_drawTopology(BondDescr& bd)
{
    if (bd.topology < 0)
        return;
    bd.tiTopology = _pushTextItem(RenderItem::RIT_TOPOLOGY, CWC_BASE, false);
    TextItem& ti = _data.textitems[bd.tiTopology];
    ti.fontsize = FONT_SIZE_ATTR;
    if (bd.topology == TOPOLOGY_RING)
        bprintf(ti.text, "rng");
    else if (bd.topology == TOPOLOGY_CHAIN)
        bprintf(ti.text, "chn");
    else
        throw Error("Unknown topology value");

    _cw.setTextItemSize(ti);
    float shift = (fabs(bd.norm.x * ti.bbsz.x) + fabs(bd.norm.y * ti.bbsz.y)) / 2 + _settings.unit;

    if (bd.extP < bd.extN)
        shift = shift + bd.extP;
    else
        shift = -shift - bd.extN;
    Vec2f c;
    c.copy(bd.center);
    c.addScaled(bd.norm, shift);
    c.addScaled(ti.bbsz, -0.5f);
    ti.bbp.copy(c);
    _cw.drawTextItemText(ti, _idle);
}

void MoleculeRenderInternal::_drawReactingCenter(BondDescr& bd, int rc)
{
    const int rcNumPnts = 8;
    Vec2f p[rcNumPnts];
    for (int i = 0; i < rcNumPnts; ++i)
        p[i].copy(bd.center);
    float alongIntRc = _settings.unit,           // half interval along for RC_CENTER
        alongIntMadeBroken = 2 * _settings.unit, // half interval between along for RC_MADE_OR_BROKEN
        alongSz = 1.5f * _settings.bondSpace,    // half size along for RC_CENTER
        acrossInt = 1.5f * _settings.bondSpace,  // half interval across for RC_CENTER
        acrossSz = 3.0f * _settings.bondSpace,   // half size across for all
        tiltTan = 0.2f,                          // tangent of the tilt angle
        radius = _settings.bondSpace;            // radius of the circle for RC_UNCHANGED
    int numLines = 0;

    _cw.setLineWidth(_settings.unit);
    switch (rc)
    {
    case RC_NOT_CENTER: // X
        // across
        p[0].addScaled(bd.norm, acrossSz);
        p[1].addScaled(bd.norm, -acrossSz);
        p[2].copy(p[0]);
        p[3].copy(p[1]);
        p[0].addScaled(bd.dir, tiltTan * acrossSz);
        p[1].addScaled(bd.dir, -tiltTan * acrossSz);
        p[2].addScaled(bd.dir, -tiltTan * acrossSz);
        p[3].addScaled(bd.dir, tiltTan * acrossSz);
        numLines = 2;
        break;
    case RC_CENTER: // #
        // across
        p[0].addScaled(bd.norm, acrossSz);
        p[0].addScaled(bd.dir, tiltTan * acrossSz);
        p[1].addScaled(bd.norm, -acrossSz);
        p[1].addScaled(bd.dir, -tiltTan * acrossSz);
        p[2].copy(p[0]);
        p[3].copy(p[1]);
        p[0].addScaled(bd.dir, alongIntRc);
        p[1].addScaled(bd.dir, alongIntRc);
        p[2].addScaled(bd.dir, -alongIntRc);
        p[3].addScaled(bd.dir, -alongIntRc);

        // along
        p[4].addScaled(bd.dir, alongSz);
        p[5].addScaled(bd.dir, -alongSz);
        p[6].copy(p[4]);
        p[7].copy(p[5]);
        p[4].addScaled(bd.norm, acrossInt);
        p[5].addScaled(bd.norm, acrossInt);
        p[6].addScaled(bd.norm, -acrossInt);
        p[7].addScaled(bd.norm, -acrossInt);
        numLines = 4;
        break;
    case RC_UNCHANGED: // o
        _cw.fillCircle(bd.center, radius);
        break;
    case RC_MADE_OR_BROKEN:
        // across
        p[0].addScaled(bd.norm, acrossSz);
        p[1].addScaled(bd.norm, -acrossSz);
        p[2].copy(p[0]);
        p[3].copy(p[1]);
        p[0].addScaled(bd.dir, alongIntMadeBroken);
        p[1].addScaled(bd.dir, alongIntMadeBroken);
        p[2].addScaled(bd.dir, -alongIntMadeBroken);
        p[3].addScaled(bd.dir, -alongIntMadeBroken);
        numLines = 2;
        break;
    case RC_ORDER_CHANGED:
        // across
        p[0].addScaled(bd.norm, acrossSz);
        p[1].addScaled(bd.norm, -acrossSz);
        numLines = 1;
        break;
    case RC_TOTAL:
        break;
    }
    for (int i = 0; i < numLines; ++i)
        _cw.drawLine(p[2 * i], p[2 * i + 1]);
    if (rc == RC_UNCHANGED)
    {
        bd.extN = std::max(bd.extN, radius);
        bd.extP = std::max(bd.extP, radius);
    }
    else
    {
        bd.extN = std::max(bd.extN, acrossSz);
        bd.extP = std::max(bd.extP, acrossSz);
    }
}

double MoleculeRenderInternal::_getAdjustmentFactor(const int aid, const int anei, const double acos, const double asin, const double /*tgb*/, const double csb,
                                                    const double snb, const double len, const double w, double& csg, double& sng)
{
    csg = csb;
    sng = snb;
    bool adjustLeft = acos < 0.99 && acos > -0.99;
    if (!adjustLeft || _bd(_be(anei).bid).isShort)
        return -1;
    const BondDescr& nbd = _bd(_be(anei).bid);
    if (nbd.type == BOND_DOUBLE && nbd.centered)
    {
        if (asin <= 0)
            return -1;
        return (len * asin - _settings.bondSpace) / (snb * acos + csb * asin);
    }
    if ((_bd(_be(anei).bid).stereodir == BOND_UP && _bd(_be(anei).bid).end == aid) || _bd(_be(anei).bid).stereodir == BOND_STEREO_BOLD)
    {
        if (fabs(asin) < 0.01)
            return -1;
        double sna = sqrt((1 - acos) / 2);                       // sin(a/2)
        double csa = (asin > 0 ? 1 : -1) * sqrt((1 + acos) / 2); // cos(a/2)
        double gamma = w / sna;
        double x = sqrt(len * len + gamma * gamma - 2 * gamma * len * csa);
        sng = gamma * sna / x;
        csg = sqrt(1 - sng * sng);
        return x;
    }
    if (asin > 0.01)
        return len / (acos * snb / asin + csb);
    return -1;
}

void MoleculeRenderInternal::_adjustAngle(Vec2f& l, const BondEnd& be1, const BondEnd& be2, bool left)
{
    const Vec2f& p1 = _ad(be1.aid).pos;
    const Vec2f& p2 = _ad(be2.aid).pos;
    const double len = Vec2f::dist(p1, p2);
    double w = _settings.stereoBondSpace;
    double tgb = w / len;
    double csb = sqrt(1 / (1 + tgb * tgb));
    double snb = tgb * csb;
    double sng = 0, csg = 0;
    double ttr = left ? _getAdjustmentFactor(be2.aid, be2.rnei, be2.rcos, be2.rsin, tgb, csb, snb, len, w, csg, sng)
                      : _getAdjustmentFactor(be2.aid, be2.lnei, be2.lcos, be2.lsin, tgb, csb, snb, len, w, csg, sng);
    if (ttr < 0)
        return;
    l.diff(p2, p1);
    l.normalize();
    l.scale(_2FLOAT(ttr));
    l.rotateL(_2FLOAT(left ? sng : -sng), _2FLOAT(csg));
    l.add(p1);
}

void MoleculeRenderInternal::_bondBoldStereo(BondDescr& bd, const BondEnd& be1, const BondEnd& be2)
{
    Vec2f r0(be1.p), l0(be1.p), r1(be2.p), l1(be2.p);
    float w = _settings.stereoBondSpace;
    l0.addScaled(bd.norm, -w);
    r0.addScaled(bd.norm, w);
    l1.addScaled(bd.norm, -w);
    r1.addScaled(bd.norm, w);

    _adjustAngle(l1, be1, be2, true);
    _adjustAngle(r1, be1, be2, false);
    _adjustAngle(r0, be2, be1, true);
    _adjustAngle(l0, be2, be1, false);
    _cw.fillHex(be1.p, r0, r1, be2.p, l1, l0);
}

void MoleculeRenderInternal::_bondHydrogen(BondDescr& bd, const BondEnd& be1, const BondEnd& be2)
{
    _cw.setDash(_settings.bondDashHydro, Vec2f::dist(be1.p, be2.p));
    // double len = Vec2f::dist(be2.p, be1.p);
    Vec2f l(be2.p), r(be2.p);
    float w = _settings.bondSpace;
    l.addScaled(bd.norm, -w);
    r.addScaled(bd.norm, w);
    bd.extP = bd.extN = w;

    float lw = _cw.currentLineWidth();
    Vec2f r0(be1.p), l0(be1.p);
    l0.addScaled(bd.norm, -lw / 2);
    r0.addScaled(bd.norm, lw / 2);

    _cw.drawLine(be1.p, be2.p);
    _cw.resetDash();
}

void MoleculeRenderInternal::_bondCoordination(BondDescr& bd, const BondEnd& be1, const BondEnd& be2)
{
    double len = Vec2f::dist(be2.p, be1.p);
    Vec2f l(be2.p), r(be2.p);
    float w = _settings.bondSpace;
    l.addScaled(bd.norm, -w);
    r.addScaled(bd.norm, w);
    bd.extP = bd.extN = w;

    float lw = _cw.currentLineWidth();
    Vec2f r0(be1.p), l0(be1.p);
    l0.addScaled(bd.norm, -lw / 2);
    r0.addScaled(bd.norm, lw / 2);

    _cw.drawLine(be1.p, be2.p);

    double arrow_length = lw * 5;
    double shorten = len - arrow_length;
    Vec2f reduced = (be2.p - be1.p) * static_cast<float>((1 - shorten / len));
    Vec2f slope_right(-reduced.y * 0.25f, reduced.x * 0.25f);
    _cw.drawLine(be2.p, (be2.p - reduced) + slope_right);
    Vec2f slope_left(reduced.y * 0.25f, -reduced.x * 0.25f);
    _cw.drawLine(be2.p, (be2.p - reduced) + slope_left);
}

void MoleculeRenderInternal::_bondSingle(BondDescr& bd, const BondEnd& be1, const BondEnd& be2)
{
    double len = Vec2f::dist(be2.p, be1.p);

    if (bd.stereodir == BOND_STEREO_BOLD)
    {
        _bondBoldStereo(bd, be1, be2);
        return;
    }

    float lw = _cw.currentLineWidth();
    if (bd.stereodir == 0)
    {
        _cw.drawLine(be1.p, be2.p);
        bd.extP = bd.extN = lw / 2;
        return;
    }

    // stereo bonds
    Vec2f l(be2.p), r(be2.p);
    float w = _settings.stereoBondSpace;
    l.addScaled(bd.norm, -w);
    r.addScaled(bd.norm, w);
    bd.extP = bd.extN = w;

    Vec2f r0(be1.p), l0(be1.p);
    l0.addScaled(bd.norm, -lw / 2);
    r0.addScaled(bd.norm, lw / 2);

    if (bd.stereodir == BOND_UP)
    {
        if (_ad(be2.aid).showLabel == false && !bd.isShort)
        {
            _adjustAngle(l, be1, be2, true);
            _adjustAngle(r, be1, be2, false);
            _cw.fillPentagon(r0, r, be2.p, l, l0);
        }
        else
        {
            _cw.fillQuad(r0, r, l, l0);
        }
    }
    else if (bd.stereodir == BOND_DOWN)
    {
        int constexpr min_count = 4;
        auto count = len / (_settings.hashSpacing > 0 ? _settings.hashSpacing : (lw * 2));
        if (_settings.hashSpacing > 0 && count > min_count)
        {
            _cw.fillQuadStripesSpacing(r0, l0, r, l, _settings.hashSpacing);
        }
        else
        {
            int stripeCnt = std::max((int)count, min_count);
            _cw.fillQuadStripes(r0, l0, r, l, stripeCnt);
        }
    }
    else if (bd.stereodir == BOND_EITHER)
    {
        int stripeCnt = std::max((int)((len) / lw / 1.5), 5);
        _cw.drawTriangleZigzag(be1.p, r, l, stripeCnt);
    }
    else
        throw Error("Unknown single bond stereo type");
}

void MoleculeRenderInternal::_bondAny(BondDescr& bd, const BondEnd& be1, const BondEnd& be2)
{
    _cw.setDash(_settings.bondDashAny, Vec2f::dist(be1.p, be2.p));
    _cw.drawLine(be1.p, be2.p);
    _cw.resetDash();
    bd.extP = bd.extN = _settings.bondLineWidth / 2;
}

float MoleculeRenderInternal::_ctghalf(float cs)
{
    return sqrt(1 - cs * cs) / (1 - cs);
}

float MoleculeRenderInternal::_doubleBondShiftValue(const BondEnd& be, bool right, bool centered)
{
    const BondDescr& bd = _bd(_be(right ? be.rnei : be.lnei).bid);
    float si = right ? be.rsin : be.lsin, co = right ? be.rcos : be.lcos;
    if (centered && bd.type == BOND_SINGLE && bd.end == be.aid && bd.stereodir != 0)
    {
        float tga = si / co;
        Vec2f dd;
        dd.diff(_be(bd.be1).p, _be(bd.be2).p);
        float len = dd.length();
        float tgb = (_settings.bondSpace + _settings.bondLineWidth) / len;
        float tgab = (tga + tgb) / (1 - tga * tgb);
        return -(len * si - _settings.bondSpace) / tgab + len * co - _settings.bondLineWidth / 2;
    }
    else
        return co * _settings.bondSpace / si;
}

void MoleculeRenderInternal::_prepareDoubleBondCoords(Vec2f* coord, BondDescr& bd, const BondEnd& be1, const BondEnd& be2, bool allowCentered)
{
    Vec2f ns, ds;
    ns.scaled(bd.norm, 2 * _settings.bondSpace + (bd.stereodir == BOND_STEREO_BOLD ? 1 : 0) * _settings.bondLineWidth);

    if (!(bd.stereodir == BOND_STEREO_BOLD) && ((allowCentered && bd.centered) || bd.cistrans))
    {
        Vec2f p0, p1, q0, q1;
        ns.scale(0.5f);
        p0.sum(be1.p, ns);
        p1.sum(be2.p, ns);
        q0.diff(be1.p, ns);
        q1.diff(be2.p, ns);

        if (be1.prolong)
        {
            p0.addScaled(be1.dir, _doubleBondShiftValue(be1, true, bd.centered));
            q0.addScaled(be1.dir, _doubleBondShiftValue(be1, false, bd.centered));
        }
        if (be2.prolong)
        {
            p1.addScaled(be2.dir, _doubleBondShiftValue(be2, false, bd.centered));
            q1.addScaled(be2.dir, _doubleBondShiftValue(be2, true, bd.centered));
        }

        coord[0].copy(p0);
        coord[1].copy(p1);
        coord[2].copy(q0);
        coord[3].copy(q1);
        bd.extP = bd.extN = _settings.bondSpace + _settings.bondLineWidth / 2;
    }
    else
    {
        bd.extP = ns.length() + _settings.bondLineWidth / 2;
        bd.extN = _settings.bondLineWidth / 2;

        if (!bd.lineOnTheRight)
        {

            std::swap(bd.extP, bd.extN);
            ns.negate();
        }

        Vec2f p0, p1;
        p0.sum(be1.p, ns);
        p1.sum(be2.p, ns);

        float cs;
        if (!_ad(be1.aid).showLabel)
        {
            cs = bd.lineOnTheRight ? be1.rcos : be1.lcos;
            if (fabs(cs) < _settings.cosineTreshold)
                p0.addScaled(be1.dir, _settings.bondSpace * _ctghalf(cs) * 2);
        }

        if (!_ad(be2.aid).showLabel)
        {
            cs = bd.lineOnTheRight ? be2.lcos : be2.rcos;
            if (fabs(cs) < _settings.cosineTreshold)
                p1.addScaled(be2.dir, _settings.bondSpace * _ctghalf(cs) * 2);
        }

        coord[0].copy(be1.p);
        coord[1].copy(be2.p);
        coord[2].copy(p0);
        coord[3].copy(p1);
    }
}

void MoleculeRenderInternal::_drawStereoCareBox(BondDescr& bd, const BondEnd& be1, const BondEnd& /*be2*/)
{
    Vec2f ns;
    ns.scaled(bd.norm, _settings.bondSpace);
    if (!bd.lineOnTheRight)
        ns.negate();
    if (bd.stereoCare)
    {
        Vec2f p0, p1, p2, p3;
        p0.lineCombin(be1.p, bd.dir, (bd.length - _settings.stereoCareBoxSize) / 2);
        p0.addScaled(bd.norm, -_settings.stereoCareBoxSize / 2);
        bd.extP = bd.extN = _settings.stereoCareBoxSize / 2 + _settings.unit / 2;
        if (!bd.centered)
        {
            float shift = Vec2f::dot(ns, bd.norm);
            bd.extP += shift;
            bd.extN -= shift;
            p0.add(ns);
        }
        p1.lineCombin(p0, bd.dir, _settings.stereoCareBoxSize);
        p2.lineCombin(p1, bd.norm, _settings.stereoCareBoxSize);
        p3.lineCombin(p0, bd.norm, _settings.stereoCareBoxSize);

        _cw.setLineWidth(_settings.unit);
        _cw.drawQuad(p0, p1, p2, p3);
    }
}

void MoleculeRenderInternal::_bondDouble(BondDescr& bd, const BondEnd& be1, const BondEnd& be2)
{
    Vec2f coord[4];
    _prepareDoubleBondCoords(coord, bd, be1, be2, true);
    if (bd.stereodir == BOND_STEREO_BOLD)
    {
        _bondBoldStereo(bd, be1, be2);
        _cw.drawLine(coord[2], coord[3]);
    }
    else if (bd.cistrans)
    {
        _cw.drawLine(coord[0], coord[3]);
        _cw.drawLine(coord[2], coord[1]);
    }
    else
    {
        _cw.drawLine(coord[0], coord[1]);
        _cw.drawLine(coord[2], coord[3]);
    }

    _drawStereoCareBox(bd, be1, be2);
}

void MoleculeRenderInternal::_bondSingleOrAromatic(BondDescr& bd, const BondEnd& be1, const BondEnd& be2)
{
    Vec2f coord[4];
    _prepareDoubleBondCoords(coord, bd, be1, be2, true);
    _cw.drawLine(coord[0], coord[1]);
    _cw.setDash(_settings.bondDashSingleOrAromatic);
    _cw.drawLine(coord[2], coord[3]);
    _cw.resetDash();

    _drawStereoCareBox(bd, be1, be2);
}

void MoleculeRenderInternal::_bondDoubleOrAromatic(BondDescr& bd, const BondEnd& be1, const BondEnd& be2)
{
    Vec2f coord[4];
    _prepareDoubleBondCoords(coord, bd, be1, be2, true);
    _cw.setDash(_settings.bondDashDoubleOrAromatic);
    _cw.drawLine(coord[0], coord[1]);
    _cw.drawLine(coord[2], coord[3]);
    _cw.resetDash();

    _drawStereoCareBox(bd, be1, be2);
}

void MoleculeRenderInternal::_bondSingleOrDouble(BondDescr& bd, const BondEnd& be1, const BondEnd& be2)
{
    Vec2f ns, ds;
    ns.scaled(bd.norm, 2 * _settings.bondSpace);

    ds.diff(be2.p, be1.p);
    float len = ds.length();
    ds.normalize();

    // Get number of segments of single-or-double bond
    // An average bond in our coordinates has length 1. We want an average bond to have 5 segments, like -=-=-
    // For longer bond more segments may be necessary, for shorter one - less, but not less then 3 segments, like -=-
    int numSegments = std::max((int)(len / 0.4f), 1) * 2 + 1;

    Vec2f r0, r1, p0, p1, q0, q1;
    float step = len / numSegments;
    ns.scale(0.5f);
    for (int i = 0; i < numSegments; ++i)
    {
        r0.lineCombin(be1.p, ds, i * step);
        r1.lineCombin(be1.p, ds, (i + 1) * step);
        if (i & 1)
        {
            p0.sum(r0, ns);
            p1.sum(r1, ns);
            q0.diff(r0, ns);
            q1.diff(r1, ns);
            _cw.drawLine(p0, p1);
            _cw.drawLine(q0, q1);
        }
        else
            _cw.drawLine(r0, r1);
    }
}

void MoleculeRenderInternal::_bondAromatic(BondDescr& bd, const BondEnd& be1, const BondEnd& be2)
{

    if (bd.aromRing)
    {
        // bond is in a ring, draw only a single line
        _cw.drawLine(be1.p, be2.p);
        bd.extP = bd.extN = _settings.bondLineWidth / 2;
    }
    else
    {
        Vec2f coord[4];
        _prepareDoubleBondCoords(coord, bd, be1, be2, false);

        _cw.drawLine(coord[0], coord[1]);
        _cw.setDash(_settings.bondDashAromatic);
        _cw.drawLine(coord[2], coord[3]);
        _cw.resetDash();
    }
}

void MoleculeRenderInternal::_bondTriple(BondDescr& bd, const BondEnd& be1, const BondEnd& be2)
{
    Vec2f ns;
    ns.scaled(bd.norm, _settings.bondSpace * 2);

    Vec2f vr1(be1.p), vr2(be2.p), vl1(be1.p), vl2(be2.p);
    vr1.add(ns);
    vr2.add(ns);
    vl1.sub(ns);
    vl2.sub(ns);

    _cw.drawLine(be1.p, be2.p);
    _cw.drawLine(vr1, vr2);
    _cw.drawLine(vl1, vl2);

    bd.extP = bd.extN = _settings.bondSpace * 2 + _settings.bondLineWidth / 2;
}

void MoleculeRenderInternal::_precalcScale()
{
    // Check structure for long atom labels (pseudoatoms, SMARTS) and change scale to fix
    // the issue with labels overlapping each other
    long long int max_output_length = 4;
    BaseMolecule& bm = *_mol;
    Array<long long int> output_lengths;
    int max_index = -1;
    output_lengths.resize(_mol->vertexEnd());
    for (int i = _mol->vertexBegin(); i < _mol->vertexEnd(); i = _mol->vertexNext(i))
    {
        long long int output_length = 0;
        Array<int> iarr;
        Array<char> carr;
        if (bm.isPseudoAtom(i))
        {
            carr.readString(bm.getPseudoAtom(i), true);
            output_length = carr.size();
        }
        else if (bm.isTemplateAtom(i))
        {
            carr.readString(bm.getTemplateAtom(i), true);
            output_length = carr.size();
        }
        else if (bm.isRSite(i))
        {
            output_length = 0;
            QS_DEF(Array<int>, rg);
            bm.getAllowedRGroups(i, rg);
            if (rg.size() == 0)
            {
                output_length += 1;
            }
            else
            {
                for (int j = 0; j < rg.size(); ++j)
                {
                    if (j > 0)
                    {
                        output_length += 1;
                    }
                    output_length += 2;
                }
            }
        }
        else if (_mol->isQueryMolecule())
        {
            QueryMolecule& qmol = _mol->asQueryMolecule();
            int queryLabel = QueryMolecule::parseQueryAtom(qmol, i, iarr);
            if (queryLabel < 0)
            {
                bm.getAtomDescription(i, carr);
                output_length = carr.size();
            }
            else if (!QueryMolecule::queryAtomIsRegular(qmol, i))
            {
                output_length = 1;
                for (int j = 0; j < iarr.size(); ++j)
                {
                    if (j > 0)
                    {
                        output_length += 1;
                    }
                    output_length += strlen(Element::toString(iarr[j]));
                }
                output_length += 1;
            }
            else
            {
                output_length = strlen(Element::toString(bm.getAtomNumber(i)));
            }
        }
        else
        {
            output_length = strlen(Element::toString(bm.getAtomNumber(i)));
        }
        output_lengths[i] = output_length;
        if (max_output_length < output_lengths[i])
        {
            max_output_length = output_lengths[i];
            max_index = i;
        }
    }
    float scale_modificator = 1.0;
    if (max_index >= 0)
    {
        const Vertex& max_length_vertex = bm.getVertex(max_index);
        for (auto nei = max_length_vertex.neiBegin(); nei != max_length_vertex.neiEnd(); nei = max_length_vertex.neiNext(nei))
        {
            if (max_output_length - output_lengths[max_length_vertex.neiVertex(nei)] > 10)
            {
                scale_modificator = 2.0;
                break;
            }
        }
    }
    _scale = std::max(_scale, float(max_output_length) / ((float)10.0 * scale_modificator));
}

#ifdef _WIN32
#pragma warning(pop)
#endif
