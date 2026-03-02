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

static bool _isBondWide(const BondDescr& bd)
{
    return bd.type == BOND_DOUBLE || bd.type == BOND_TRIPLE || bd.queryType == _BOND_DOUBLE_OR_AROMATIC || bd.queryType == _BOND_SINGLE_OR_AROMATIC ||
           _BOND_SINGLE_OR_DOUBLE;
}

RenderOptions::RenderOptions()
{
    clearRenderOptions();
}

void RenderOptions::clearRenderOptions()
{
    baseColor.set(0, 0, 0);
    backgroundColor.set(-1, -1, -1);
    highlightThicknessEnable = false;
    highlightThicknessFactor = 1.8f;
    highlightColorEnable = true;
    highlightColor.set(1, 0, 0);
    aamColor.set(0, 0, 0);
    commentFontFactor = 20;
    commentSpacing = 0.5;
    titleFontFactor = 20;
    titleSpacing = 0.5;
    labelMode = LABEL_MODE_TERMINAL_HETERO;
    highlightedLabelsVisible = false;
    boldBondDetection = true;
    implHVisible = true;
    commentColor.set(0, 0, 0);
    titleColor.set(0, 0, 0);
    dataGroupColor.set(0, 0, 0);
    mode = MODE_NONE;
    hdc = 0;
    output = NULL;
    showAtomIds = false;
    showBondIds = false;
    atomBondIdsFromOne = false;
    showBondEndIds = false;
    showNeighborArcs = false;
    showValences = true;
    showCIPLabels = true;
    atomColoring = false;
    stereoMode = STEREO_STYLE_OLD;
    showReactingCenterUnchanged = false;
    centerDoubleBondWhenStereoAdjacent = false;
    showCycles = false;
    agentsBelowArrow = true;
    atomColorProp.clear();
    ppi = LayoutOptions::DEFAULT_PPI;
    fontSize = -1;
    fontSizeUnit = UnitsOfMeasure::PT;
    fontSizeSub = -1;
    fontSizeSubUnit = UnitsOfMeasure::PT;
    bondThickness = -1;
    bondThicknessUnit = UnitsOfMeasure::PT;
    bondSpacing = -1;
    stereoBondWidth = -1;
    stereoBondWidthUnit = UnitsOfMeasure::PT;
    hashSpacing = -1;
    hashSpacingUnit = UnitsOfMeasure::PT;
}

AcsOptions::AcsOptions()
{
    clear();
}

void AcsOptions::clear()
{
    bondSpacing = -1;
    fontSizeAngstrom = -1;
    fontSizeSubAngstrom = -1;
    bondThicknessAngstrom = -1;
    stereoBondWidthAngstrom = -1;
    hashSpacingAngstrom = -1;
}

IMPL_ERROR(MoleculeRenderInternal, "molecule render internal");

CP_DEF(MoleculeRenderInternal);

MoleculeRenderInternal::MoleculeRenderInternal(const RenderOptions& opt, const RenderSettings& settings, RenderContext& cw, bool idle)
    : _mol(NULL), _cw(cw), _settings(settings), _opt(opt), CP_INIT, TL_CP_GET(_data), TL_CP_GET(_atomMapping), TL_CP_GET(_atomMappingInv),
      TL_CP_GET(_bondMappingInv), isRFragment(false), _idle(idle)
{
    _data.clear();
    _atomMapping.clear();
    _atomMappingInv.clear();
    _bondMappingInv.clear();
}

MoleculeRenderInternal::~MoleculeRenderInternal()
{
    if (_own_mol)
    {
        delete _mol;
    }
}

void MoleculeRenderInternal::setMolecule(BaseMolecule* mol)
{
    _mol = mol;
    _data.clear();
    _atomMapping.clear();

    bool superatoms = _mol->sgroups.getSGroupCount(SGroup::SG_TYPE_SUP) > 0;
    bool mulsgroups = _mol->sgroups.getSGroupCount(SGroup::SG_TYPE_MUL) > 0;
    auto isThereAtLeastOneContracted = false;
    if (superatoms || mulsgroups)
    {
        // auto count = _mol->sgroups.getSGroupCount(SGroup::SG_TYPE_SUP);
        BaseMolecule& bmol = *_mol;
        for (int i = bmol.sgroups.begin(); i != bmol.sgroups.end(); i = bmol.sgroups.next(i))
        {
            SGroup& sgroup = bmol.sgroups.getSGroup(i);
            if (sgroup.contracted == DisplayOption::Contracted || sgroup.contracted == DisplayOption::Undefined)
            {
                isThereAtLeastOneContracted = true;
                break;
            }
        }
    }
    if (mulsgroups || isThereAtLeastOneContracted && superatoms)
    {
        _prepareSGroups(isThereAtLeastOneContracted);
    }

    _data.atoms.clear();
    _data.atoms.resize(_mol->vertexEnd());
    for (auto i = _mol->vertexBegin(); i != _mol->vertexEnd(); i = _mol->vertexNext(i))
    {
        _ad(i).clear();
    }

    _data.bonds.clear();
    _data.bonds.resize(_mol->edgeEnd());
    for (auto i = _mol->edgeBegin(); i != _mol->edgeEnd(); i = _mol->edgeNext(i))
    {
        _bd(i).clear();
    }
}

void MoleculeRenderInternal::setIsRFragment(bool new_isRFragment)
{
    this->isRFragment = new_isRFragment;
}

void MoleculeRenderInternal::setScaleFactor(const float scaleFactor, const Vec2f& min, const Vec2f& max)
{
    _scale = scaleFactor;
    _min.copy(min);
    _max.copy(max);
}

float MoleculeRenderInternal::getScaleFactor(Vec2f& min, Vec2f& max)
{
    min.copy(_min);
    max.copy(_max);
    return _scale;
}

void mapArray(Array<int>& dst, const Array<int>& src, const int* mapping)
{
    for (int i = 0; i < src.size(); ++i)
    {
        int j = mapping == NULL ? i : mapping[i];
        dst[j] = src[i];
    }
}

void MoleculeRenderInternal::setReactionComponentProperties(const Array<int>* aam, const Array<int>* reactingCenters, const Array<int>* inversions)
{
    if (aam != NULL)
        _data.aam.copy(*aam);
    if (reactingCenters != NULL)
        _data.reactingCenters.copy(*reactingCenters);
    if (inversions != NULL)
        _data.inversions.copy(*inversions);
}

void MoleculeRenderInternal::setQueryReactionComponentProperties(const Array<int>* exactChanges)
{
    if (exactChanges != NULL)
        _data.exactChanges.copy(*exactChanges);
}

void MoleculeRenderInternal::render()
{
    _precalcScale();

    _initCoordinates();

    _initBondData();

    _initBondEndData();

    _findNeighbors();

    _initBoldStereoBonds();

    _findRings();

    _determineDoubleBondShift();

    _determineStereoGroupsMode();

    _initAtomData();

    _initRGroups();

    _findCenteredCase();

    _prepareLabels();

    _initSGroups();

    _extendRenderItems();

    _findAnglesOverPi();

    _calculateBondOffset();

    _applyBondOffset();

    _setBondCenter();

    _renderBonds();

    _renderRings();

    _renderSGroups();

    _renderLabels();

    _renderBondIds();

    _renderAtomIds();

    _renderEmptyRFragment();
}

BondEnd& MoleculeRenderInternal::_be(int beid)
{
    return _data.bondends[beid];
}

const BondEnd& MoleculeRenderInternal::_be(int beid) const
{
    return _data.bondends[beid];
}

BondDescr& MoleculeRenderInternal::_bd(int bid)
{
    return _data.bonds[bid];
}

const BondDescr& MoleculeRenderInternal::_bd(int bid) const
{
    return _data.bonds[bid];
}

AtomDesc& MoleculeRenderInternal::_ad(int aid)
{
    return _data.atoms[aid];
}

const AtomDesc& MoleculeRenderInternal::_ad(int aid) const
{
    return _data.atoms[aid];
}

int MoleculeRenderInternal::_getOpposite(int beid) const
{
    int bid = _be(beid).bid;
    const BondDescr& bd = _bd(bid);
    if (bd.be1 == beid)
        return bd.be2;
    if (bd.be2 == beid)
        return bd.be1;
    throw Error("The bond end given is not adjacent to this bond");
}

void MoleculeRenderInternal::_determineDoubleBondShift()
{
    // determine double bond shift direction, if any
    // (the bond may not be double or appear to be centered later on - so far we don't care)
    for (int i = _mol->edgeBegin(); i < _mol->edgeEnd(); i = _mol->edgeNext(i))
    {
        BondDescr& bd = _bd(i);
        const BondEnd& be1 = _be(bd.be1);
        const BondEnd& be2 = _be(bd.be2);

        if (bd.inRing)
            if (be1.lRing < 0)
                bd.lineOnTheRight = true;
            else if (be2.lRing < 0)
                bd.lineOnTheRight = false;
            else
            {
                const Ring& r1 = _data.rings[be1.lRing];
                const Ring& r2 = _data.rings[be2.lRing];
                // compare the ratios of double bonds in the two rings
                bd.lineOnTheRight = r1.dblBondCount * r2.bondEnds.size() < r2.dblBondCount * r1.bondEnds.size();
            }
        else
        {
            const BondEnd& bel1 = _be(be1.lnei);
            const BondEnd& ber1 = _be(be1.rnei);
            // right neighbor for the second bond end is on the left of the bond!
            const BondEnd& bel2 = _be(be2.rnei);
            const BondEnd& ber2 = _be(be2.lnei);

            // angles adjacent to the bond
            float al1 = be1.lang, ar1 = ber1.lang, al2 = bel2.lang, ar2 = be2.lang;
            int neiBalance = (al1 < M_PI ? 1 : 0) + (al2 < M_PI ? 1 : 0) + (ar1 < M_PI ? -1 : 0) + (ar2 < M_PI ? -1 : 0);
            if (neiBalance > 0)
                bd.lineOnTheRight = false;
            else if (neiBalance < 0)
                bd.lineOnTheRight = true;
            else
            {
                // compare the number of wide (double, triple, etc.) bonds on both sides
                int wideNeiBalance = (al1 < M_PI && _isBondWide(_bd(bel1.bid)) ? 1 : 0) + (al2 < M_PI && _isBondWide(_bd(bel2.bid)) ? 1 : 0) +
                                     (ar1 < M_PI && _isBondWide(_bd(ber1.bid)) ? -1 : 0) + (ar2 < M_PI && _isBondWide(_bd(ber2.bid)) ? -1 : 0);
                if (wideNeiBalance > 0)
                    bd.lineOnTheRight = false;
                else if (wideNeiBalance < 0)
                    bd.lineOnTheRight = true;
                else
                {
                    // compare the number of wide (double, triple, etc.) bonds on both sides
                    int stereoBalance = (al1 < M_PI && _bd(bel1.bid).stereodir != 0 ? 1 : 0) + (al2 < M_PI && _bd(bel2.bid).stereodir != 0 ? 1 : 0) +
                                        (ar1 < M_PI && _bd(ber1.bid).stereodir != 0 ? -1 : 0) + (ar2 < M_PI && _bd(ber2.bid).stereodir != 0 ? -1 : 0);
                    if (stereoBalance > 0)
                        bd.lineOnTheRight = true;
                    else if (stereoBalance < 0)
                        bd.lineOnTheRight = false;
                    else
                        bd.lineOnTheRight = (al1 + al2 < ar1 + ar2);
                }
            }
        }
    }
}

void MoleculeRenderInternal::_extendRenderItem(RenderItem& item, const float extent)
{
    Vec2f exv(extent, extent);
    item.bbsz.addScaled(exv, 2);
    item.bbp.sub(exv);
    item.relpos.add(exv);
}

#define __minmax(inv, t1, t2) (inv ? std::min(t1, t2) : std::max(t1, t2))
bool MoleculeRenderInternal::_clipRaySegment(float& offset, const Vec2f& p, const Vec2f& d, const Vec2f& n0, const Vec2f& a, const Vec2f& b, const float w)
{
    Vec2f ab, pa, pb;
    ab.diff(b, a);
    ab.normalize();
    Vec2f n(n0);
    float dot = Vec2f::dot(ab, n);
    if (fabs(dot) < 1e-4)
        return 0;
    if (dot < 0)
        n.negate();
    pa.diff(a, p);
    pb.diff(b, p);
    float ta = Vec2f::dot(pa, n);
    float tb = Vec2f::dot(pb, n);
    float tl = -w / 2;
    float tr = w / 2;
    float t = 0;
    bool f = false;
    bool inv = Vec2f::dot(ab, d) < 0;
    if (ta < tl + 1e-8 && tl < tb + 1e-8)
    {
        t = f ? __minmax(inv, t, tl) : tl;
        f = true;
    }
    if (ta < tr + 1e-8 && tr < tb + 1e-8)
    {
        t = f ? __minmax(inv, t, tr) : tr;
        f = true;
    }
    if (tl < ta + 1e-8 && ta < tr + 1e-8)
    {
        t = f ? __minmax(inv, t, ta) : ta;
        f = true;
    }
    if (tl < tb + 1e-8 && tb < tr + 1e-8)
    {
        t = f ? __minmax(inv, t, tb) : tb;
        f = true;
    }

    if (!f)
        return false;
    pa.addScaled(ab, (t - ta) / fabs(dot));
    offset = Vec2f::dot(d, pa);
    return true;
}

bool MoleculeRenderInternal::_clipRayBox(float& offset, const Vec2f& p, const Vec2f& d, const Vec2f& rp, const Vec2f& sz, const float w)
{
    Vec2f n(-d.y, d.x);
    Vec2f a, b;
    bool f = false;
    float t = 0, tt = 0;

    a.set(rp.x, rp.y);
    b.set(rp.x + sz.x, rp.y);
    if (_clipRaySegment(tt, p, d, n, a, b, w))
    {
        f = true;
        t = std::max(t, tt);
    }

    a.set(rp.x, rp.y);
    b.set(rp.x, rp.y + sz.y);
    if (_clipRaySegment(tt, p, d, n, a, b, w))
    {
        f = true;
        t = std::max(t, tt);
    }

    a.set(rp.x + sz.x, rp.y);
    b.set(rp.x + sz.x, rp.y + sz.y);
    if (_clipRaySegment(tt, p, d, n, a, b, w))
    {
        f = true;
        t = std::max(t, tt);
    }

    a.set(rp.x, rp.y + sz.y);
    b.set(rp.x + sz.x, rp.y + sz.y);
    if (_clipRaySegment(tt, p, d, n, a, b, w))
    {
        f = true;
        t = std::max(t, tt);
    }

    if (f)
        offset = t;
    return f;
}

const char* MoleculeRenderInternal::_getStereoGroupText(int type)
{
    switch (type)
    {
    case MoleculeStereocenters::ATOM_ABS:
        return "abs";
    case MoleculeStereocenters::ATOM_AND:
        return "and";
    case MoleculeStereocenters::ATOM_OR:
        return "or";
    case MoleculeStereocenters::ATOM_ANY:
        return "any";
    default:
        throw Error("Unknown stereocenter type");
    }
}

int MoleculeRenderInternal::_parseColorString(Scanner& scanner, float& r, float& g, float& b)
{
    if (!scanner.tryReadFloat(r))
        return -1;
    scanner.skipSpace();
    if (scanner.isEOF())
        return -1;
    if (scanner.readChar() != ',')
        return -1;
    scanner.skipSpace();
    if (!scanner.tryReadFloat(g))
        return -1;
    scanner.skipSpace();
    if (scanner.isEOF())
        return -1;
    if (scanner.readChar() != ',')
        return -1;
    scanner.skipSpace();
    if (!scanner.tryReadFloat(b))
        return -1;
    return 1;
}

void MoleculeRenderInternal::_initRGroups()
{
    if (_mol->attachmentPointCount() > 0)
    {
        for (int i = 1; i <= _mol->attachmentPointCount(); ++i)
            for (int j = 0, k; (k = _mol->getAttachmentPoint(i, j)) >= 0; ++j)
                _ad(k).isRGroupAttachmentPoint = true;
    }
}

void MoleculeRenderInternal::_initSGroups(Tree& sgroups, Rect2f parent)
{
    BaseMolecule& mol = *_mol;

    if (sgroups.label != -1)
    {
        SGroup& sgroup = mol.sgroups.getSGroup(sgroups.label);

        const Rect2f bound = _bound(sgroup.atoms);

        if (sgroup.sgroup_type == SGroup::SG_TYPE_DAT)
        {
            const DataSGroup& group = (DataSGroup&)sgroup;
            const char* atomColorProp = _opt.atomColorProp.size() > 0 ? _opt.atomColorProp.ptr() : NULL;
            if (atomColorProp != NULL && strcmp(atomColorProp, group.name.ptr()) == 0)
            {
                Vec3f color;
                BufferScanner scanner(group.data);
                if (_parseColorString(scanner, color.x, color.y, color.z) < 0)
                    throw Error("Color value format invalid");
                for (int j = 0; j < group.atoms.size(); ++j)
                {
                    AtomDesc& ad = _ad(group.atoms[j]);
                    if (ad.hcolorSet)
                        throw Error("An atom belongs to more then one color group");
                    ad.hcolor.copy(color);
                    ad.hcolorSet = true;
                }
                return;
            }
            Sgroup& sg = _data.sgroups.push();
            int tii = _pushTextItem(sg, RenderItem::RIT_DATASGROUP);
            TextItem& ti = _data.textitems[tii];
            if (group.tag != ' ')
            {
                ti.text.push(group.tag);
                ti.text.appendString(" = ", false);
            }

            if (group.data.size())
            {
                Array<char> data;
                data.copy(group.data);
                if (data[0] == '\\')
                    data.remove(0);
                ti.text.concat(data);
            }

            ti.text.push(0);
            ti.fontsize = FONT_SIZE_DATA_SGROUP;
            _cw.setTextItemSize(ti);

            if (!group.detached)
            {
                if (group.atoms.size() > 0)
                {
                    const AtomDesc& ad = _ad(group.atoms[0]);
                    ti.bbp.copy(_ad(group.atoms[0]).pos);
                    ti.bbp.x += ad.boundBoxMax.x + _settings.unit * 2;
                    ti.bbp.y -= ti.bbsz.y / 2;
                }
            }
            else if (group.relative)
            {
                _objDistTransform(ti.bbp, group.display_pos);
                if (group.atoms.size() > 0)
                {
                    ti.bbp.add(_ad(group.atoms[0]).pos);
                }
                else
                {
                    ti.bbp.add(parent.rightTop());
                }
            }
            else
            {
                _objCoordTransform(ti.bbp, group.display_pos);
            }

            parent = ILLEGAL_RECT();
        }

        if (sgroup.sgroup_type == SGroup::SG_TYPE_SRU)
        {
            const RepeatingUnit& group = (RepeatingUnit&)sgroup;
            Sgroup& sg = _data.sgroups.push();
            _loadBracketsAuto(group, sg);
            int tiIndex = _pushTextItem(sg, RenderItem::RIT_SGROUP);
            TextItem& index = _data.textitems[tiIndex];
            index.fontsize = FONT_SIZE_ATTR;
            bprintf(index.text, group.subscript.size() > 0 ? group.subscript.ptr() : "n");
            _positionIndex(sg, tiIndex, true);
            if (group.connectivity != RepeatingUnit::HEAD_TO_TAIL)
            {
                int tiConn = _pushTextItem(sg, RenderItem::RIT_SGROUP);
                TextItem& conn = _data.textitems[tiConn];
                conn.fontsize = FONT_SIZE_ATTR;
                if (group.connectivity == RepeatingUnit::HEAD_TO_HEAD)
                {
                    bprintf(conn.text, "hh");
                }
                else
                {
                    bprintf(conn.text, "eu");
                }
                _positionIndex(sg, tiConn, false);
            }
            parent = bound;
        }

        if (sgroup.sgroup_type == SGroup::SG_TYPE_GEN)
        {
            Sgroup& sg = _data.sgroups.push();
            _loadBracketsAuto(sgroup, sg);
            parent = ILLEGAL_RECT();
        }

        if (sgroup.sgroup_type == SGroup::SG_TYPE_MUL)
        {
            const MultipleGroup& group = (MultipleGroup&)sgroup;
            Sgroup& sg = _data.sgroups.push();
            _loadBracketsAuto(group, sg);
            int tiIndex = _pushTextItem(sg, RenderItem::RIT_SGROUP);
            TextItem& index = _data.textitems[tiIndex];
            index.fontsize = FONT_SIZE_ATTR;
            bprintf(index.text, "%d", group.multiplier);
            _positionIndex(sg, tiIndex, true);
            parent = ILLEGAL_RECT();
        }

        if (sgroup.sgroup_type == SGroup::SG_TYPE_SUP)
        {
            const Superatom& group = (Superatom&)sgroup;
            Sgroup& sg = _data.sgroups.push();
            QS_DEF(Array<Vec2f[2]>, brackets);
            _placeBrackets(sg, group.atoms, brackets);
            _loadBrackets(sg, brackets);

            if (group.subscript.size() == 0 || std::string(group.subscript.ptr()).empty())
                sg.hide_brackets = true;
            int tiIndex = _pushTextItem(sg, RenderItem::RIT_SGROUP);
            TextItem& index = _data.textitems[tiIndex];
            index.fontsize = FONT_SIZE_ATTR;
            bprintf(index.text, "%s", group.subscript.ptr());
            _positionIndex(sg, tiIndex, true);

            parent = ILLEGAL_RECT();
        }
    }

    ObjArray<Tree>& children = sgroups.children();
    for (int i = 0; i < children.size(); i++)
    {
        _initSGroups(children[i], parent);
    }
}

void MoleculeRenderInternal::_initSGroups()
{
    BaseMolecule& mol = *_mol;

    Tree sgroups;
    mol.sgroups.buildTree(sgroups);
    _initSGroups(sgroups, Rect2f(_min, _max));
}

void MoleculeRenderInternal::_loadBrackets(Sgroup& sg, const Array<Vec2f[2]>& coord)
{
    for (int j = 0; j < coord.size(); ++j)
    {
        int bracketId = _data.brackets.size();
        if (j == 0)
        {
            sg.bibegin = bracketId;
            sg.bicount = 1;
        }
        else
        {
            sg.bicount++;
        }
        RenderItemBracket& bracket = _data.brackets.push();
        bracket.p0.copy(coord[j][0]);
        bracket.p1.copy(coord[j][1]);
        bracket.d.diff(bracket.p1, bracket.p0);
        bracket.length = bracket.d.length();
        bracket.d.normalize();
        bracket.n.copy(bracket.d);
        bracket.n.rotateL(-1, 0);
        bracket.width = bracket.length * 0.15f;
        bracket.q0.lineCombin(bracket.p0, bracket.n, bracket.width);
        bracket.q1.lineCombin(bracket.p1, bracket.n, bracket.width);
        bracket.invertUpperLowerIndex = bracket.n.x > 0;
    }
}

void MoleculeRenderInternal::_convertCoordinate(const Array<Vec2f[2]>& original, Array<Vec2f[2]>& converted)
{
    auto& left = original.at(0);
    auto& right = original.at(1);
    auto& adjLeft = converted.push();
    auto& adjRight = converted.push();

    _objCoordTransform(adjLeft[0], left[0]);
    _objCoordTransform(adjLeft[1], left[1]);
    _objCoordTransform(adjRight[0], right[0]);
    _objCoordTransform(adjRight[1], right[1]);
}

void MoleculeRenderInternal::_loadBracketsAuto(const SGroup& group, Sgroup& sg)
{
    Array<Vec2f[2]> brackets;
    _placeBrackets(sg, group.atoms, brackets);

    const bool isBracketsCoordinates = group.brackets.size() != 0 && Vec2f::distSqr(group.brackets.at(0)[0], group.brackets.at(0)[1]) > EPSILON;
    if (isBracketsCoordinates)
    {
        Array<Vec2f[2]> temp;
        _convertCoordinate(group.brackets, temp);
        _loadBrackets(sg, temp);
        return;
    }

    _loadBrackets(sg, brackets);
}

void MoleculeRenderInternal::_positionIndex(Sgroup& sg, int ti, bool lower)
{
    RenderItemBracket& bracket = _data.brackets[sg.bibegin + sg.bicount - 1];
    TextItem& index = _data.textitems[ti];
    if (bracket.invertUpperLowerIndex)
        lower = !lower;
    _cw.setTextItemSize(index, lower ? bracket.p1 : bracket.p0);
    float xShift = (fabs(index.bbsz.x * bracket.n.x) + fabs(index.bbsz.y * bracket.n.y)) / 2 + _settings.unit;
    float yShift = (fabs(index.bbsz.x * bracket.d.x) + fabs(index.bbsz.y * bracket.d.y)) / 2;
    index.bbp.addScaled(bracket.n, -xShift);
    index.bbp.addScaled(bracket.d, lower ? -yShift : yShift);
}

void MoleculeRenderInternal::_placeBrackets(Sgroup& /*sg*/, const Array<int>& atoms, Array<Vec2f[2]>& brackets)
{
    auto left = brackets.push();
    auto right = brackets.push();

    Vec2f min, max, a, b;
    for (int i = 0; i < atoms.size(); ++i)
    {
        int aid = atoms[i];
        const AtomDesc& ad = _ad(aid);
        a.sum(ad.pos, ad.boundBoxMin);
        b.sum(ad.pos, ad.boundBoxMax);
        if (i == 0)
        {
            min.copy(a);
            max.copy(b);
        }
        else
        {
            min.min(a);
            max.max(b);
        }
    }
    float extent = _settings.unit * 3;
    min.sub(Vec2f(extent, extent));
    max.add(Vec2f(extent, extent));
    left[0].set(min.x, max.y);
    left[1].set(min.x, min.y);
    right[0].set(max.x, min.y);
    right[1].set(max.x, max.y);
}

void MoleculeRenderInternal::_cloneAndFillMappings()
{
    BaseMolecule* clone = _mol->neu();
    clone->clone(*_mol, &_atomMapping, &_atomMappingInv);

    _bondMappingInv.clear();
    for (int i = clone->edgeBegin(); i < clone->edgeEnd(); i = clone->edgeNext(i))
    {
        _bondMappingInv.emplace(i, BaseMolecule::findMappedEdge(*clone, *_mol, i, _atomMapping.ptr()));
    }
    _mol = clone;
    _own_mol = true;
}

void MoleculeRenderInternal::_prepareSGroups(bool collapseAtLeastOneSuperatom)
{
    _cloneAndFillMappings();

    BaseMolecule& mol = *_mol;
    if (collapseAtLeastOneSuperatom)
    {
        for (int i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
        {
            SGroup& sgroup = mol.sgroups.getSGroup(i);
            if (sgroup.contracted == DisplayOption::Contracted || sgroup.contracted == DisplayOption::Undefined)
            {
                if (sgroup.sgroup_type == SGroup::SG_TYPE_SUP)
                {
                    const Superatom& group = (Superatom&)sgroup;
                    Vec3f displayPosition = group.display_position;
                    bool useDisplayPosition = false;
                    if (fabs(displayPosition.x) > EPSILON || fabs(displayPosition.y) > EPSILON || fabs(displayPosition.z) > EPSILON)
                    {
                        useDisplayPosition = true;
                    }

                    Vec3f centre;
                    if (!useDisplayPosition)
                    {
                        for (int j = 0; j < group.atoms.size(); ++j)
                        {
                            int atomID = group.atoms[j];
                            centre.add(mol.getAtomXyz(atomID));
                        }
                        centre.scale(1.0f / group.atoms.size());
                    }
                    int superAtomID = -1;

                    if (mol.isQueryMolecule())
                    {
                        superAtomID = mol.asQueryMolecule().addAtom(new QueryMolecule::Atom(QueryMolecule::ATOM_PSEUDO, group.subscript.ptr()));
                    }
                    else
                    {
                        Molecule& amol = mol.asMolecule();
                        superAtomID = amol.addAtom(ELEM_PSEUDO);
                        amol.setPseudoAtom(superAtomID, group.subscript.ptr());
                    }
                    QS_DEF(RedBlackSet<int>, groupAtoms);
                    groupAtoms.clear();
                    for (int j = 0; j < group.atoms.size(); ++j)
                    {
                        groupAtoms.insert(group.atoms[j]);
                    }
                    Vec3f pos;
                    int posCnt = 0;
                    while (mol.sgroups.hasSGroup(i) && group.atoms.size() > 0)
                    {
                        int atomID = group.atoms[0];
                        const Vertex& v = mol.getVertex(atomID);
                        for (int j = v.neiBegin(); j < v.neiEnd(); j = v.neiNext(j))
                        {
                            int neighboringAtomID = v.neiVertex(j);
                            if (!groupAtoms.find(neighboringAtomID))
                            {
                                if (!useDisplayPosition)
                                {
                                    pos.add(mol.getAtomXyz(atomID));
                                    posCnt++;
                                }
                                int neighboringBondID = v.neiEdge(j), bondID = -1;
                                if (mol.findEdgeIndex(neighboringAtomID, superAtomID) < 0)
                                {
                                    int oldBondMappingInvPosition = _bondMappingInv.at(neighboringBondID);
                                    if (mol.isQueryMolecule())
                                    {
                                        QueryMolecule& qm = mol.asQueryMolecule();
                                        bondID = qm.addBond(superAtomID, neighboringAtomID, qm.getBond(neighboringBondID).clone());
                                    }
                                    else
                                    {
                                        Molecule& amol = mol.asMolecule();
                                        int oldBondTopology = amol.getBondTopology(neighboringBondID);
                                        int oldBondDirection = amol.getBondDirection(neighboringBondID);
                                        amol.removeBond(neighboringBondID);
                                        bondID = amol.addBond(neighboringAtomID, superAtomID, amol.getBondOrder(neighboringBondID));
                                        amol.setEdgeTopology(bondID, oldBondTopology);
                                        amol.setBondDirection(bondID, oldBondDirection);
                                    }
                                    if (_bondMappingInv.find(bondID) != _bondMappingInv.end())
                                        _bondMappingInv.erase(bondID);
                                    _bondMappingInv.emplace(bondID, oldBondMappingInvPosition);
                                }
                            }
                        }
                        mol.removeAtom(atomID);
                    }

                    if (useDisplayPosition)
                    {
                        mol.setAtomXyz(superAtomID, displayPosition.x, displayPosition.y, displayPosition.z);
                    }
                    else
                    {
                        if (posCnt == 0)
                        {
                            pos.copy(centre);
                        }
                        else
                        {
                            pos.scale(1.f / posCnt);
                        }
                        mol.setAtomXyz(superAtomID, pos.x, pos.y, pos.z);
                    }
                }
            }
        }
    }

    QS_DEF(BaseMolecule::Mapping, mapAtom);
    mapAtom.clear();
    for (int i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
    {
        SGroup& sgroup = mol.sgroups.getSGroup(i);
        if (sgroup.sgroup_type == SGroup::SG_TYPE_MUL)
        {
            BaseMolecule::collapse(mol, i, mapAtom, _bondMappingInv);
        }
    }
}

int dblcmp(double a, double b, void* /*context*/)
{
    return a > b ? 1 : (a < b ? -1 : 0);
}

struct Segment
{
    int id;
    Vec2f p0, p1;
    int beg, end;
    int pos;
};

struct Event
{
    int id;
    Vec2f p;
    bool begin;
};

int evcmp(const Event& a, const Event& b, void* /*context*/)
{
    if (a.p.x > b.p.x)
        return 1;
    if (a.p.x < b.p.x)
        return -1;
    if (a.p.y > b.p.y)
        return 1;
    if (a.p.y < b.p.y)
        return -1;
    if (a.begin && !b.begin)
        return 1;
    if (!a.begin && b.begin)
        return -1;
    return 0;
}

float getFreeAngle(const ObjArray<Vec2f>& pp)
{
    QS_DEF(Array<float>, angle);
    angle.clear();
    int len = pp.size();
    for (int j = 0; j < len; ++j)
    {
        Vec2f d;
        d.diff(pp[(j + 1) % len], pp[j]);
        angle.push(atan2f(d.y, d.x));
    }
    angle.qsort(dblcmp, NULL);
    int j0 = -1;
    float maxAngle = -1;
    for (int j = 0; j < angle.size() - 1; ++j)
    {
        float a = angle[j + 1] - angle[j];
        if (a > maxAngle)
        {
            maxAngle = a;
            j0 = j;
        }
    }
    return angle[j0] + maxAngle / 4;
}

int loopDist(int i, int j, int len)
{
    if (i > j)
    {

        std::swap(i, j);
    }
    int d1 = j - i;
    int d2 = i + len - j;
    return std::min(d1, d2);
}

class SegmentList : protected RedBlackSet<int>
{
public:
    SegmentList(ObjArray<Segment>& ss) : segments(ss)
    {
        xPos = 0;
    }

    // returns true if no intersection was detected upon this insertion
    bool insertSegment(double pos, int seg)
    {
        xPos = pos;
        if (find(seg))
            return false;
        int curId = insert(seg);
        int nextId = next(curId);
        int prevId = nextPost(curId);
        Segment& segmentCurrent = segments[seg];
        segmentCurrent.pos = curId;
        if (nextId < end())
        {
            int nextSeg = key(nextId);
            if (loopDist(seg, nextSeg, segments.size()) > 1)
            {
                const Segment& segmentNext = segments[nextSeg];
                bool intersectNext = Vec2f::segmentsIntersect(segmentCurrent.p0, segmentCurrent.p1, segmentNext.p0, segmentNext.p1);
                if (intersectNext)
                    return false;
            }
        }
        if (prevId < end())
        {
            int prevSeg = key(prevId);
            if (loopDist(seg, prevSeg, segments.size()) > 1)
            {
                const Segment& segmentPrev = segments[prevSeg];
                bool intersectPrev = Vec2f::segmentsIntersect(segmentCurrent.p0, segmentCurrent.p1, segmentPrev.p0, segmentPrev.p1);
                if (intersectPrev)
                    return false;
            }
        }
        return true;
    }

    void removeSegment(int segmentId)
    {
        _removeNode(segments[segmentId].pos);
    }

    double xPos;

protected:
    int _compare(int key, const Node& node) const override
    {
        const Segment& a = segments[key];
        const Segment& b = segments[node.key];
        double ya = a.p0.y + (xPos - a.p0.x) * (a.p1.y - a.p0.y) / (a.p1.x - a.p0.x);
        double yb = b.p0.y + (xPos - b.p0.x) * (b.p1.y - b.p0.y) / (b.p1.x - b.p0.x);
        return ya > yb ? 1 : (ya < yb ? -1 : 0);
    }

private:
    ObjArray<Segment>& segments;

    SegmentList(const SegmentList& other);
};

float getMinDotProduct(const ObjArray<Vec2f>& pp, float tilt)
{
    float minDot = 1.0;
    for (int j = 0; j < pp.size(); ++j)
    {
        Vec2f a, b, d;
        a.copy(pp[j]);
        b.copy(pp[(j + 1) % pp.size()]);
        a.rotate(tilt);
        b.rotate(tilt);
        d.diff(b, a);
        float dot = fabs(d.x / d.length());
        if (dot < minDot)
            minDot = dot;
    }
    return minDot;
}

bool MoleculeRenderInternal::_ringHasSelfIntersectionsSimple(const Ring& ring)
{
    for (int j = 0; j < ring.bondEnds.size(); ++j)
    {
        for (int k = j + 2; k < std::min(ring.bondEnds.size(), ring.bondEnds.size() + j - 1); ++k)
        {
            const BondEnd& be1 = _be(ring.bondEnds[j]);
            const BondEnd& be2 = _be(ring.bondEnds[k]);
            const BondDescr& b1 = _bd(be1.bid);
            const BondDescr& b2 = _bd(be2.bid);
            const Vec2f& a00 = _ad(b1.beg).pos;
            const Vec2f& a01 = _ad(b1.end).pos;
            const Vec2f& a10 = _ad(b2.beg).pos;
            const Vec2f& a11 = _ad(b2.end).pos;
            if (Vec2f::segmentsIntersect(a00, a01, a10, a11))
                return true;
        }
    }
    return false;
}

bool MoleculeRenderInternal::_ringHasSelfIntersections(const Ring& ring)
{
    QS_DEF(ObjArray<Vec2f>, pp);
    pp.clear();
    int len = ring.bondEnds.size();
    for (int j = 0; j < len; ++j)
    {
        pp.push().copy(_ad(_be(ring.bondEnds[j]).aid).pos);
    }

    float tilt = getFreeAngle(pp) + (float)(M_PI / 2);

    QS_DEF(ObjArray<Event>, events);
    events.clear();
    events.reserve(2 * len);
    QS_DEF(ObjArray<Segment>, segments);
    segments.clear();
    segments.reserve(len);
    for (int j = 0; j < len; ++j)
    {
        Vec2f p1, p2;
        p1.copy(pp[j]);
        p2.copy(pp[(j + 1) % len]);
        p1.rotate(tilt);
        p2.rotate(tilt);
        bool revOrder = (p1.x > p2.x) || (p1.x == p2.x && p1.y > p2.y);
        Segment& segment = segments.push();
        segment.id = j;
        segment.p0.copy(revOrder ? p2 : p1);
        segment.p1.copy(revOrder ? p1 : p2);
        Event& ev1 = events.push();
        ev1.id = j;
        ev1.begin = true;
        ev1.p.copy(segment.p0);
        Event& ev2 = events.push();
        ev2.id = j;
        ev2.begin = false;
        ev2.p.copy(segment.p1);
    }

    // order the events
    events.qsort(evcmp, NULL);

    // sweep line pass
    SegmentList sl(segments);
    for (int i = 0; i < events.size(); ++i)
    {
        Event& ev = events[i];
        if (ev.begin)
        {
            if (!sl.insertSegment(ev.p.x + 1e-4, ev.id))
                return true; // intersection detected
        }
        else
        {
            sl.removeSegment(ev.id);
        }
    }

    return false;
}

void MoleculeRenderInternal::_findRings()
{
    QS_DEF(RedBlackSet<int>, mask);
    for (int i = 0; i < _data.bondends.size(); ++i)
    {
        BondEnd& be = _be(i);
        if (be.lRing != -1)
            continue;
        mask.clear();
        int rid = _data.rings.size();
        _data.rings.push();
        Ring& ring = _data.rings[rid];
        ring.bondEnds.push(i);

        int j = be.next;
        mask.insert(be.aid);
        for (int c = 0; j != i; j = _be(j).next, ++c)
        {
            if (c > _data.bondends.size() || j < 0 || _be(j).lRing != -1)
                break;
            int aid = _be(j).aid;
            if (mask.find(aid))
            {
                while (ring.bondEnds.size() > 1 && _be(ring.bondEnds.top()).aid != aid)
                {
                    _be(ring.bondEnds.top()).lRing = -2;
                    ring.bondEnds.pop();
                }
                ring.bondEnds.pop();
            }
            else
            {
                mask.insert(aid);
            }
            ring.bondEnds.push(j);
        }
        if (i != j || ring.bondEnds.size() < 3)
        {
            for (int k = 0; k < ring.bondEnds.size(); ++k)
                _be(ring.bondEnds[k]).lRing = -2;
            _data.rings.pop();
            continue;
        }

        bool selfIntersection = _ringHasSelfIntersections(ring);
        if (selfIntersection)
        {
            for (int k = 0; k < ring.bondEnds.size(); ++k)
                _be(ring.bondEnds[k]).lRing = -2;
            _data.rings.pop();
            continue;
        }

        // for the inner loops, sum of the angles should be (n-2)*pi,
        // for the outer ones (n+2)*pi
        float angleSum = 0;
        for (int k = 0; k < ring.bondEnds.size(); ++k)
        {
            int j1 = (k + 1) % ring.bondEnds.size();
            const Vec2f& da = _be(ring.bondEnds[k]).dir;
            const Vec2f& db = _be(ring.bondEnds[j1]).dir;
            float angle = (float)M_PI - atan2(-Vec2f::cross(da, db), Vec2f::dot(da, db));
            angleSum += angle;
        }

        // sum of all angles for inner loop is (n - 2) Pi and (n + 2) Pi for the outer one
        bool inner = (angleSum < ring.bondEnds.size() * M_PI);

        if (!inner)
        {
            for (int k = 0; k < ring.bondEnds.size(); ++k)
                _be(ring.bondEnds[k]).lRing = -2;
            _data.rings.pop();
            continue;
        }

        for (int k = 0; k < ring.bondEnds.size(); ++k)
            _be(ring.bondEnds[k]).lRing = rid;

        if (_opt.showCycles)
        {
            float cycleLineOffset = _settings.unit * 9;
            QS_DEF(Array<Vec2f>, vv);
            vv.clear();
            for (int k = 0; k < ring.bondEnds.size() + 1; ++k)
            {
                const BondEnd& be1 = _be(ring.bondEnds[k % ring.bondEnds.size()]);
                Vec2f v = be1.dir;
                v.rotateL(be1.lang / 2);
                v.scale(cycleLineOffset);
                v.add(_ad(be1.aid).pos);
                vv.push(v);
            }
            _cw.setSingleSource(CWC_BLUE);
            _cw.setLineWidth(_settings.unit);
            _cw.drawPoly(vv);
        }
    }

    for (int i = 0; i < _data.rings.size(); ++i)
    {
        Ring& ring = _data.rings[i];

        Array<Vec2f> pp;
        for (int j = 0; j < ring.bondEnds.size(); ++j)
            pp.push().copy(_ad(_be(ring.bondEnds[j]).aid).pos);

        for (int j = 0; j < ring.bondEnds.size(); ++j)
            ring.center.add(pp[j]);
        ring.center.scale(1.0f / ring.bondEnds.size());

        //{
        //   TextItem ti;
        //   bprintf(ti.text, "%.2f", angleSum);
        //   ti.color = CWC_BLUE;
        //   ti.fontsize = _settings.labelFont;
        //   _cw.setTextItemSize(ti, ring.center);
        //   _cw.drawTextItemText(ti);
        //}

        float r = -1;
        for (int j = 0; j < ring.bondEnds.size(); ++j)
        {
            Vec2f df;
            BondEnd& be = _be(ring.bondEnds[j]);
            df.diff(pp[j], ring.center);
            float l = fabs(Vec2f::dot(df, be.lnorm));
            if (r < 0 || l < r)
                r = l;
            ring.angles.push(atan2(df.y, df.x));
        }
        ring.radius = r;

        ring.aromatic = true;
        int dblBondCount = 0;
        for (int j = 0; j < ring.bondEnds.size(); ++j)
        {
            int type = _bd(_be(ring.bondEnds[j]).bid).type;
            if (type != BOND_AROMATIC)
                ring.aromatic = false;
            if (type == BOND_DOUBLE)
                dblBondCount++;
        }
        ring.dblBondCount = dblBondCount;
    }

    for (int i = _mol->edgeBegin(); i < _mol->edgeEnd(); i = _mol->edgeNext(i))
    {
        BondDescr& bd = _bd(i);
        BondEnd& be1 = _be(bd.be1);
        BondEnd& be2 = _be(bd.be2);
        bd.inRing = (be1.lRing >= 0 || be2.lRing >= 0);
        bd.aromRing = ((be1.lRing >= 0) ? _data.rings[be1.lRing].aromatic : false) || ((be2.lRing >= 0) ? _data.rings[be2.lRing].aromatic : false);
    }
}

void MoleculeRenderInternal::_prepareLabels()
{
    for (int i = _mol->vertexBegin(); i < _mol->vertexEnd(); i = _mol->vertexNext(i))
        _prepareLabelText(i);
}

void MoleculeRenderInternal::_objCoordTransform(Vec2f& p, const Vec2f& v) const
{
    // shift, mirror of Y axis, scale
    p.set((v.x - _min.x) * _scale, (_max.y - v.y) * _scale);
}

void MoleculeRenderInternal::_objDistTransform(Vec2f& p, const Vec2f& v) const
{
    p.set(v.x * _scale, -v.y * _scale);
}

void MoleculeRenderInternal::_initCoordinates()
{
    Vec2f v;
    for (int i = _mol->vertexBegin(); i < _mol->vertexEnd(); i = _mol->vertexNext(i))
    {
        Vec2f::projectZ(v, _mol->getAtomXyz(i));
        _objCoordTransform(_ad(i).pos, v);
    }
}

void MoleculeRenderInternal::_determineStereoGroupsMode()
{
    const MoleculeStereocenters& sc = _mol->stereocenters;

    _lopt.stereoMode = STEREOGROUPS_HIDE;
    if (_opt.stereoMode == STEREO_STYLE_NONE)
        return;
    bool allAbs = true, singleAndGroup = true, none = true;
    int andGid = -1;
    for (int i = sc.begin(); i < sc.end(); i = sc.next(i))
    {
        int aid, type, gid, pyramid[4];
        sc.get(i, aid, type, gid, pyramid);

        if (type != MoleculeStereocenters::ATOM_ANY)
            none = false;
        if (type != MoleculeStereocenters::ATOM_ABS)
            allAbs = false;
        if (type != MoleculeStereocenters::ATOM_AND || (andGid > 0 && andGid != gid))
            singleAndGroup = false;
        else
            andGid = gid;

        if (!allAbs && !singleAndGroup)
            break;
    }

    if (_opt.stereoMode == STEREO_STYLE_OLD)
    {
        if (singleAndGroup)
            return;

        if (allAbs && !none)
        {

            TextItem& tiChiral = _data.textitems[_pushTextItem(RenderItem::RIT_CHIRAL, CWC_BASE, false)];
            bprintf(tiChiral.text, "Chiral");
            tiChiral.fontsize = FONT_SIZE_LABEL;
            _cw.setTextItemSize(tiChiral);
            Rect2f bbox;
            Vec2f pos;
            _mol->getBoundingBox(bbox);
            _objCoordTransform(pos, Vec2f(bbox.left(), bbox.top()));
            tiChiral.bbp.set(pos.x + (_max.x - _min.x) * _scale - tiChiral.bbsz.x, pos.y - tiChiral.bbsz.y * 2);
            _cw.setSingleSource(CWC_BASE);
            _cw.drawTextItemText(tiChiral, _idle);
            return;
        }
    }

    _lopt.stereoMode = STEREOGROUPS_SHOW;
    int aid, type, groupId, pyramid[4];
    for (int i = sc.begin(); i < sc.end(); i = sc.next(i))
    {
        sc.get(i, aid, type, groupId, pyramid);
        AtomDesc& ad = _ad(aid);

        ad.stereoGroupType = type;
        if (type == MoleculeStereocenters::ATOM_AND || type == MoleculeStereocenters::ATOM_OR)
            ad.stereoGroupNumber = groupId;
    }
}

bool MoleculeRenderInternal::_isSingleHighlighted(int aid)
{
    const Vertex& vertex = _mol->getVertex(aid);
    if (!_vertexIsHighlighted(aid))
        return false;
    if (_opt.highlightedLabelsVisible)
        return true;
    for (int j = vertex.neiBegin(); j < vertex.neiEnd(); j = vertex.neiNext(j))
        if (_edgeIsHighlighted(vertex.neiEdge(j)))
            return false;
    return true;
}

bool MoleculeRenderInternal::_vertexIsHighlighted(int aid)
{
    return _mol->isAtomHighlighted(aid);
}

bool MoleculeRenderInternal::_edgeIsHighlighted(int bid)
{
    return _mol->isBondHighlighted(bid);
}

bool MoleculeRenderInternal::_hasQueryModifiers(int aid)
{
    bool hasConstraints = false;
    QUERY_MOL_BEGIN(_mol);
    QueryMolecule::Atom& qa = qmol.getAtom(aid);
    hasConstraints = qa.hasConstraint(QueryMolecule::ATOM_RING_BONDS) || qa.hasConstraint(QueryMolecule::ATOM_RING_BONDS_AS_DRAWN) ||
                     qa.hasConstraint(QueryMolecule::ATOM_SUBSTITUENTS) || qa.hasConstraint(QueryMolecule::ATOM_SUBSTITUENTS_AS_DRAWN) ||
                     qa.hasConstraint(QueryMolecule::ATOM_UNSATURATION) || qa.hasConstraint(QueryMolecule::ATOM_TOTAL_H);
    QUERY_MOL_END;
    return hasConstraints || _ad(aid).fixed || _ad(aid).exactChange;
}

void MoleculeRenderInternal::_findNearbyAtoms()
{
    float maxDistance = _settings.neighboringAtomDistanceTresholdA * 2;
    RedBlackObjMap<int, RedBlackObjMap<int, Array<int>>> buckets;

    for (int i = _mol->vertexBegin(); i < _mol->vertexEnd(); i = _mol->vertexNext(i))
    {
        const Vec2f& v = _ad(i).pos;
        int xBucket = (int)(v.x / maxDistance);
        int yBucket = (int)(v.y / maxDistance);
        RedBlackObjMap<int, Array<int>>& bucketRow = buckets.findOrInsert(yBucket);
        Array<int>& bucket = bucketRow.findOrInsert(xBucket);
        bucket.push(i);
    }

    for (int i = _mol->vertexBegin(); i < _mol->vertexEnd(); i = _mol->vertexNext(i))
    {
        const Vec2f& v = _ad(i).pos;
        int xBucket = (int)(v.x / maxDistance);
        int yBucket = (int)(v.y / maxDistance);
        for (int j = 0; j < 3; ++j)
        {
            for (int k = 0; k < 3; ++k)
            {
                int x = xBucket + j - 1;
                int y = yBucket + k - 1;
                if (!buckets.find(y))
                    continue;
                RedBlackObjMap<int, Array<int>>& bucketRow = buckets.at(y);
                if (!bucketRow.find(x))
                    continue;
                const Array<int>& bucket = bucketRow.at(x);
                for (int r = 0; r < bucket.size(); ++r)
                {
                    int aid = bucket[r];
                    if (aid == i)
                        continue;
                    const Vec2f& v1 = _ad(aid).pos;
                    if (Vec2f::dist(v, v1) < maxDistance)
                    {
                        _ad(i).nearbyAtoms.push(aid);
                    }
                }
            }
        }
        // const Array<int>& natoms = _ad(i).nearbyAtoms;
        // printf("%02d:", i);
        // for (int j = 0; j < natoms.size(); ++j) {
        //   printf(" %02d", natoms[j]);
        //}
        // printf("\n");
    }
    // printf("\n");
}

int _argMax(float* vv, int length)
{
    int iMax = 0;
    for (int i = 1; i < length; ++i)
    {
        if (vv[iMax] < vv[i])
            iMax = i;
    }
    return iMax;
}

float _sqr(float a)
{
    return a * a;
}

void MoleculeRenderInternal::_initHydroPos(int aid)
{
    AtomDesc& ad = _ad(aid);
    const Vertex& v = _mol->getVertex(aid);
    if (v.degree() == 0 && ElementHygrodenOnLeft(ad.label))
    {
        ad.implHPosWeights[HYDRO_POS_RIGHT] = 0.2f; // weights are relative, absoute values don't matter
        ad.implHPosWeights[HYDRO_POS_LEFT] = 0.3f;
    }
    else
    {
        ad.implHPosWeights[HYDRO_POS_RIGHT] = 0.3f;
        ad.implHPosWeights[HYDRO_POS_LEFT] = 0.2f;
    }
    ad.implHPosWeights[HYDRO_POS_UP] = 0.1f;
    ad.implHPosWeights[HYDRO_POS_DOWN] = 0.0f;
    ad.implHPosWeights[HYDRO_POS_RIGHT] -= ad.rightSin > _settings.minSin ? ad.rightSin : 0;
    ad.implHPosWeights[HYDRO_POS_LEFT] -= ad.leftSin > _settings.minSin ? ad.leftSin : 0;
    ad.implHPosWeights[HYDRO_POS_UP] -= ad.upperSin > _settings.minSin ? ad.upperSin : 0;
    ad.implHPosWeights[HYDRO_POS_DOWN] -= ad.lowerSin > _settings.minSin ? ad.lowerSin : 0;
}

int MoleculeRenderInternal::_hydroPosFindConflict(int i)
{
    const AtomDesc& ad = _ad(i);
    for (int j = 0; j < ad.nearbyAtoms.size(); ++j)
    {
        int aid = ad.nearbyAtoms[j];
        Vec2f d;
        d.diff(_ad(aid).pos, ad.pos);
        HYDRO_POS orientation = d.x < d.y ? (d.x > -d.y ? HYDRO_POS_DOWN : HYDRO_POS_LEFT) : (d.x > -d.y ? HYDRO_POS_RIGHT : HYDRO_POS_UP);
        float aDist = std::max(fabs(d.x), fabs(d.y));
        float bDist = std::min(fabs(d.x), fabs(d.y));
        if (orientation == ad.hydroPos && bDist < _settings.neighboringAtomDistanceTresholdB &&
            (aDist < _settings.neighboringAtomDistanceTresholdA ||
             (aDist < _settings.neighboringAtomDistanceTresholdA * 2 && _ad(aid).hydroPos == 3 - ad.hydroPos)))
            return orientation;
    }
    return -1;
}

bool MoleculeRenderInternal::_hydroPosCorrectGreedy()
{
    for (int i = _mol->vertexBegin(); i < _mol->vertexEnd(); i = _mol->vertexNext(i))
    {
        AtomDesc& ad = _ad(i);
        if (!ad.showLabel || ad.implicit_h <= 0)
            continue;

        int orientation = _hydroPosFindConflict(i);
        if (orientation >= 0)
        {
            ad.implHPosWeights[orientation] -= 1;
            ad.hydroPos = (HYDRO_POS)_argMax(ad.implHPosWeights, 4);
        }
    }

    for (int i = _mol->vertexBegin(); i < _mol->vertexEnd(); i = _mol->vertexNext(i))
    {
        AtomDesc& ad = _ad(i);
        if (!ad.showLabel || ad.implicit_h <= 0)
            continue;

        int orientation = _hydroPosFindConflict(i);
        if (orientation >= 0)
            return false;
    }
    return true;
}

void MoleculeRenderInternal::_hydroPosCorrectRepulse()
{
    for (int i = _mol->vertexBegin(); i < _mol->vertexEnd(); i = _mol->vertexNext(i))
    {
        AtomDesc& ad = _ad(i);
        if (!ad.showLabel || ad.implicit_h <= 0)
            continue;
        _initHydroPos(i);
        for (int j = 0; j < ad.nearbyAtoms.size(); ++j)
        {
            int aid = ad.nearbyAtoms[j];
            Vec2f d;
            d.diff(_ad(aid).pos, ad.pos);
            if (d.length() < _settings.neighboringLabelTolerance && _ad(aid).showLabel)
            {
                ad.implHPosWeights[d.x < d.y ? (d.x > -d.y ? HYDRO_POS_DOWN : HYDRO_POS_LEFT) : (d.x > -d.y ? HYDRO_POS_RIGHT : HYDRO_POS_UP)] -= 1;
            }
        }
        ad.hydroPos = (HYDRO_POS)_argMax(ad.implHPosWeights, 4);
    }
}

void MoleculeRenderInternal::_initAtomData()
{
    QUERY_MOL_BEGIN(_mol);
    for (int i = 0; i < qmol.fixed_atoms.size(); ++i)
        _ad(i).fixed = true;
    QUERY_MOL_END;
    for (int i = 0; i < _data.exactChanges.size(); ++i)
        if (_data.exactChanges[i])
            _ad(i).exactChange = true;

    _findNearbyAtoms();
    for (int i = _mol->vertexBegin(); i < _mol->vertexEnd(); i = _mol->vertexNext(i))
    {
        AtomDesc& ad = _ad(i);
        BaseMolecule& bm = *_mol;
        const Vertex& vertex = bm.getVertex(i);

        // QS_DEF(Array<char>, buf);
        // buf.clear();
        // bm.getAtomDescription(i, buf);
        // printf("%s\n", buf.ptr());

        int atomNumber = bm.getAtomNumber(i);
        QUERY_MOL_BEGIN(_mol);
        if (!QueryMolecule::queryAtomIsRegular(qmol, i))
            atomNumber = -1;
        QUERY_MOL_END;
        if (bm.isPseudoAtom(i))
        {
            ad.type = AtomDesc::TYPE_PSEUDO;
            ad.pseudo.readString(bm.getPseudoAtom(i), true);
        }
        else if (bm.isTemplateAtom(i))
        {
            ad.type = AtomDesc::TYPE_PSEUDO;
            ad.pseudo.readString(bm.getTemplateAtom(i), true);
        }
        else if (atomNumber < 0 || atomNumber == ELEM_RSITE)
        {
            ad.type = AtomDesc::TYPE_QUERY;
        }
        else
        {
            ad.type = AtomDesc::TYPE_REGULAR;
        }

        ad.label = 0;
        if (ad.type == AtomDesc::TYPE_REGULAR)
            ad.label = atomNumber;

        ad.queryLabel = -1;
        if (ad.type == AtomDesc::TYPE_QUERY)
        {
            if (!bm.isRSite(i))
            {
                QUERY_MOL_BEGIN(_mol);
                ad.queryLabel = QueryMolecule::parseQueryAtom(qmol, i, ad.list);
                if (ad.queryLabel < 0)
                {
                    bm.getAtomDescription(i, ad.pseudo);
                    ad.type = AtomDesc::TYPE_PSEUDO;
                    ad.pseudoAtomStringVerbose = true;
                }
                QUERY_MOL_END;
            }
        }

        if (_opt.atomColoring && ad.label > 0)
            ad.color = _cw.getElementColor(ad.label);

        Vec2f h(1, 0);
        bool hasBondOnRight = false, hasBondOnLeft = false;
        for (int j = vertex.neiBegin(); j < vertex.neiEnd(); j = vertex.neiNext(j))
        {
            Vec2f d(_ad(vertex.neiVertex(j)).pos);
            d.sub(ad.pos);
            d.normalize();
            if (d.x > 0)
                ad.rightSin = std::max(ad.rightSin, d.x);
            else
                ad.leftSin = std::max(ad.leftSin, -d.x);
            if (d.y > 0)
                ad.lowerSin = std::max(ad.lowerSin, d.y);
            else
                ad.upperSin = std::max(ad.upperSin, -d.y);
        }
        if (ad.rightSin > 0.7)
            hasBondOnRight = true;
        if (ad.leftSin > 0.7)
            hasBondOnLeft = true;

        int charge = bm.getAtomCharge(i);
        int isotope = bm.getAtomIsotope(i);
        int radical = -1;
        int valence;
        bool query = bm.isQueryMolecule();

        valence = bm.getExplicitValence(i);

        if (!bm.isRSite(i) && !bm.isPseudoAtom(i) && !bm.isTemplateAtom(i))
        {
            radical = bm.getAtomRadical_NoThrow(i, -1);
            if (!bm.isQueryMolecule())
                ad.implicit_h = bm.asMolecule().getImplicitH_NoThrow(i, 0);
        }

        bool plainCarbon = ad.label == ELEM_C && charge == (query ? CHARGE_UNKNOWN : 0) && isotope == (query ? -1 : 0) && radical <= 0 && valence == -1 &&
                           !_hasQueryModifiers(i);

        ad.showLabel = true;
        if (_opt.labelMode == LABEL_MODE_ALL || vertex.degree() == 0)
            ;
        else if (_opt.labelMode == LABEL_MODE_NONE)
            ad.showLabel = false;
        else if (plainCarbon && (_opt.labelMode == LABEL_MODE_HETERO || vertex.degree() > 1) && !_isSingleHighlighted(i))
        {
            ad.showLabel = false;
            if (vertex.degree() == 2)
            {
                int k1 = vertex.neiBegin();
                int k2 = vertex.neiNext(k1);
                if (_bd(vertex.neiEdge(k1)).type == _bd(vertex.neiEdge(k2)).type)
                {
                    float dot = Vec2f::dot(_getBondEnd(i, k1).dir, _getBondEnd(i, k2).dir);
                    if (dot < -0.97)
                        ad.showLabel = true;
                }
            }
        }

        _initHydroPos(i);
        ad.hydroPos = (HYDRO_POS)_argMax(ad.implHPosWeights, 4);

        int uaid = _atomMappingInv.size() > i ? _atomMappingInv[i] : i;
        if (uaid >= 0)
        {
            if (_data.inversions.size() > uaid)
            {
                ad.inversion = _data.inversions[uaid];
            }
            if (_data.aam.size() > uaid)
            {
                ad.aam = _data.aam[uaid];
            }
        }
    }

    if (!_hydroPosCorrectGreedy())
        if (!_hydroPosCorrectGreedy())
            _hydroPosCorrectRepulse();
}

void MoleculeRenderInternal::_findAnglesOverPi()
{
    for (int i = _mol->vertexBegin(); i < _mol->vertexEnd(); i = _mol->vertexNext(i))
    {
        int rightmost = -1, leftmost = -1;
        const Vertex& vertex = _mol->getVertex(i);
        if (vertex.degree() <= 1)
            continue;
        for (int j = vertex.neiBegin(); j < vertex.neiEnd(); j = vertex.neiNext(j))
        {
            int rbeidx = _getBondEndIdx(i, j);
            BondEnd& rbe = _be(rbeidx);
            if (rbe.lang > M_PI)
            {
                leftmost = rbe.lnei;
                rightmost = rbeidx;
                break;
            }
        }
        if (leftmost < 0)
        {
            for (int j = vertex.neiBegin(); j < vertex.neiEnd(); j = vertex.neiNext(j))
            {
                int rbeidx = _getBondEndIdx(i, j);
                BondEnd& rbe = _be(rbeidx);
                BondEnd& lbe = _be(rbe.lnei);
                if (_bd(rbe.bid).type == BOND_DOUBLE && _bd(lbe.bid).type == BOND_DOUBLE && rbe.lang < M_PI)
                {
                    rightmost = rbeidx;
                    leftmost = rbe.lnei;
                }
            }
        }
        if (leftmost < 0)
        {
            for (int j = vertex.neiBegin(); j < vertex.neiEnd(); j = vertex.neiNext(j))
            {
                BondDescr& bd = _bd(vertex.neiEdge(j));
                if (bd.type == BOND_SINGLE && bd.stereodir == 0)
                    continue;
                BondEnd& be = _getBondEnd(i, j);
                if (be.lsin > 0 && be.rsin > 0 && be.lang + be.rang > M_PI)
                {
                    leftmost = be.lnei;
                    rightmost = be.rnei;
                }
            }
        }
        if (leftmost < 0)
            continue;
        if (!_ad(i).showLabel)
        {
            BondEnd& rmbe = _be(rightmost);
            BondEnd& lmbe = _be(leftmost);
            float dot = Vec2f::dot(rmbe.dir, lmbe.dir);
            if (dot > 0 || 1 + dot < 1e-4)
                continue;
            float ahs = sqrt((1 + dot) / (1 - dot));
            lmbe.offset = rmbe.offset = -ahs * std::min(_bd(lmbe.bid).thickness, _bd(rmbe.bid).thickness) / 2;
        }
    }
}

#ifdef _WIN32
#pragma warning(pop)
#endif
