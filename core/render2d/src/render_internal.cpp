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
#include "base_cpp/queue.h"
#include "base_cpp/scanner.h"
#include "base_cpp/tree.h"
#include "molecule/ket_commons.h"
#include "molecule/molecule.h"
#include "molecule/molecule_sgroups.h"
#include "molecule/query_molecule.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction.h"
#include "render_context.h"

using namespace indigo;

#define BOND_STEREO_BOLD 10001

static bool ElementHygrodenOnLeft[] = {
    false, // filler
    false, // ELEM_H
    false, // ELEM_He
    false, // ELEM_Li
    false, // ELEM_Be
    false, // ELEM_B
    false, // ELEM_C
    false, // ELEM_N
    true,  // ELEM_O
    true,  // ELEM_F
    false, // ELEM_Ne
    false, // ELEM_Na
    false, // ELEM_Mg
    false, // ELEM_Al
    false, // ELEM_Si
    false, // ELEM_P
    true,  // ELEM_S
    true,  // ELEM_Cl
    false, // ELEM_Ar
    false, // ELEM_K
    false, // ELEM_Ca
    false, // ELEM_Sc
    false, // ELEM_Ti
    false, // ELEM_V
    false, // ELEM_Cr
    false, // ELEM_Mn
    false, // ELEM_Fe
    false, // ELEM_Co
    false, // ELEM_Ni
    false, // ELEM_Cu
    false, // ELEM_Zn
    false, // ELEM_Ga
    false, // ELEM_Ge
    false, // ELEM_As
    true,  // ELEM_Se
    true,  // ELEM_Br
    false, // ELEM_Kr
    false, // ELEM_Rb
    false, // ELEM_Sr
    false, // ELEM_Y
    false, // ELEM_Zr
    false, // ELEM_Nb
    false, // ELEM_Mo
    false, // ELEM_Tc
    false, // ELEM_Ru
    false, // ELEM_Rh
    false, // ELEM_Pd
    false, // ELEM_Ag
    false, // ELEM_Cd
    false, // ELEM_In
    false, // ELEM_Sn
    false, // ELEM_Sb
    false, // ELEM_Te
    true,  // ELEM_I
    false, // ELEM_Xe
    false, // ELEM_Cs
    false, // ELEM_Ba
    false, // ELEM_La
    false, // ELEM_Ce
    false, // ELEM_Pr
    false, // ELEM_Nd
    false, // ELEM_Pm
    false, // ELEM_Sm
    false, // ELEM_Eu
    false, // ELEM_Gd
    false, // ELEM_Tb
    false, // ELEM_Dy
    false, // ELEM_Ho
    false, // ELEM_Er
    false, // ELEM_Tm
    false, // ELEM_Yb
    false, // ELEM_Lu
    false, // ELEM_Hf
    false, // ELEM_Ta
    false, // ELEM_W
    false, // ELEM_Re
    false, // ELEM_Os
    false, // ELEM_Ir
    false, // ELEM_Pt
    false, // ELEM_Au
    false, // ELEM_Hg
    false, // ELEM_Tl
    false, // ELEM_Pb
    false, // ELEM_Bi
    false, // ELEM_Po
    false, // ELEM_At
    false, // ELEM_Rn
    false, // ELEM_Fr
    false, // ELEM_Ra
    false, // ELEM_Ac
    false, // ELEM_Th
    false, // ELEM_Pa
    false, // ELEM_U
    false, // ELEM_Np
    false, // ELEM_Pu
    false, // ELEM_Am
    false, // ELEM_Cm
    false, // ELEM_Bk
    false, // ELEM_Cf
    false, // ELEM_Es
    false, // ELEM_Fm
    false, // ELEM_Md
    false, // ELEM_No
    false  // ELEM_Lr
};

static bool _isBondWide(const BondDescr& bd)
{
    return bd.type == BOND_DOUBLE || bd.type == BOND_TRIPLE || bd.queryType == _BOND_DOUBLE_OR_AROMATIC || bd.queryType == _BOND_SINGLE_OR_AROMATIC ||
           _BOND_SINGLE_OR_DOUBLE;
}

RenderOptions::RenderOptions()
{
    clear();
}

void RenderOptions::clear()
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
    atomColoring = false;
    stereoMode = STEREO_STYLE_OLD;
    showReactingCenterUnchanged = false;
    centerDoubleBondWhenStereoAdjacent = false;
    showCycles = false;
    agentsBelowArrow = true;
    atomColorProp.clear();
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
        auto count = _mol->sgroups.getSGroupCount(SGroup::SG_TYPE_SUP);
        BaseMolecule& mol = *_mol;
        for (int i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
        {
            SGroup& sgroup = mol.sgroups.getSGroup(i);
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

void MoleculeRenderInternal::setIsRFragment(bool isRFragment)
{
    this->isRFragment = isRFragment;
}

void MoleculeRenderInternal::setScaleFactor(const float scaleFactor, const Vec2f& min, const Vec2f& max)
{
    _scale = scaleFactor;
    _min.copy(min);
    _max.copy(max);
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

void MoleculeRenderInternal::_placeBrackets(Sgroup& sg, const Array<int>& atoms, Array<Vec2f[2]>& brackets)
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
        _bondMappingInv.emplace(i, BaseMolecule::findMappedEdge(*clone, *_mol, i, _atomMappingInv.ptr()));
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
                    Vec3f centre;
                    for (int i = 0; i < group.atoms.size(); ++i)
                    {
                        int atomID = group.atoms[i];
                        centre.add(mol.getAtomXyz(atomID));
                    }
                    centre.scale(1.0f / group.atoms.size());
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
                        bool posCounted = false;
                        for (int j = v.neiBegin(); j < v.neiEnd(); j = v.neiNext(j))
                        {
                            int neighboringAtomID = v.neiVertex(j);
                            if (!groupAtoms.find(neighboringAtomID))
                            {
                                pos.add(mol.getAtomXyz(atomID));
                                posCounted = true;
                                posCnt++;
                                int neighboringBondID = v.neiEdge(j), bondID = -1;
                                if (mol.findEdgeIndex(neighboringAtomID, superAtomID) < 0)
                                {
                                    if (mol.isQueryMolecule())
                                    {
                                        QueryMolecule& qm = mol.asQueryMolecule();
                                        bondID = qm.addBond(superAtomID, neighboringAtomID, qm.getBond(neighboringBondID).clone());
                                    }
                                    else
                                    {
                                        Molecule& amol = mol.asMolecule();
                                        bondID = amol.addBond(superAtomID, neighboringAtomID, amol.getBondOrder(neighboringBondID));
                                        amol.setEdgeTopology(bondID, amol.getBondTopology(neighboringBondID));
                                    }
                                    if (_bondMappingInv.find(bondID) != _bondMappingInv.end())
                                        _bondMappingInv.erase(bondID);
                                    _bondMappingInv.emplace(bondID, _bondMappingInv.at(neighboringBondID));
                                }
                            }
                        }
                        mol.removeAtom(atomID);
                    }
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

int dblcmp(double a, double b, void* context)
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

int evcmp(const Event& a, const Event& b, void* context)
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
            for (int j = 0; j < ring.bondEnds.size(); ++j)
                _be(ring.bondEnds[j]).lRing = -2;
            _data.rings.pop();
            continue;
        }

        bool selfIntersection = _ringHasSelfIntersections(ring);
        if (selfIntersection)
        {
            for (int j = 0; j < ring.bondEnds.size(); ++j)
                _be(ring.bondEnds[j]).lRing = -2;
            _data.rings.pop();
            continue;
        }

        // for the inner loops, sum of the angles should be (n-2)*pi,
        // for the outer ones (n+2)*pi
        float angleSum = 0;
        for (int j = 0; j < ring.bondEnds.size(); ++j)
        {
            int j1 = (j + 1) % ring.bondEnds.size();
            const Vec2f& da = _be(ring.bondEnds[j]).dir;
            const Vec2f& db = _be(ring.bondEnds[j1]).dir;
            float angle = (float)M_PI - atan2(-Vec2f::cross(da, db), Vec2f::dot(da, db));
            angleSum += angle;
        }

        // sum of all angles for inner loop is (n - 2) Pi and (n + 2) Pi for the outer one
        bool inner = (angleSum < ring.bondEnds.size() * M_PI);

        if (!inner)
        {
            for (int j = 0; j < ring.bondEnds.size(); ++j)
                _be(ring.bondEnds[j]).lRing = -2;
            _data.rings.pop();
            continue;
        }

        for (int j = 0; j < ring.bondEnds.size(); ++j)
            _be(ring.bondEnds[j]).lRing = rid;

        if (_opt.showCycles)
        {
            float cycleLineOffset = _settings.unit * 9;
            QS_DEF(Array<Vec2f>, vv);
            vv.clear();
            for (int j = 0; j < ring.bondEnds.size() + 1; ++j)
            {
                const BondEnd& be = _be(ring.bondEnds[j % ring.bondEnds.size()]);
                Vec2f v = be.dir;
                v.rotateL(be.lang / 2);
                v.scale(cycleLineOffset);
                v.add(_ad(be.aid).pos);
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
        for (int i = 0; i < ring.bondEnds.size(); ++i)
        {
            int type = _bd(_be(ring.bondEnds[i]).bid).type;
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
            tiChiral.bbp.set((_max.x - _min.x) * _scale - tiChiral.bbsz.x, -tiChiral.bbsz.y * 2);
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
    if (v.degree() == 0 && ElementHygrodenOnLeft[ad.label])
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

        ad.label = -1;
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
        if (_data.inversions.size() > uaid)
        {
            ad.inversion = _data.inversions[uaid];
        }
        if (_data.aam.size() > uaid)
        {
            ad.aam = _data.aam[uaid];
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

void MoleculeRenderInternal::_renderBondIds()
{
    // show bond ids
    if (_opt.showBondIds)
    {
        for (int i = _mol->edgeBegin(); i < _mol->edgeEnd(); i = _mol->edgeNext(i))
        {
            TextItem ti;
            ti.fontsize = FONT_SIZE_INDICES;
            ti.color = CWC_DARKGREEN;
            int base = _opt.atomBondIdsFromOne ? 1 : 0;
            bprintf(ti.text, "%i", i + base);
            Vec2f v;
            v.sum(_be(_bd(i).be1).p, _be(_bd(i).be2).p);
            v.scale(0.5);
            _cw.setTextItemSize(ti, v);
            _extendRenderItem(ti, _settings.boundExtent);
            _cw.drawItemBackground(ti);
            _cw.drawTextItemText(ti, _idle);
        }
    }

    // show bond end ids
    if (_opt.showBondEndIds)
    {
        for (int i = 0; i < _data.bondends.size(); ++i)
        {
            TextItem ti;
            ti.fontsize = FONT_SIZE_INDICES;
            ti.color = CWC_RED;
            bprintf(ti.text, "%i", i);
            Vec2f v;
            v.lineCombin2(_be(i).p, 0.75, _be(_getOpposite(i)).p, 0.25);
            _cw.setTextItemSize(ti, v);
            _extendRenderItem(ti, _settings.boundExtent);
            _cw.drawItemBackground(ti);
            _cw.drawTextItemText(ti, _idle);
        }
    }
    if (_opt.showNeighborArcs)
    {
        for (int i = 0; i < _data.bondends.size(); ++i)
        {
            BondEnd& be = _be(i);
            BondEnd& be1 = _be(be.lnei);
            BondEnd& be2 = _be(be.rnei);
            float a0 = atan2(be.dir.y, be.dir.x) - 0.1f;
            float a1 = atan2(be1.dir.y, be1.dir.x) + 0.1f;
            _cw.setSingleSource(CWC_RED);
            _cw.drawArc(_ad(be.aid).pos, _settings.bondSpace * 3, a1, a0);

            float a2 = atan2(be2.dir.y, be2.dir.x) - 0.1f;
            float a3 = atan2(be.dir.y, be.dir.x) + 0.1f;
            _cw.setSingleSource(CWC_DARKGREEN);
            _cw.drawArc(_ad(be.aid).pos, _settings.bondSpace * 3 + _settings.unit, a3, a2);
        }
    }
}

void MoleculeRenderInternal::_renderAtomIds()
{
    if (_opt.showAtomIds)
    {
        for (int i = _mol->vertexBegin(); i < _mol->vertexEnd(); i = _mol->vertexNext(i))
        {
            const AtomDesc& desc = _ad(i);
            for (int i = 0; i < desc.ticount; ++i)
            {
                const TextItem& ti = _data.textitems[i + desc.tibegin];
                if (ti.ritype == RenderItem::RIT_ATOMID)
                {
                    _cw.drawItemBackground(ti);
                    _cw.drawTextItemText(ti, _idle);
                }
            }
        }
    }
}

void MoleculeRenderInternal::_renderEmptyRFragment()
{
    if (!isRFragment || _data.atoms.size() > 0)
        return;
    int attachmentPointBegin = _data.attachmentPoints.size(); // always 0
    int attachmentPointCount = 2;

    Vec2f pos, dir1(1, 0), dir2(-1, 0);
    float offset = 0.4f;
    {
        RenderItemAttachmentPoint& attachmentPoint = _data.attachmentPoints.push();

        attachmentPoint.dir.copy(dir1);
        attachmentPoint.p0.set(0, 0);
        attachmentPoint.p1.lineCombin(pos, dir1, offset);
        attachmentPoint.color = CWC_BASE;
        attachmentPoint.highlighted = false;
        attachmentPoint.number = 1;
    }
    {
        RenderItemAttachmentPoint& attachmentPoint = _data.attachmentPoints.push();

        attachmentPoint.dir.copy(dir2);
        attachmentPoint.p0.set(0, 0);
        attachmentPoint.p1.lineCombin(pos, dir2, offset);
        attachmentPoint.color = CWC_BASE;
        attachmentPoint.highlighted = false;
        attachmentPoint.number = 2;
    }
    _cw.setSingleSource(CWC_BASE);
    for (int i = 0; i < attachmentPointCount; ++i)
        _cw.drawAttachmentPoint(_data.attachmentPoints[attachmentPointBegin + i], _idle);
}

void MoleculeRenderInternal::_renderLabels()
{
    for (int i = _mol->vertexBegin(); i < _mol->vertexEnd(); i = _mol->vertexNext(i))
        _drawAtom(_ad(i));
}

void MoleculeRenderInternal::_renderRings()
{
    for (int i = 0; i < _data.rings.size(); ++i)
    {
        const Ring& ring = _data.rings[i];
        if (ring.aromatic)
        {
            float r = 0.75f * ring.radius;
            for (int k = 0; k < ring.bondEnds.size(); ++k)
            {
                BondEnd& be = _be(ring.bondEnds[k]);
                if (_edgeIsHighlighted(be.bid))
                    _cw.setHighlight();
                float a0 = ring.angles[k], a1 = ring.angles[(k + 1) % ring.bondEnds.size()];
                if (fabs(a1 - a0) > M_PI)
                    _cw.drawArc(ring.center, r, std::max(a0, a1), std::min(a0, a1));
                else
                    _cw.drawArc(ring.center, r, std::min(a0, a1), std::max(a0, a1));
                if (_edgeIsHighlighted(be.bid))
                    _cw.resetHighlight();
            }
        }
    }
}

void MoleculeRenderInternal::_renderBonds()
{
    for (int i = _mol->edgeBegin(); i < _mol->edgeEnd(); i = _mol->edgeNext(i))
        _drawBond(i);
}

void MoleculeRenderInternal::_renderSGroups()
{
    for (int i = 0; i < _data.sgroups.size(); ++i)
    {
        const Sgroup& sg = _data.sgroups[i];
        for (int j = 0; j < sg.ticount; ++j)
            _cw.drawTextItemText(_data.textitems[j + sg.tibegin], _idle);
        for (int j = 0; j < sg.gicount; ++j)
            _cw.drawGraphItem(_data.graphitems[j + sg.gibegin]);
        for (int j = 0; j < sg.bicount; ++j)
            _cw.drawBracket(_data.brackets[j + sg.bibegin]);
    }

    /*
     * Additional fix for #44
     * To avoid overlapping of sgroups brackets with atom labels,
     * we draw a rectangle filled with background color at every
     * atom label position
     */
    if (_data.sgroups.size() > 0)
    {
        for (int i = _mol->vertexBegin(); i < _mol->vertexEnd(); i = _mol->vertexNext(i))
        {
            const AtomDesc& desc = _ad(i);
            for (int i = 0; i < desc.ticount; ++i)
            {
                const TextItem& ti = _data.textitems[i + desc.tibegin];
                _cw.drawItemBackground(ti);
            }
        }
    }
}

void MoleculeRenderInternal::_applyBondOffset()
{
    for (int i = 0; i < _data.bondends.size(); ++i)
    {
        BondEnd& be = _be(i);
        be.p.addScaled(be.dir, be.offset);
    }
}

void MoleculeRenderInternal::_setBondCenter()
{
    // find bond center
    for (int i = _mol->edgeBegin(); i < _mol->edgeEnd(); i = _mol->edgeNext(i))
    {
        BondDescr& bd = _bd(i);
        bd.center.lineCombin2(_be(bd.be1).p, 0.5f, _be(bd.be2).p, 0.5f);
    }
}

void MoleculeRenderInternal::_findNeighbors()
{
    // prepare bond end data
    for (int i = _mol->vertexBegin(); i < _mol->vertexEnd(); i = _mol->vertexNext(i))
    {
        const Vertex& vertex = _mol->getVertex(i);
        for (int j = vertex.neiBegin(); j < vertex.neiEnd(); j = vertex.neiNext(j))
        {
            int be1idx = _getBondEndIdx(i, j);
            BondEnd& be1 = _be(be1idx);
            if (vertex.degree() > 1)
            {
                // find right and left neighbor of each bond end
                for (int k = vertex.neiBegin(); k < vertex.neiEnd(); k = vertex.neiNext(k))
                {
                    if (k == j)
                        continue;
                    int be2idx = _getBondEndIdx(i, k);
                    BondEnd& be2 = _be(be2idx);
                    float dot = Vec2f::dot(be1.dir, be2.dir);
                    float cross = -Vec2f::cross(be1.dir, be2.dir);
                    float angle = atan2(cross, dot);
                    if (angle < 0)
                        angle += 2 * (float)M_PI;
                    float rAngle = (2 * (float)M_PI - angle);
                    if (be1.lnei < 0 || angle < be1.lang)
                    {
                        be1.lnei = be2idx;
                        be1.lsin = cross;
                        be1.lcos = dot;
                        be1.lang = angle;
                    }
                    if (be1.rnei < 0 || rAngle < be1.rang)
                    {
                        be1.rnei = be2idx;
                        be1.rsin = sin(rAngle);
                        be1.rcos = cos(rAngle);
                        be1.rang = rAngle;
                    }
                }
                int prev = _getOpposite(be1.lnei);
                _be(prev).next = be1idx;
            }
            else if (vertex.degree() == 1)
            {
                int prev = _getOpposite(be1idx);
                _be(prev).next = be1idx;
                be1.lnei = be1.rnei = be1idx;
                be1.lsin = be1.rsin = 0;
                be1.lcos = be1.rcos = 1;
                be1.lang = be1.rang = (float)(2 * M_PI);
            }
        }
    }
}

void MoleculeRenderInternal::_findCenteredCase()
{
    // bonds with both labels visible
    for (int i = _mol->edgeBegin(); i < _mol->edgeEnd(); i = _mol->edgeNext(i))
    {
        BondEnd& be1 = _be(_bd(i).be1);
        BondEnd& be2 = _be(_bd(i).be2);
        if (_ad(be1.aid).showLabel && _ad(be2.aid).showLabel)
            be1.centered = be2.centered = true;
    }

    // other cases
    for (int i = _mol->vertexBegin(); i < _mol->vertexEnd(); i = _mol->vertexNext(i))
    {
        const Vertex& vertex = _mol->getVertex(i);
        const AtomDesc& ad = _ad(i);
        for (int j = vertex.neiBegin(); j < vertex.neiEnd(); j = vertex.neiNext(j))
        {
            if (_bd(vertex.neiEdge(j)).inRing)
                continue;
            BondEnd& be = _getBondEnd(i, j);
            if (ad.showLabel)
            {
                be.centered = true;
                continue; // prolongation is not required when label is shown
            }

            if (be.lnei == be.rnei) // zero or one neighbor
                continue;
            if (be.lsin < _settings.prolongAdjSinTreshold || be.rsin < _settings.prolongAdjSinTreshold) // angle should not be to large or too small
                continue;
            const BondDescr& bdl = _bd(_be(be.lnei).bid);
            const BondDescr& bdr = _bd(_be(be.rnei).bid);

            if (((bdl.type == BOND_SINGLE && (bdl.stereodir == 0 || _opt.centerDoubleBondWhenStereoAdjacent)) &&
                 (bdr.type == BOND_SINGLE && (bdr.stereodir == 0 || _opt.centerDoubleBondWhenStereoAdjacent))) ||
                (bdr.type == BOND_SINGLE && bdl.type == BOND_SINGLE && bdl.stereodir != 0 && bdr.stereodir != 0))
            {
                be.centered = true;
                be.prolong = true;
            }
        }
    }

    for (int i = _mol->edgeBegin(); i < _mol->edgeEnd(); i = _mol->edgeNext(i))
    {
        BondDescr& bd = _bd(i);
        bd.centered = _be(bd.be1).centered && _be(bd.be2).centered;
    }
}

float MoleculeRenderInternal::_getBondOffset(int aid, const Vec2f& pos, const Vec2f& dir, const float bondWidth)
{
    if (!_ad(aid).showLabel)
        return -1;

    float maxOffset = 0, offset = 0;
    for (int k = 0; k < _ad(aid).ticount; ++k)
    {
        TextItem& item = _data.textitems[_ad(aid).tibegin + k];
        if (item.noBondOffset)
            continue;
        if (_clipRayBox(offset, pos, dir, item.bbp, item.bbsz, bondWidth))
            maxOffset = std::max(maxOffset, offset);
    }
    for (int k = 0; k < _ad(aid).gicount; ++k)
    {
        GraphItem& item = _data.graphitems[_ad(aid).gibegin + k];
        if (item.noBondOffset)
            continue;
        if (_clipRayBox(offset, pos, dir, item.bbp, item.bbsz, bondWidth))
            maxOffset = std::max(maxOffset, offset);
    }
    return maxOffset + _settings.unit * 2;
}

void MoleculeRenderInternal::_calculateBondOffset()
{
    // calculate offset for bonds
    for (int i = _mol->vertexBegin(); i < _mol->vertexEnd(); i = _mol->vertexNext(i))
    {
        const Vertex& vertex = _mol->getVertex(i);
        for (int j = vertex.neiBegin(); j < vertex.neiEnd(); j = vertex.neiNext(j))
        {
            BondEnd& be1 = _getBondEnd(i, j);
            be1.offset = std::max(be1.offset, _getBondOffset(i, be1.p, be1.dir, be1.width));
        }
    }

    for (int i = _mol->edgeBegin(); i < _mol->edgeEnd(); i = _mol->edgeNext(i))
    {
        const BondDescr& bd = _bd(i);
        BondEnd& be1 = _be(bd.be1);
        BondEnd& be2 = _be(bd.be2);

        // if the offsets may result in too short bond,
        float offsum = be1.offset + be2.offset;
        if (offsum > 1e-3f && bd.length < offsum + _settings.minBondLength)
        {
            // if possible, scale down the offsets so that the bond has at least the specified length
            if (bd.length > _settings.minBondLength)
            {
                float factor = (bd.length - _settings.minBondLength) / offsum;
                be1.offset *= factor;
                be2.offset *= factor;
            }
            // otherwise, ignore offsets (extreme case)
            else
            {
                be1.offset = be2.offset = 0;
            }
        }
    }
}

void MoleculeRenderInternal::_initBondData()
{
    float thicknessHighlighted = _cw.highlightedBondLineWidth();

    for (int i = _mol->edgeBegin(); i < _mol->edgeEnd(); i = _mol->edgeNext(i))
    {
        BondDescr& d = _bd(i);
        d.type = _mol->getBondOrder(i);
        d.thickness = _edgeIsHighlighted(i) ? thicknessHighlighted : _settings.bondLineWidth;
        d.queryType = -1;
        QUERY_MOL_BEGIN(_mol);
        {
            QueryMolecule::Bond& qb = qmol.getBond(i);
            d.queryType = QueryMolecule::getQueryBondType(qb);
            d.stereoCare = qmol.bondStereoCare(i);
            if (qb.hasConstraint(QueryMolecule::BOND_TOPOLOGY))
            {
                bool chainPossible = qb.possibleValue(QueryMolecule::BOND_TOPOLOGY, TOPOLOGY_CHAIN);
                bool ringPossible = qb.possibleValue(QueryMolecule::BOND_TOPOLOGY, TOPOLOGY_RING);
                d.topology = 0;
                if (chainPossible && !ringPossible)
                {
                    d.topology = TOPOLOGY_CHAIN;
                }
                if (ringPossible && !chainPossible)
                {
                    d.topology = TOPOLOGY_RING;
                }
            }
        }
        QUERY_MOL_END;
        const Edge& edge = _mol->getEdge(i);
        d.beg = edge.beg;
        d.end = edge.end;
        d.vb = _ad(d.beg).pos;
        d.ve = _ad(d.end).pos;
        d.dir.diff(d.ve, d.vb);
        d.length = d.dir.length();
        d.dir.normalize();
        d.norm.set(-d.dir.y, d.dir.x);
        d.isShort = d.length < (_settings.bondSpace + _settings.bondLineWidth) * 2; // TODO: check

        d.stereodir = _mol->getBondDirection(i);
        d.cistrans = _mol->cis_trans.isIgnored(i);
        int ubid = _bondMappingInv.size() > i ? _bondMappingInv.at(i) : i;
        if (_data.reactingCenters.size() > ubid)
            d.reactingCenter = _data.reactingCenters[ubid];
    }
}

void MoleculeRenderInternal::_initBoldStereoBonds()
{
    if (!_opt.boldBondDetection)
        return;
    for (int i = _mol->edgeBegin(); i < _mol->edgeEnd(); i = _mol->edgeNext(i))
    {
        BondDescr& d = _bd(i);
        const Vertex& v1 = _mol->getVertex(d.beg);
        const Vertex& v2 = _mol->getVertex(d.end);
        bool hasNeighboringUpBond1 = false;
        for (int j = v1.neiBegin(); j < v1.neiEnd(); j = v1.neiNext(j))
            if (v1.neiEdge(j) != i && _bd(v1.neiEdge(j)).stereodir == BOND_UP && _bd(v1.neiEdge(j)).end == d.beg)
                hasNeighboringUpBond1 = true;
        bool hasNeighboringUpBond2 = false;
        for (int j = v2.neiBegin(); j < v2.neiEnd(); j = v2.neiNext(j))
            if (v2.neiEdge(j) != i && _bd(v2.neiEdge(j)).stereodir == BOND_UP && _bd(v2.neiEdge(j)).end == d.end)
                hasNeighboringUpBond2 = true;
        if (hasNeighboringUpBond1 && hasNeighboringUpBond2)
            d.stereodir = BOND_STEREO_BOLD;
    }
}

void MoleculeRenderInternal::_initBondEndData()
{
    for (int i = _mol->edgeBegin(); i < _mol->edgeEnd(); i = _mol->edgeNext(i))
    {
        const Edge& edge = _mol->getEdge(i);
        const AtomDesc& bdesc = _ad(edge.beg);
        const AtomDesc& edesc = _ad(edge.end);

        BondDescr& bondd = _bd(i);

        bondd.be1 = _data.bondends.size();
        _data.bondends.push();
        bondd.be2 = _data.bondends.size();
        _data.bondends.push();

        BondEnd& be1 = _be(bondd.be1);
        BondEnd& be2 = _be(bondd.be2);
        be1.clear();
        be2.clear();
        be1.bid = be2.bid = i;
        be1.aid = edge.beg;
        be1.dir.diff(edesc.pos, bdesc.pos);
        be1.dir.normalize();
        be1.lnorm.copy(be1.dir);
        be1.lnorm.rotate(1, 0);
        be1.p.copy(bdesc.pos);

        be2.aid = edge.end;
        be2.dir.negation(be1.dir);
        be2.lnorm.negation(be1.lnorm);
        be2.p.copy(edesc.pos);
    }

    for (int i = 0; i < _data.bondends.size(); ++i)
    {
        BondEnd& be = _be(i);
        const BondDescr& bd = _bd(be.bid);
        if (bd.type == BOND_SINGLE)
            if (bd.stereodir != 0)
                if (bd.be1 == i) // is source end?
                    be.width = 0;
                else
                    be.width = 2 * (_settings.bondSpace + _settings.bondLineWidth);
            else
                be.width = 2 * _settings.bondSpace + _settings.bondLineWidth;
        else if (bd.type == BOND_DOUBLE || bd.type == BOND_AROMATIC || bd.type == BOND_TRIPLE || bd.queryType >= 0)
            be.width = 4 * _settings.bondSpace + _settings.bondLineWidth;
        else if (bd.type == _BOND_HYDROGEN || bd.type == _BOND_COORDINATION)
            be.width = 2 * (_settings.bondSpace + _settings.bondLineWidth);
        else
        {
            Array<char> buf;
            _mol->getBondDescription(be.bid, buf);
            throw Error("Unknown bond type %s. Can not determine bond width.", buf.ptr());
        }
    }
}

void MoleculeRenderInternal::_extendRenderItems()
{
    for (int i = 0; i < _data.textitems.size(); ++i)
        _extendRenderItem(_data.textitems[i], _settings.boundExtent);
    for (int i = 0; i < _data.graphitems.size(); ++i)
        _extendRenderItem(_data.graphitems[i], _settings.boundExtent);
}

BondEnd& MoleculeRenderInternal::_getBondEnd(int aid, int nei)
{
    return _be(_getBondEndIdx(aid, nei));
}

int MoleculeRenderInternal::_getBondEndIdx(int aid, int nei)
{
    int ne = _mol->getVertex(aid).neiEdge(nei);
    int be = _bd(ne).getBondEnd(aid);
    return be;
}

void MoleculeRenderInternal::_drawAtom(const AtomDesc& desc)
{
#ifdef RENDER_SHOW_BACKGROUND
    for (int i = 0; i < desc.ticount; ++i)
        _cw.drawItemBackground(_data.textitems[i + desc.tibegin]);
    for (int i = 0; i < desc.gicount; ++i)
        _cw.drawItemBackground(_data.graphitems[i + desc.gibegin]);
#endif

    _cw.setSingleSource(desc.color);
    for (int i = 0; i < desc.ticount; ++i)
    {
        if (desc.hcolorSet)
            _cw.drawTextItemText(_data.textitems[i + desc.tibegin], desc.hcolor, _idle);
        else
            _cw.drawTextItemText(_data.textitems[i + desc.tibegin], _idle);
    }
    for (int i = 0; i < desc.attachmentPointCount; ++i)
        _cw.drawAttachmentPoint(_data.attachmentPoints[desc.attachmentPointBegin + i], _idle);
    for (int i = 0; i < desc.rSiteAttachmentIndexCount; ++i)
        _cw.drawRSiteAttachmentIndex(_data.rSiteAttachmentIndices[desc.rSiteAttachmentIndexBegin + i]);
    for (int i = 0; i < desc.gicount; ++i)
    {
        if (desc.hcolorSet)
            _cw.drawGraphItem(_data.graphitems[i + desc.gibegin], desc.hcolor);
        else
            _cw.drawGraphItem(_data.graphitems[i + desc.gibegin]);
    }
}

void MoleculeRenderInternal::_writeQueryAtomToString(Output& output, int aid)
{
    BaseMolecule& bm = *_mol;
    AtomDesc& ad = _ad(aid);

    if (bm.isRSite(aid))
    {
        QS_DEF(Array<int>, rg);
        bm.getAllowedRGroups(aid, rg);

        if (rg.size() == 0)
            output.printf("R");
        else
            for (int i = 0; i < rg.size(); ++i)
            {
                if (i > 0)
                    output.printf(",");
                output.printf("R%i", rg[i]);
            }
    }
    else
    {
        if (!bm.isQueryMolecule())
            throw Error("Atom type %d not supported in non-queries", ad.queryLabel);

        if (ad.queryLabel == QueryMolecule::QUERY_ATOM_A)
        {
            output.printf("A");
        }
        else if (ad.queryLabel == QueryMolecule::QUERY_ATOM_X)
        {
            output.printf("X");
        }
        else if (ad.queryLabel == QueryMolecule::QUERY_ATOM_Q)
        {
            output.printf("Q");
        }
        else if (ad.queryLabel == QueryMolecule::QUERY_ATOM_LIST || ad.queryLabel == QueryMolecule::QUERY_ATOM_NOTLIST)
        {
            QueryMolecule& qm = bm.asQueryMolecule();
            QueryMolecule::Atom& qa = qm.getAtom(aid);
            QueryMolecule::writeSmartsAtom(output, &qa, -1, -1, 0, false, false, bm.original_format);
        }
        else
        {
            throw Error("Query atom type %d not supported", ad.queryLabel);
        }
    }
}

bool MoleculeRenderInternal::_writeDelimiter(bool needDelimiter, Output& output)
{
    if (needDelimiter)
        output.printf(",");
    else
        output.printf("(");
    return true;
}

QueryMolecule::Atom* atomNodeInConjunction(QueryMolecule::Atom& qa, int type)
{
    if (qa.type != QueryMolecule::OP_AND)
        return NULL;
    for (int i = 0; i < qa.children.size(); ++i)
        if (qa.child(i)->type == type)
            return qa.child(i);
    return NULL;
}

void MoleculeRenderInternal::_writeQueryModifier(Output& output, int aid)
{
    QUERY_MOL_BEGIN(_mol);
    {
        bool needDelimiter = false;
        QueryMolecule::Atom& qa = qmol.getAtom(aid);

        if (qa.hasConstraint(QueryMolecule::ATOM_SUBSTITUENTS))
        {
            int subst = qmol.getAtomSubstCount(aid);
            needDelimiter = _writeDelimiter(needDelimiter, output);
            if (subst >= 0)
                output.printf("s%i", subst);
        }

        if (qa.hasConstraint(QueryMolecule::ATOM_SUBSTITUENTS_AS_DRAWN))
        {
            needDelimiter = _writeDelimiter(needDelimiter, output);
            output.printf("s*");
        }

        if (qa.hasConstraint(QueryMolecule::ATOM_RING_BONDS))
        {
            int ringBondCount = qmol.getAtomRingBondsCount(aid);
            needDelimiter = _writeDelimiter(needDelimiter, output);
            if (ringBondCount >= 0)
                output.printf("rb%i", ringBondCount);
        }

        if (qa.hasConstraint(QueryMolecule::ATOM_RING_BONDS_AS_DRAWN))
        {
            needDelimiter = _writeDelimiter(needDelimiter, output);
            output.printf("rb*");
        }

        if (qa.hasConstraint(QueryMolecule::ATOM_UNSATURATION))
        {
            needDelimiter = _writeDelimiter(needDelimiter, output);
            output.printf("u");
        }

        if (qa.hasConstraint(QueryMolecule::ATOM_TOTAL_H))
        {
            QueryMolecule::Atom* qc = atomNodeInConjunction(qa, QueryMolecule::ATOM_TOTAL_H);
            if (qc != NULL)
            {
                int totalH = qc->value_min;
                needDelimiter = _writeDelimiter(needDelimiter, output);
                output.printf("H%i", totalH);
            }
        }

        if (_ad(aid).fixed)
        {
            needDelimiter = _writeDelimiter(needDelimiter, output);
            output.printf("f");
        }

        if (needDelimiter)
            output.printf(")");

        if (_ad(aid).exactChange)
        {
            output.printf(".ext.");
        }
    }
    QUERY_MOL_END;
}

static void _expandBoundRect(AtomDesc& ad, const RenderItem& item)
{
    Vec2f min, max;
    min.diff(item.bbp, ad.pos);
    max.sum(min, item.bbsz);
    ad.boundBoxMin.min(min);
    ad.boundBoxMax.max(max);
}

int MoleculeRenderInternal::_findClosestBox(Vec2f& p, int aid, const Vec2f& sz, float mrg, int skip)
{
    const Vertex& vertex = _mol->getVertex(aid);
    const AtomDesc& ad = _ad(aid);
    float w2 = sz.x / 2 + mrg;
    float h2 = sz.y / 2 + mrg;

    if (vertex.degree() == 0)
    {
        p.set(0, -h2);
        p.add(ad.pos);
        return -1;
    }
    if (vertex.degree() == 1)
    {
        float offset = 0;
        const Vec2f& d = _getBondEnd(aid, vertex.neiBegin()).dir;
        float val = -1, dx = fabs(d.x), dy = fabs(d.y);
        if (dx > 0.01)
            if (val < 0 || val > w2 / dx)
                val = w2 / dx;
        if (dy > 0.01)
            if (val < 0 || val > h2 / dy)
                val = h2 / dy;
        if (val > offset)
            offset = val;
        p.lineCombin(ad.pos, d, -offset);
        return vertex.neiBegin();
    }
    int iMin = -1;
    for (int i = vertex.neiBegin(); i < vertex.neiEnd(); i = vertex.neiNext(i))
    {
        if (skip == i)
            continue;
        Vec2f q;
        const BondEnd& lbe = _getBondEnd(aid, i);
        const BondEnd& rbe = _be(lbe.rnei);
        const BondDescr& lbd = _bd(lbe.bid);
        const BondDescr& rdb = _bd(rbe.bid);

        // correction of positioning according to bond types
        float turn = atan(_settings.bondSpace / lbd.length);
        float leftShift = 0, rightShift = 0;
        bool leftBondCentered = _be(lbd.be1).centered && _be(lbd.be2).centered;
        bool leftTurn = false, rightTurn = false;
        bool leftIsSourceEnd = (lbd.beg == aid);
        bool leftBondOrientedInwards = (leftIsSourceEnd == lbd.lineOnTheRight);
        bool leftIsStereo = lbd.stereodir > 0;
        bool rightBondCentered = _be(rdb.be1).centered && _be(rdb.be2).centered;
        bool rightIsSourceEnd = (rdb.beg == aid);
        bool rightIsStereo = rdb.stereodir > 0;
        bool rightBondOrientedInwards = (rightIsSourceEnd == !rdb.lineOnTheRight);

        if (lbd.type == BOND_DOUBLE)
        {
            if (leftBondCentered)
                leftShift = _settings.bondSpace;
            else if (leftBondOrientedInwards)
                leftShift = 2 * _settings.bondSpace;
        }
        else if (lbd.type == BOND_TRIPLE)
            leftShift = 2 * _settings.bondSpace;
        else if (leftIsStereo)
        {
            if (leftIsSourceEnd)
                leftTurn = true;
            else
                leftShift = _settings.bondSpace;
        }

        if (rdb.type == BOND_DOUBLE)
        {
            if (rightBondCentered)
                rightShift = _settings.bondSpace;
            else if (rightBondOrientedInwards)
                rightShift = 2 * _settings.bondSpace;
        }
        else if (rdb.type == BOND_TRIPLE)
            rightShift = 2 * _settings.bondSpace;
        else if (rightIsStereo)
        {
            if (rightIsSourceEnd)
                rightTurn = true;
            else
                rightShift = _settings.bondSpace;
        }

        Vec2f leftDir, rightDir;
        leftDir.copy(lbe.dir);
        rightDir.copy(rbe.dir);
        if (leftTurn)
            leftDir.rotateL(turn);
        if (rightTurn)
            rightDir.rotateL(-turn);
        Vec2f origin;
        float si = Vec2f::cross(leftDir, rightDir);
        float co = Vec2f::dot(leftDir, rightDir);
        float ang = atan2(si, co);
        if (ang < 0)
            ang += 2 * (float)M_PI;

        float factor = fabs(sin(ang / 2));
        Vec2f rightNorm(rightDir), leftNorm(leftDir);
        rightNorm.rotateL((float)M_PI / 2);
        leftNorm.rotateL(-(float)M_PI / 2);

        float rightOffset = 0, leftOffset = 0;
        rightOffset = w2 * fabs(leftNorm.x) + h2 * fabs(leftNorm.y);
        leftOffset = w2 * fabs(rightNorm.x) + h2 * fabs(rightNorm.y);

        float t = std::max(leftShift + rightOffset, leftOffset + rightShift) / factor;
        Vec2f median(rightDir);
        median.rotateL(ang / 2);
        q.addScaled(median, t);
        if (iMin < 0 || q.lengthSqr() < p.lengthSqr())
        {
            iMin = i;
            p.copy(q);
        }
    }
    p.add(ad.pos);
    return iMin;
}

int MoleculeRenderInternal::_findClosestCircle(Vec2f& p, int aid, float radius, int skip)
{
    const Vertex& vertex = _mol->getVertex(aid);
    const AtomDesc& ad = _ad(aid);
    if (vertex.degree() == 0)
    {
        p.copy(ad.pos);
        p.y -= radius;
        return -1;
    }
    if (vertex.degree() == 1)
    {
        p.lineCombin(ad.pos, _getBondEnd(aid, vertex.neiBegin()).dir, -radius);
        return vertex.neiBegin();
    }
    int iMin = -1;
    for (int i = vertex.neiBegin(); i < vertex.neiEnd(); i = vertex.neiNext(i))
    {
        if (skip == i)
            continue;
        Vec2f q;
        const BondEnd& rbe = _be(i);
        const BondEnd& lbe = _be(rbe.lnei);
        ;
        const BondDescr& rbd = _bd(rbe.bid);
        const BondDescr& lbd = _bd(lbe.bid);

        // correction of positioning according to bond types
        float turn = atan(_settings.bondSpace / rbd.length);
        float rightShift = 0, leftShift = 0;
        bool rightBondCentered = _be(rbd.be1).centered && _be(rbd.be2).centered;
        bool rightTurn = false, leftTurn = false;
        bool rightIsSourceEnd = rbd.beg == aid;
        bool rightBondOrientationLeft = !rbd.lineOnTheRight;
        bool rightBondOrientedInwards = (rightIsSourceEnd && rightBondOrientationLeft) || (!rightIsSourceEnd && !rightBondOrientationLeft);
        bool rightIsStereo = rbd.stereodir > 0;
        bool leftBondCentered = _getBondEnd(aid, lbd.be1).centered && _getBondEnd(aid, lbd.be2).centered;
        bool leftIsSourceEnd = lbd.beg == aid;
        bool leftIsStereo = lbd.stereodir > 0;
        bool leftBondOrientationRight = lbd.lineOnTheRight;
        bool leftBondOrientedInwards = (leftIsSourceEnd && leftBondOrientationRight) || (!leftIsSourceEnd && !leftBondOrientationRight);

        if (rbd.type == BOND_DOUBLE)
        {
            if (rightBondCentered)
                rightShift = _settings.bondSpace;
            else if (rightBondOrientedInwards)
                rightShift = 2 * _settings.bondSpace;
        }
        else if (rbd.type == BOND_TRIPLE)
            rightShift = 2 * _settings.bondSpace;
        else if (rightIsStereo)
        {
            if (rightIsSourceEnd)
                rightTurn = true;
            else
                rightShift = _settings.bondSpace;
        }

        if (lbd.type == BOND_DOUBLE)
        {
            if (leftBondCentered)
                leftShift = _settings.bondSpace;
            else if (leftBondOrientedInwards)
                leftShift = 2 * _settings.bondSpace;
        }
        else if (lbd.type == BOND_TRIPLE)
            leftShift = 2 * _settings.bondSpace;
        else if (leftIsStereo)
        {
            if (leftIsSourceEnd)
                leftTurn = true;
            else
                leftShift = _settings.bondSpace;
        }

        Vec2f rightDir, leftDir;
        rightDir.copy(rbe.dir);
        leftDir.copy(lbe.dir);
        if (rightTurn)
            rightDir.rotate(turn);
        if (leftTurn)
            leftDir.rotate(-turn);
        Vec2f origin;
        float si = Vec2f::cross(rightDir, leftDir);
        float co = Vec2f::dot(rightDir, leftDir);
        float ang = atan2(si, co);
        if (ang < 0)
            ang += (float)(2 * M_PI);

        float factor = std::max(fabsf(si), 0.01f);
        if (rightShift > 0)
            origin.addScaled(leftDir, rightShift / factor);
        if (leftShift > 0)
            origin.addScaled(rightDir, leftShift / factor);

        q.copy(rightDir);
        q.rotate(ang / 2);

        float dst = radius / (float)sin2c(Vec2f::dot(rightDir, leftDir));
        q.scale(dst);
        q.add(origin);

        if (iMin < 0 || q.lengthSqr() < p.lengthSqr())
        {
            iMin = i;
            p.copy(q);
        }
        q.add(ad.pos);
        //_cw.setLineWidth(_settings.unit);
        //_cw.setSingleSource(CWC_BLUE);
        //_cw.drawCircle(q, radius);
    }
    p.add(ad.pos);
    return iMin;
}

enum CHARCAT
{
    DIGIT = 0,
    LETTER,
    TAG_SUPERSCRIPT,
    TAG_SUBSCRIPT,
    TAG_NORMAL,
    SIGN,
    WHITESPACE
};
enum SCRIPT
{
    MAIN,
    SUB,
    SUPER
};

void MoleculeRenderInternal::_preparePseudoAtom(int aid, int color, bool highlighted)
{
    AtomDesc& ad = _ad(aid);
    const char* str = ad.pseudo.ptr();

    int cnt = 0;
    CHARCAT a = WHITESPACE, b = WHITESPACE;
    int i0 = 0, i1;
    SCRIPT script = MAIN, newscript = MAIN;
    int len = (int)strlen(str);
    GraphItem::TYPE signType;
    // TODO: replace remembering item ids and shifting each of them with single offset value for an atom
    Array<int> tis, gis;

    TextItem fake;
    fake.fontsize = FONT_SIZE_LABEL;
    fake.text.push('C');
    fake.text.push((char)0);
    _cw.setTextItemSize(fake, ad.pos);
    float xpos = fake.bbp.x, width = fake.bbsz.x, offset = _settings.unit / 2, totalwdt = 0, upshift = -0.6f, downshift = 0.2f, space = width / 2;
    ad.ypos = fake.bbp.y;
    ad.height = fake.bbsz.y;
    ad.leftMargin = ad.rightMargin = xpos;

    if (ad.pseudoAtomStringVerbose)
    {
        int id = _pushTextItem(ad, RenderItem::RIT_PSEUDO, color, highlighted);
        tis.push(id);
        TextItem& item = _data.textitems[id];
        item.fontsize = FONT_SIZE_ATTR;
        item.text.copy(str, len);
        item.text.push((char)0);
        _cw.setTextItemSize(item);
        item.bbp.set(xpos, ad.ypos);
        _expandBoundRect(ad, item);
        totalwdt += item.bbsz.x;
    }
    else
    {
        for (int i = 0; i <= len; ++i)
        {
            i1 = i;
            a = b;

            bool tag = false;
            {
                char c = (i == len ? ' ' : str[i]);
                if (isspace(c))
                {
                    b = WHITESPACE;
                }
                else if (isdigit(c))
                {
                    b = DIGIT;
                }
                else if (c == '+' || c == '-')
                {
                    b = SIGN, signType = ((c == '+') ? GraphItem::PLUS : GraphItem::MINUS);
                }
                else if (c == '\\' && i < len - 1 && str[i + 1] == 'S')
                {
                    b = TAG_SUPERSCRIPT, ++i, tag = true;
                }
                else if (c == '\\' && i < len - 1 && str[i + 1] == 's')
                {
                    b = TAG_SUBSCRIPT, ++i, tag = true;
                }
                else if (c == '\\' && i < len - 1 && str[i + 1] == 'n')
                {
                    b = TAG_NORMAL, ++i, tag = true;
                }
                else
                {
                    b = LETTER;
                }
            }

            {
                bool aTag = a == TAG_SUPERSCRIPT || a == TAG_SUBSCRIPT || a == TAG_NORMAL;
                if (b == TAG_SUPERSCRIPT)
                {
                    newscript = SUPER;
                }
                else if (b == TAG_SUBSCRIPT)
                {
                    newscript = SUB;
                }
                else if (b == TAG_NORMAL)
                {
                    newscript = MAIN;
                }
                else if ((b == WHITESPACE && a != WHITESPACE) || (b != WHITESPACE && a == WHITESPACE) || (b == LETTER && !aTag))
                {
                    newscript = MAIN;
                }
                else if (b == DIGIT && a == SIGN)
                {
                    newscript = script;
                }
                else if (b == DIGIT && a != DIGIT && !aTag)
                {
                    newscript = ((a == LETTER) ? SUB : MAIN);
                }
                else if (b == SIGN)
                {
                    if (a == LETTER || a == DIGIT)
                    {
                        newscript = SUPER;
                    }
                }
                else if (a == SIGN && script == SUPER)
                {
                    newscript = MAIN;
                }
                else
                {
                    continue;
                }
            }

            if (i1 > i0)
            {
                if (a == SIGN && script == SUPER)
                {
                    int id = _pushGraphItem(ad, RenderItem::RIT_CHARGESIGN, color, highlighted);
                    gis.push(id);
                    GraphItem& sign = _data.graphitems[id];
                    _cw.setGraphItemSizeSign(sign, signType);

                    totalwdt += offset;
                    sign.bbp.set(xpos + totalwdt, ad.ypos + ad.height - sign.bbsz.y + upshift * ad.height);
                    _expandBoundRect(ad, sign);
                    totalwdt += sign.bbsz.x;
                }
                else if (a == WHITESPACE)
                {
                    totalwdt += space * (i1 - i0);
                }
                else
                {
                    float shift = (script == SUB) ? downshift : ((script == SUPER) ? upshift : 0);
                    int id = _pushTextItem(ad, RenderItem::RIT_PSEUDO, color, highlighted);
                    tis.push(id);
                    TextItem& item = _data.textitems[id];
                    item.fontsize = (script == MAIN) ? FONT_SIZE_LABEL : FONT_SIZE_ATTR;
                    item.text.copy(str + i0, i1 - i0);
                    item.text.push((char)0);
                    _cw.setTextItemSize(item);

                    if (cnt > 0)
                        totalwdt += offset;
                    item.bbp.set(xpos + totalwdt, ad.ypos + ad.height - item.relpos.y + shift * ad.height);
                    _expandBoundRect(ad, item);
                    totalwdt += item.bbsz.x;
                }
                cnt++;
            }
            script = newscript;
            i0 = i + (tag ? 1 : 0);
        }
    }
    ad.rightMargin += totalwdt;
    if (ad.hydroPos == HYDRO_POS_LEFT)
    {
        float dx = totalwdt - width;
        for (int i = 0; i < tis.size(); ++i)
            _data.textitems[tis[i]].bbp.x -= dx;
        for (int i = 0; i < gis.size(); ++i)
            _data.graphitems[gis[i]].bbp.x -= dx;
        ad.leftMargin -= dx;
        ad.rightMargin -= dx;
    }
}

void MoleculeRenderInternal::_prepareChargeLabel(int aid, int color, bool highlighted)
{
    AtomDesc& ad = _ad(aid);
    BaseMolecule& bm = *_mol;

    int charge = bm.getAtomCharge(aid);
    if (charge != CHARGE_UNKNOWN && charge != 0)
    {
        ad.rightMargin += _settings.labelInternalOffset;
        if (abs(charge) != 1)
        {
            int tiChargeValue = _pushTextItem(ad, RenderItem::RIT_CHARGEVAL, color, highlighted);

            TextItem& itemChargeValue = _data.textitems[tiChargeValue];
            itemChargeValue.fontsize = FONT_SIZE_ATTR;
            bprintf(itemChargeValue.text, "%i", abs(charge));
            _cw.setTextItemSize(itemChargeValue);

            itemChargeValue.bbp.set(ad.rightMargin, ad.ypos + _settings.upperIndexShift * ad.height);
            _expandBoundRect(ad, itemChargeValue);
            ad.rightMargin += itemChargeValue.bbsz.x;
        }

        GraphItem::TYPE type = charge > 0 ? GraphItem::PLUS : GraphItem::MINUS;
        int giChargeSign = _pushGraphItem(ad, RenderItem::RIT_CHARGESIGN, color, highlighted);

        GraphItem& itemChargeSign = _data.graphitems[giChargeSign];
        _cw.setGraphItemSizeSign(itemChargeSign, type);

        itemChargeSign.bbp.set(ad.rightMargin, ad.ypos + _settings.upperIndexShift * ad.height);
        _expandBoundRect(ad, itemChargeSign);
        ad.rightMargin += itemChargeSign.bbsz.x;
    }
}

void MoleculeRenderInternal::_prepareLabelText(int aid)
{
    AtomDesc& ad = _ad(aid);
    BaseMolecule& bm = *_mol;
    ad.boundBoxMin.set(0, 0);
    ad.boundBoxMax.set(0, 0);

    int color = ad.color;
    bool highlighted = _vertexIsHighlighted(aid);

    int tilabel = -1, tihydro = -1, tiHydroIndex = -1, tiChargeValue = -1, tiValence = -1, tiIsotope = -1, tiindex = -1;
    int giChargeSign = -1, giRadical = -1, giRadical1 = -1, giRadical2 = -1;
    ad.rightMargin = ad.leftMargin = ad.ypos = ad.height = 0;
    int isotope = bm.getAtomIsotope(aid);

    if (ad.type == AtomDesc::TYPE_PSEUDO)
    {
        _preparePseudoAtom(aid, CWC_BASE, highlighted);

        bool chargeSignAdded = false;
        for (auto i = 0; i < _data.graphitems.size(); i++)
        {
            if (_data.graphitems[i].ritype == RenderItem::RIT_CHARGESIGN)
            {
                chargeSignAdded = true;
                break;
            }
        }

        if (!chargeSignAdded)
        {
            _prepareChargeLabel(aid, color, highlighted);
        }
    }
    else if (ad.showLabel)
    {
        tilabel = _pushTextItem(ad, RenderItem::RIT_LABEL, color, highlighted);
        {
            TextItem& label = _data.textitems[tilabel];
            label.fontsize = FONT_SIZE_LABEL;
            ArrayOutput output(label.text);
            if (ad.type == AtomDesc::TYPE_REGULAR)
                if (ad.label == ELEM_H && isotope == DEUTERIUM)
                    output.printf("D");
                else if (ad.label == ELEM_H && isotope == TRITIUM)
                    output.printf("T");
                else
                    output.printf(Element::toString(ad.label));
            else if (ad.type == AtomDesc::TYPE_QUERY)
                _writeQueryAtomToString(output, aid);
            else
                throw Error("Neither label nor query atom type available");

            _writeQueryModifier(output, aid);
            output.writeChar(0);

            _cw.setTextItemSize(label, ad.pos);
            _expandBoundRect(ad, label);
            ad.rightMargin = label.bbp.x + label.bbsz.x;
            ad.leftMargin = label.bbp.x;
            ad.ypos = label.bbp.y;
            ad.height = label.bbsz.y;
        }

        if (bm.isRSite(aid) && bm.getVertex(aid).degree() > 1)
        {
            // show indices for attachment bonds
            const Vertex& v = bm.getVertex(aid);
            ad.rSiteAttachmentIndexBegin = _data.rSiteAttachmentIndices.size();

            // Check if there are indices for attachment bonds
            bool hasAttachmentIndex = false, hasNoAttachmentIndex = false;
            for (int k = v.neiBegin(), j = 0; k < v.neiEnd(); k = v.neiNext(k), ++j)
            {
                int apIdx = bm.getRSiteAttachmentPointByOrder(aid, j);
                hasAttachmentIndex |= (apIdx != -1);
                hasNoAttachmentIndex |= (apIdx == -1);
            }
            if (hasAttachmentIndex && hasNoAttachmentIndex)
                throw Error("RSite %d is invalid: some attachments indices are specified and some are not");
            if (hasNoAttachmentIndex)
                ad.rSiteAttachmentIndexCount = 0;
            else
            {
                ad.rSiteAttachmentIndexCount = v.degree();

                for (int k = v.neiBegin(), j = 0; k < v.neiEnd(); k = v.neiNext(k), ++j)
                {
                    int apIdx = bm.getRSiteAttachmentPointByOrder(aid, j);
                    int i = v.findNeiVertex(apIdx);
                    BondEnd& be = _getBondEnd(aid, i);
                    int tii = _pushTextItem(ad, RenderItem::RIT_ATTACHMENTPOINT, CWC_BASE, false);
                    TextItem& ti = _data.textitems[tii];
                    RenderItemRSiteAttachmentIndex& item = _data.rSiteAttachmentIndices.push();
                    item.number = j + 1;
                    item.radius = 0.7f * _settings.fzz[FONT_SIZE_RSITE_ATTACHMENT_INDEX];
                    item.bbsz.set(2 * item.radius, 2 * item.radius);
                    item.bbp = ad.pos;
                    item.color = CWC_BASE;
                    item.highlighted = false;
                    item.noBondOffset = true;
                    bprintf(ti.text, "%d", item.number);
                    ti.fontsize = FONT_SIZE_RSITE_ATTACHMENT_INDEX;
                    ti.noBondOffset = true;
                    _cw.setTextItemSize(ti, ad.pos);
                    TextItem& label = _data.textitems[tilabel];
                    // this is just an upper bound, it won't be used
                    float shift = item.bbsz.length() + label.bbsz.length();
                    // one of the next conditions should be satisfied
                    if (fabs(be.dir.x) > 1e-3)
                        shift = std::min(shift, (item.bbsz.x + label.bbsz.x) / 2.f / fabsf(be.dir.x));
                    if (fabs(be.dir.y) > 1e-3)
                        shift = std::min(shift, (item.bbsz.y + label.bbsz.y) / 2.f / fabsf(be.dir.y));
                    shift += _settings.unit;
                    item.bbp.addScaled(be.dir, shift);
                    ti.bbp.addScaled(be.dir, shift);
                    be.offset = shift + item.radius;
                }
            }
        }

        // isotope
        if (isotope > 0 && (ad.label != ELEM_H || isotope > 3 || isotope < 2))
        {
            tiIsotope = _pushTextItem(ad, RenderItem::RIT_ISOTOPE, color, highlighted);

            TextItem& itemIsotope = _data.textitems[tiIsotope];
            itemIsotope.fontsize = FONT_SIZE_ATTR;
            bprintf(itemIsotope.text, "%i", isotope);
            _cw.setTextItemSize(itemIsotope);

            ad.leftMargin -= _settings.labelInternalOffset + itemIsotope.bbsz.x;
            itemIsotope.bbp.set(ad.leftMargin, ad.ypos + _settings.upperIndexShift * ad.height);
            _expandBoundRect(ad, itemIsotope);
        }

        // hydrogen drawing
        ad.showHydro = false;
        if (!bm.isQueryMolecule())
        {
            int implicit_h = 0;

            if (!bm.isRSite(aid) && !bm.isPseudoAtom(aid) && !bm.isTemplateAtom(aid))
                implicit_h = bm.asMolecule().getImplicitH_NoThrow(aid, 0);

            if (implicit_h > 0 && _opt.implHVisible)
            {
                ad.showHydro = true;

                tihydro = _pushTextItem(ad, RenderItem::RIT_HYDROGEN, color, highlighted);
                Vec2f hydrogenGroupSz;
                {

                    TextItem& itemHydrogen = _data.textitems[tihydro];
                    itemHydrogen.fontsize = FONT_SIZE_LABEL;
                    bprintf(itemHydrogen.text, "H");
                    _cw.setTextItemSize(itemHydrogen, ad.pos);
                    hydrogenGroupSz.x = itemHydrogen.bbsz.x + _settings.labelInternalOffset;
                    hydrogenGroupSz.y = itemHydrogen.bbsz.y;
                }

                if (implicit_h > 1)
                {
                    tiHydroIndex = _pushTextItem(ad, RenderItem::RIT_HYDROINDEX, color, highlighted);

                    TextItem& itemHydroIndex = _data.textitems[tiHydroIndex];
                    TextItem& itemHydrogen = _data.textitems[tihydro];
                    itemHydroIndex.fontsize = FONT_SIZE_ATTR;
                    bprintf(itemHydroIndex.text, "%i", implicit_h);
                    _cw.setTextItemSize(itemHydroIndex, ad.pos);
                    hydrogenGroupSz.x += itemHydroIndex.bbsz.x + _settings.labelInternalOffset;
                    hydrogenGroupSz.y = std::max(hydrogenGroupSz.y, _settings.lowerIndexShift * itemHydrogen.bbsz.y + itemHydroIndex.bbsz.y);
                }

                // take new reference, old one may be corrupted after adding 'tiHydroIndex'
                TextItem& itemHydrogen = _data.textitems[tihydro];
                if (ad.hydroPos == HYDRO_POS_LEFT)
                {
                    ad.leftMargin -= hydrogenGroupSz.x;
                    itemHydrogen.bbp.set(ad.leftMargin, ad.ypos);
                }
                else if (ad.hydroPos == HYDRO_POS_RIGHT)
                {
                    ad.rightMargin += _settings.labelInternalOffset;
                    itemHydrogen.bbp.set(ad.rightMargin, ad.ypos);
                    ad.rightMargin += hydrogenGroupSz.x;
                }
                else if (ad.hydroPos == HYDRO_POS_UP)
                {
                    itemHydrogen.bbp.y = ad.pos.y + ad.boundBoxMin.y - hydrogenGroupSz.y - _settings.unit;
                }
                else if (ad.hydroPos == HYDRO_POS_DOWN)
                {
                    itemHydrogen.bbp.y = ad.pos.y + ad.boundBoxMax.y + _settings.unit;
                }
                else
                {
                    throw Error("hydrogen position value invalid");
                }
                _expandBoundRect(ad, itemHydrogen);
                if (tiHydroIndex > 0)
                {
                    _data.textitems[tiHydroIndex].bbp.set(itemHydrogen.bbp.x + itemHydrogen.bbsz.x + _settings.labelInternalOffset,
                                                          itemHydrogen.bbp.y + _settings.lowerIndexShift * itemHydrogen.bbsz.y);
                    _expandBoundRect(ad, _data.textitems[tiHydroIndex]);
                }
            }
        }

        // charge
        _prepareChargeLabel(aid, color, highlighted);

        // valence
        int valence = bm.getExplicitValence(aid);

        if (_opt.showValences && valence >= 0)
        {
            tiValence = _pushTextItem(ad, RenderItem::RIT_VALENCE, color, highlighted);

            TextItem& itemValence = _data.textitems[tiValence];
            itemValence.fontsize = FONT_SIZE_ATTR;
            bprintf(itemValence.text, _valenceText(valence));
            _cw.setTextItemSize(itemValence);

            ad.rightMargin += _settings.labelInternalOffset;
            itemValence.bbp.set(ad.rightMargin, ad.ypos + _settings.upperIndexShift * ad.height);
            _expandBoundRect(ad, itemValence);
            ad.rightMargin += itemValence.bbsz.x;
        }

        // radical
        int radical = -1;

        if (!bm.isRSite(aid) && !bm.isPseudoAtom(aid) && !bm.isTemplateAtom(aid))
            radical = bm.getAtomRadical_NoThrow(aid, -1);

        if (radical > 0)
        {
            const TextItem& label = _data.textitems[tilabel];
            Vec2f ltc(label.bbp);

            if (radical == RADICAL_DOUBLET)
            {
                giRadical = _pushGraphItem(ad, RenderItem::RIT_RADICAL, color, highlighted);
                GraphItem& itemRadical = _data.graphitems[giRadical];
                _cw.setGraphItemSizeDot(itemRadical);

                if (!(ad.showHydro && ad.hydroPos == HYDRO_POS_RIGHT) && giChargeSign < 0 && tiValence < 0)
                {
                    ltc.x += label.bbsz.x + _settings.radicalRightOffset;
                    ltc.y += _settings.radicalRightVertShift * ad.height;
                    itemRadical.bbp.copy(ltc);
                }
                else
                {
                    ltc.x += label.bbsz.x / 2 - itemRadical.bbsz.x / 2;
                    ltc.y -= itemRadical.bbsz.y + _settings.radicalTopOffset;
                    itemRadical.bbp.copy(ltc);
                }
                _expandBoundRect(ad, itemRadical);
            }
            else
            {
                giRadical1 = _pushGraphItem(ad, RenderItem::RIT_RADICAL, color, highlighted);
                giRadical2 = _pushGraphItem(ad, RenderItem::RIT_RADICAL, color, highlighted);

                GraphItem& itemRadical1 = _data.graphitems[giRadical1];
                GraphItem& itemRadical2 = _data.graphitems[giRadical2];

                float dist;
                if (radical == RADICAL_SINGLET)
                {
                    _cw.setGraphItemSizeDot(itemRadical1);
                    _cw.setGraphItemSizeDot(itemRadical2);
                    dist = _settings.radicalTopDistDot;
                }
                else // if (radical == RADICAL_TRIPLET)
                {
                    _cw.setGraphItemSizeCap(itemRadical1);
                    _cw.setGraphItemSizeCap(itemRadical2);
                    dist = _settings.radicalTopDistCap;
                }

                ltc.y -= itemRadical1.bbsz.y + _settings.radicalTopOffset;
                ltc.x += label.bbsz.x / 2 - dist / 2 - itemRadical1.bbsz.x;
                itemRadical1.bbp.copy(ltc);
                ltc.x += dist + itemRadical1.bbsz.x;
                itemRadical2.bbp.copy(ltc);
                _expandBoundRect(ad, itemRadical1);
                _expandBoundRect(ad, itemRadical2);
            }
        }
    }

    int bondEndRightToStereoGroupLabel = -1;
    // prepare stereogroup labels
    if ((ad.stereoGroupType > 0 && ad.stereoGroupType != MoleculeStereocenters::ATOM_ANY) || ad.inversion == STEREO_INVERTS || ad.inversion == STEREO_RETAINS)
    {
        int tiStereoGroup = _pushTextItem(ad, RenderItem::RIT_STEREOGROUP, CWC_BASE, false);

        TextItem& itemStereoGroup = _data.textitems[tiStereoGroup];
        itemStereoGroup.fontsize = FONT_SIZE_ATTR;
        ArrayOutput itemOutput(itemStereoGroup.text);
        if (ad.stereoGroupType > 0 && ad.stereoGroupType != MoleculeStereocenters::ATOM_ANY)
        {
            const char* stereoGroupText = _getStereoGroupText(ad.stereoGroupType);
            itemOutput.printf("%s", stereoGroupText);
            if (ad.stereoGroupType != MoleculeStereocenters::ATOM_ABS)
                itemOutput.printf("%i", ad.stereoGroupNumber);
        }
        if (ad.inversion == STEREO_INVERTS || ad.inversion == STEREO_RETAINS)
        {
            if (itemOutput.tell() > 0)
                itemOutput.printf(",");
            itemOutput.printf("%s", ad.inversion == STEREO_INVERTS ? "Inv" : "Ret");
        }
        itemOutput.writeChar(0);

        _cw.setTextItemSize(itemStereoGroup);

        if (ad.showLabel)
        {
            // label visible - put stereo group label on the over or under the label
            const Vertex& v = bm.getVertex(aid);
            float vMin = 0, vMax = 0;
            for (int i = v.neiBegin(); i < v.neiEnd(); i = v.neiNext(i))
            {
                float y = _getBondEnd(aid, i).dir.y;
                if (y > vMax)
                    vMax = y;
                if (y < vMin)
                    vMin = y;
            }
            if (vMax > -vMin)
                itemStereoGroup.bbp.set(ad.pos.x - itemStereoGroup.bbsz.x / 2,
                                        ad.pos.y + ad.boundBoxMin.y - itemStereoGroup.bbsz.y - _settings.stereoGroupLabelOffset);
            else
                itemStereoGroup.bbp.set(ad.pos.x - itemStereoGroup.bbsz.x / 2, ad.pos.y + ad.boundBoxMax.y + _settings.stereoGroupLabelOffset);
        }
        else
        {
            // label hidden - position stereo group label independently
            Vec2f p;
            bondEndRightToStereoGroupLabel = _findClosestBox(p, aid, itemStereoGroup.bbsz, _settings.unit);

            p.addScaled(itemStereoGroup.bbsz, -0.5);
            itemStereoGroup.bbp.copy(p);
        }
        _expandBoundRect(ad, itemStereoGroup);
    }

    // prepare AAM labels
    if (ad.aam > 0)
    {
        int tiAAM = _pushTextItem(ad, RenderItem::RIT_AAM, CWC_BASE, false);

        TextItem& itemAAM = _data.textitems[tiAAM];
        itemAAM.fontsize = FONT_SIZE_ATTR;
        bprintf(itemAAM.text, "%i", abs(ad.aam));
        _cw.setTextItemSize(itemAAM);

        if (ad.showLabel)
        {
            ad.leftMargin -= itemAAM.bbsz.x + _settings.labelInternalOffset;
            itemAAM.bbp.set(ad.leftMargin, ad.ypos + _settings.lowerIndexShift * ad.height);
        }
        else
        {
            Vec2f p;
            _findClosestBox(p, aid, itemAAM.bbsz, _settings.unit, bondEndRightToStereoGroupLabel);

            p.addScaled(itemAAM.bbsz, -0.5);
            itemAAM.bbp.copy(p);
        }
        _expandBoundRect(ad, itemAAM);
    }

    // prepare R-group attachment point labels
    QS_DEF(Array<float>, angles);
    QS_DEF(Array<int>, split);
    QS_DEF(Array<int>, rGroupAttachmentIndices);

    if (ad.isRGroupAttachmentPoint)
    {
        // collect the angles between adjacent bonds
        const Vertex& v = bm.getVertex(aid);
        angles.clear();
        split.clear();
        if (v.degree() != 0)
        {
            for (int i = v.neiBegin(); i < v.neiEnd(); i = v.neiNext(i))
            {
                float a = _getBondEnd(aid, i).lang;
                angles.push(a);
                split.push(1);
            }
        }
        // collect attachment point indices
        rGroupAttachmentIndices.clear();
        bool multipleAttachmentPoints = _mol->attachmentPointCount() > 1;
        for (int i = 1; i <= _mol->attachmentPointCount(); ++i)
            for (int j = 0, k; (k = _mol->getAttachmentPoint(i, j)) >= 0; ++j)
                if (k == aid)
                    rGroupAttachmentIndices.push(i);
        if (v.degree() != 0)
        {
            for (int j = 0; j < rGroupAttachmentIndices.size(); ++j)
            {
                int i0 = -1;
                for (int i = 0; i < angles.size(); ++i)
                    if (i0 < 0 || angles[i] / (split[i] + 1) > angles[i0] / (split[i0] + 1))
                        i0 = i;
                split[i0]++;
            }
        }
        // arrange the directions of the attachment points
        QS_DEF(Array<Vec2f>, attachmentDirection);
        attachmentDirection.clear();
        if (v.degree() == 0)
        {
            // if no adjacent bonds present
            if (rGroupAttachmentIndices.size() == 1)
                attachmentDirection.push().set(0, -1);
            else if (rGroupAttachmentIndices.size() == 2)
            {
                attachmentDirection.push().set(cos((float)M_PI / 6), -sin((float)M_PI / 6));
                attachmentDirection.push().set(cos(5 * (float)M_PI / 6), -sin(5 * (float)M_PI / 6));
            }
            else
            {
                for (int j = 0; j < rGroupAttachmentIndices.size(); ++j)
                {
                    float a = j * 2 * (float)M_PI / rGroupAttachmentIndices.size();
                    attachmentDirection.push().set(cos(a), sin(a));
                }
            }
        }
        else
        {
            // split the angles
            for (int i = 0; i < split.size(); ++i)
            {
                angles[i] /= split[i];
            }
            for (int j = 0; j < rGroupAttachmentIndices.size(); ++j)
            {
                int i0 = -1, n = v.neiBegin();
                for (int i = 0; i < split.size(); ++i, n = v.neiNext(n))
                {
                    if (split[i] > 1)
                    {
                        i0 = i;
                        break;
                    }
                }
                if (i0 < 0)
                    throw Error("Error while arranging attachment points");
                Vec2f d;
                d.copy(_getBondEnd(aid, n).dir);
                d.rotateL(angles[i0] * (--split[i0]));
                attachmentDirection.push(d);
            }
        }
        // create the attachment point items
        ad.attachmentPointBegin = _data.attachmentPoints.size();
        ad.attachmentPointCount = rGroupAttachmentIndices.size();
        for (int j = 0; j < rGroupAttachmentIndices.size(); ++j)
        {
            RenderItemAttachmentPoint& attachmentPoint = _data.attachmentPoints.push();
            float offset = std::min(std::max(_getBondOffset(aid, ad.pos, attachmentDirection[j], _settings.unit), 0.f), 0.4f);
            attachmentPoint.dir.copy(attachmentDirection[j]);
            attachmentPoint.p0.lineCombin(ad.pos, attachmentDirection[j], offset);
            attachmentPoint.p1.lineCombin(ad.pos, attachmentDirection[j], 0.8f);
            attachmentPoint.color = CWC_BASE;
            attachmentPoint.highlighted = false;
            if (multipleAttachmentPoints)
            {
                attachmentPoint.number = rGroupAttachmentIndices[j];
            }
        }
    }

    // prepare atom id's
    if (_opt.showAtomIds)
    {
        tiindex = _pushTextItem(ad, RenderItem::RIT_ATOMID, CWC_BLUE, false);

        TextItem& index = _data.textitems[tiindex];
        index.fontsize = FONT_SIZE_INDICES;

        int base = _opt.atomBondIdsFromOne ? 1 : 0;
        bprintf(index.text, "%i", aid + base);
        _cw.setTextItemSize(index, ad.pos);

        if (ad.showLabel)
            index.bbp.set(ad.rightMargin + _settings.labelInternalOffset, ad.ypos + 0.5f * ad.height);
    }
}

int MoleculeRenderInternal::_pushTextItem(RenderItem::TYPE ritype, int color, bool highlighted)
{
    _data.textitems.push();
    _data.textitems.top().clear();
    _data.textitems.top().ritype = ritype;
    _data.textitems.top().color = color;
    _data.textitems.top().highlighted = highlighted;
    return _data.textitems.size() - 1;
}

int MoleculeRenderInternal::_pushTextItem(AtomDesc& ad, RenderItem::TYPE ritype, int color, bool highlighted)
{
    int res = _pushTextItem(ritype, color, highlighted);
    if (ad.tibegin < 0)
        ad.tibegin = res;
    ad.ticount++;
    return res;
}

int MoleculeRenderInternal::_pushTextItem(Sgroup& sg, RenderItem::TYPE ritype, int color)
{
    int res = _pushTextItem(ritype, color, false);
    if (sg.tibegin < 0)
        sg.tibegin = res;
    sg.ticount++;
    return res;
}

int MoleculeRenderInternal::_pushGraphItem(RenderItem::TYPE ritype, int color, bool highlighted)
{
    _data.graphitems.push();
    _data.graphitems.top().clear();
    _data.graphitems.top().ritype = ritype;
    _data.graphitems.top().color = color;
    _data.graphitems.top().highlighted = highlighted;
    return _data.graphitems.size() - 1;
}

int MoleculeRenderInternal::_pushGraphItem(AtomDesc& ad, RenderItem::TYPE ritype, int color, bool highlighted)
{
    int res = _pushGraphItem(ritype, color, highlighted);
    if (ad.gibegin < 0)
        ad.gibegin = res;
    ad.gicount++;
    return res;
}

const char* MoleculeRenderInternal::_valenceText(const int valence)
{
    const char* vt[] = {"(0)", "(I)", "(II)", "(III)", "(IV)", "(V)", "(VI)", "(VII)", "(VIII)", "(IX)", "(X)", "(XI)", "(XII)", "(XIII)"};
    if (valence < 0 || valence >= NELEM(vt))
        throw Error("valence value %i out of range", valence);
    return vt[valence];
}

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

double MoleculeRenderInternal::_getAdjustmentFactor(const int aid, const int anei, const double acos, const double asin, const double tgb, const double csb,
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
    double w = _settings.bondSpace;
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
    float w = _settings.bondSpace;
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
    Vec2f reduced = (be2.p - be1.p) * (1 - shorten / len);
    Vec2f slope_right(-reduced.y * 0.25, reduced.x * 0.25);
    _cw.drawLine(be2.p, (be2.p - reduced) + slope_right);
    Vec2f slope_left(reduced.y * 0.25, -reduced.x * 0.25);
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
    Vec2f l(be2.p), r(be2.p);
    float w = _settings.bondSpace;
    l.addScaled(bd.norm, -w);
    r.addScaled(bd.norm, w);
    bd.extP = bd.extN = w;

    float lw = _cw.currentLineWidth();
    Vec2f r0(be1.p), l0(be1.p);
    l0.addScaled(bd.norm, -lw / 2);
    r0.addScaled(bd.norm, lw / 2);

    if (bd.stereodir == 0)
    {
        _cw.drawLine(be1.p, be2.p);
        bd.extP = bd.extN = lw / 2;
    }
    else if (bd.stereodir == BOND_UP)
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
        int stripeCnt = std::max((int)((len) / lw / 2), 4);
        _cw.fillQuadStripes(r0, l0, r, l, stripeCnt);
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

void MoleculeRenderInternal::_drawStereoCareBox(BondDescr& bd, const BondEnd& be1, const BondEnd& be2)
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
