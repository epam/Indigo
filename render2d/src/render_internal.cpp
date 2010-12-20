/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
 * 
 * This file is part of Indigo toolkit.
 * 
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#include "base_cpp/output.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "graph/graph_highlighting.h"
#include "reaction/reaction.h"
#include "reaction/query_reaction.h"
#include "render_context.h"
#include "render_internal.h"

using namespace indigo;

static bool ElementHygrodenOnLeft[] =
{
   false,  // filler
   false,  //ELEM_H
   false,  //ELEM_He
   false,  //ELEM_Li
   false,  //ELEM_Be
   false,  //ELEM_B
   false,  //ELEM_C
   false,  //ELEM_N
   true,   //ELEM_O
   true,   //ELEM_F
   false,  //ELEM_Ne
   false,  //ELEM_Na
   false,  //ELEM_Mg
   false,  //ELEM_Al
   false,  //ELEM_Si
   false,  //ELEM_P
   true,   //ELEM_S
   true,   //ELEM_Cl
   false,  //ELEM_Ar
   false,  //ELEM_K
   false,  //ELEM_Ca
   false,  //ELEM_Sc
   false,  //ELEM_Ti
   false,  //ELEM_V
   false,  //ELEM_Cr
   false,  //ELEM_Mn
   false,  //ELEM_Fe
   false,  //ELEM_Co
   false,  //ELEM_Ni
   false,  //ELEM_Cu
   false,  //ELEM_Zn
   false,  //ELEM_Ga
   false,  //ELEM_Ge
   false,  //ELEM_As
   true,   //ELEM_Se
   true,   //ELEM_Br
   false,  //ELEM_Kr
   false,  //ELEM_Rb
   false,  //ELEM_Sr
   false,  //ELEM_Y
   false,  //ELEM_Zr
   false,  //ELEM_Nb
   false,  //ELEM_Mo
   false,  //ELEM_Tc
   false,  //ELEM_Ru
   false,  //ELEM_Rh
   false,  //ELEM_Pd
   false,  //ELEM_Ag
   false,  //ELEM_Cd
   false,  //ELEM_In
   false,  //ELEM_Sn
   false,  //ELEM_Sb
   false,  //ELEM_Te
   true,   //ELEM_I
   false,  //ELEM_Xe
   false,  //ELEM_Cs
   false,  //ELEM_Ba
   false,  //ELEM_La
   false,  //ELEM_Ce
   false,  //ELEM_Pr
   false,  //ELEM_Nd
   false,  //ELEM_Pm
   false,  //ELEM_Sm
   false,  //ELEM_Eu
   false,  //ELEM_Gd
   false,  //ELEM_Tb
   false,  //ELEM_Dy
   false,  //ELEM_Ho
   false,  //ELEM_Er
   false,  //ELEM_Tm
   false,  //ELEM_Yb
   false,  //ELEM_Lu
   false,  //ELEM_Hf
   false,  //ELEM_Ta
   false,  //ELEM_W
   false,  //ELEM_Re
   false,  //ELEM_Os
   false,  //ELEM_Ir
   false,  //ELEM_Pt
   false,  //ELEM_Au
   false,  //ELEM_Hg
   false,  //ELEM_Tl
   false,  //ELEM_Pb
   false,  //ELEM_Bi
   false,  //ELEM_Po
   false,  //ELEM_At
   false,  //ELEM_Rn
   false,  //ELEM_Fr
   false,  //ELEM_Ra
   false,  //ELEM_Ac
   false,  //ELEM_Th
   false,  //ELEM_Pa
   false,  //ELEM_U
   false,  //ELEM_Np
   false,  //ELEM_Pu
   false,  //ELEM_Am
   false,  //ELEM_Cm
   false,  //ELEM_Bk
   false,  //ELEM_Cf
   false,  //ELEM_Es
   false,  //ELEM_Fm
   false,  //ELEM_Md
   false,  //ELEM_No
   false   //ELEM_Lr
};

static bool _isBondWide (const BondDescr& bd)
{
   return bd.type == BOND_DOUBLE || bd.type == BOND_TRIPLE ||
      bd.queryType == QueryMolecule::QUERY_BOND_DOUBLE_OR_AROMATIC || 
      bd.queryType == QueryMolecule::QUERY_BOND_SINGLE_OR_AROMATIC ||
      bd.queryType == QueryMolecule::QUERY_BOND_SINGLE_OR_DOUBLE;
}

RenderOptions::RenderOptions ()
{
   clear();
}

void RenderOptions::clear()
{
   highlightThicknessEnable = false;
   highlightThicknessFactor = 1.8f;
   highlightColorEnable = true;
   highlightColor.set(1, 0, 0);
   aamColor.set(0, 0, 0);
   commentFontFactor = 20;
   titleFontFactor = 20;
   labelMode = LABEL_MODE_NORMAL;
   implHMode = IHM_TERMINAL_HETERO;
   comment.clear();
   commentPos = COMMENT_POS_BOTTOM;
   commentAlign = 0.5f;
   titleAlign = 0.5f;
   commentColor.set(0,0,0);
   gridColumnNumber = 1;
   showAtomIds = false;
   showBondIds = false;
   showBondEndIds = false;
   showNeighborArcs = false;
   showValences = true;
   atomColoring = false;
   useOldStereoNotation = false;
   showReactingCenterUnchanged = false; 
   centerDoubleBondWhenStereoAdjacent = false;
   showCycles = false;
}

MoleculeRenderInternal::MoleculeRenderInternal (const RenderOptions& opt, const RenderSettings& settings, RenderContext& cw) :
_mol(NULL), _cw(cw), _highlighting(NULL), _settings(settings), _opt(opt), TL_CP_GET(_data)
{
   _data.clear();
}

void MoleculeRenderInternal::setMolecule (BaseMolecule* mol)
{
   _mol = mol;
   _data.clear();

   int i;

   // data
   _data.atoms.clear();
   _data.atoms.resize(_mol->vertexEnd());
   for (i = mol->vertexBegin(); i != mol->vertexEnd(); i = mol->vertexNext(i))
      _ad(i).clear();

   _data.bonds.clear_resize(_mol->edgeEnd());
   for (i = mol->edgeBegin(); i != mol->edgeEnd(); i = mol->edgeNext(i))
      _bd(i).clear();
}

void MoleculeRenderInternal::setScaleFactor (const float scaleFactor, const Vec2f& min, const Vec2f& max)
{
   _scale = scaleFactor;
   _min.copy(min);
   _max.copy(max);
}

void MoleculeRenderInternal::setReactionComponentProperties (const Array<int>* aam, 
                                                             const Array<int>* reactingCenters,
                                                             const Array<int>* inversions)
{
   if (aam != NULL)
      _data.aam.copy(*aam);
   if (reactingCenters != NULL)
      _data.reactingCenters.copy(*reactingCenters);
   if (inversions != NULL)
      _data.inversions.copy(*inversions);
}

void MoleculeRenderInternal::setQueryReactionComponentProperties (const Array<int>* exactChanges)
{
   if (exactChanges != NULL)
      _data.exactChanges.copy(*exactChanges);
}

void MoleculeRenderInternal::setHighlighting (const GraphHighlighting* highlighting)
{
  _highlighting = highlighting;
}

void MoleculeRenderInternal::render ()
{
   _checkSettings();

   _initCoordinates();

   _initBondData();

   _initBondEndData();

   _findNeighbors();

   _findRings();

   _determineDoubleBondShift();
                 
   _determineStereoGroupsMode();

   _initAtomData();

   _initRGroups();

   _findCenteredCase();

   _prepareLabels();

   _extendRenderItems();

   _findAnglesOverPi();

   _calculateBondOffset();

   _applyBondOffset();

   _setBondCenter();

   _renderLabels();

   _renderBonds();

   _renderRings();

   _renderBondIds();

   _renderAtomIds();
}

BondEnd& MoleculeRenderInternal::_be (int beid)
{
   return _data.bondends[beid]; 
}

const BondEnd& MoleculeRenderInternal::_be (int beid) const
{
   return _data.bondends[beid]; 
}

BondDescr& MoleculeRenderInternal::_bd (int bid)
{
   return _data.bonds[bid]; 
}

const BondDescr& MoleculeRenderInternal::_bd (int bid) const
{
   return _data.bonds[bid]; 
}

AtomDesc& MoleculeRenderInternal::_ad (int aid)
{
   return _data.atoms[aid]; 
}

const AtomDesc& MoleculeRenderInternal::_ad (int aid) const
{
   return _data.atoms[aid]; 
}

int MoleculeRenderInternal::_getOpposite (int beid) const
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
         float al1 = be1.lang, ar1 = ber1.lang,
            al2 = bel2.lang, ar2 = be2.lang;
         int neiBalance = 
            (al1 < M_PI ? 1 : 0) +
            (al2 < M_PI ? 1 : 0) +
            (ar1 < M_PI ? -1 : 0) +
            (ar2 < M_PI ? -1 : 0);
         if (neiBalance > 0)
            bd.lineOnTheRight = false;
         else if (neiBalance < 0)
            bd.lineOnTheRight = true;
         else
         {
            // compare the number of wide (double, triple, etc.) bonds on both sides
            int wideNeiBalance = 
               (al1 < M_PI && _isBondWide(_bd(bel1.bid)) ? 1 : 0) +
               (al2 < M_PI && _isBondWide(_bd(bel2.bid)) ? 1 : 0) +
               (ar1 < M_PI && _isBondWide(_bd(ber1.bid)) ? -1 : 0) +
               (ar2 < M_PI && _isBondWide(_bd(ber2.bid)) ? -1 : 0);
            if (wideNeiBalance > 0)
               bd.lineOnTheRight = false;
            else if (wideNeiBalance < 0)
               bd.lineOnTheRight = true;
            else 
            {
               // compare the number of wide (double, triple, etc.) bonds on both sides
               int stereoBalance = 
                  (al1 < M_PI && _bd(bel1.bid).stereodir != 0 ? 1 : 0) +
                  (al2 < M_PI && _bd(bel2.bid).stereodir != 0 ? 1 : 0) +
                  (ar1 < M_PI && _bd(ber1.bid).stereodir != 0 ? -1 : 0) +
                  (ar2 < M_PI && _bd(ber2.bid).stereodir != 0 ? -1 : 0);
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

void MoleculeRenderInternal::_extendRenderItem (RenderItem& item, const float extent)
{
   Vec2f exv(extent, extent);
   item.bbsz.addScaled(exv, 2);
   item.bbp.sub(exv);
   item.relpos.add(exv);
}

#define __minmax(inv, t1, t2) (inv ? __min(t1, t2) : __max(t1, t2))
bool MoleculeRenderInternal::_clipRaySegment (float& offset, const Vec2f& p, const Vec2f& d, const Vec2f& n0, const Vec2f& a, const Vec2f& b, const float w)
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
   float tl = -w/2;
   float tr = w/2;
   float t = 0;
   bool f = false;
   bool inv = Vec2f::dot(ab, d) < 0;
   if (ta < tl && tl < tb)
   {
      t = f ? __minmax(inv, t, tl) : tl;
      f = true;
   }
   if (ta < tr && tr < tb)
   {
      t = f ? __minmax(inv, t, tr) : tr;
      f = true;
   }
   if (tl < ta && ta < tr)
   {
      t = f ? __minmax(inv, t, ta) : ta;
      f = true;
   }
   if (tl < tb && tb < tr)
   {
      t = f ? __minmax(inv, t, tb) : tb;
      f = true;
   }

   if (!f)
      return false;
   pa.addScaled(ab, (t - ta)/fabs(dot));
   offset = Vec2f::dot(d, pa);
   return true;
}

bool MoleculeRenderInternal::_clipRayBox (float& offset, const Vec2f& p, const Vec2f& d, const Vec2f& rp, const Vec2f& sz, const float w)
{
   Vec2f n(-d.y, d.x);
   Vec2f a, b;
   bool f = false;
   float t = 0, tt;

   a.set(rp.x, rp.y);
   b.set(rp.x + sz.x, rp.y);
   if (_clipRaySegment(tt, p, d, n, a, b, w))
   {
      f = true;
      t = __max(t, tt);
   }

   a.set(rp.x, rp.y);
   b.set(rp.x, rp.y + sz.y);
   if (_clipRaySegment(tt, p, d, n, a, b, w))
   {
      f = true;
      t = __max(t, tt);
   }

   a.set(rp.x + sz.x, rp.y);
   b.set(rp.x + sz.x, rp.y + sz.y);
   if (_clipRaySegment(tt, p, d, n, a, b, w))
   {
      f = true;
      t = __max(t, tt);
   }

   a.set(rp.x, rp.y + sz.y);
   b.set(rp.x + sz.x, rp.y + sz.y);
   if (_clipRaySegment(tt, p, d, n, a, b, w))
   {
      f = true;
      t = __max(t, tt);
   }

   if (f)
      offset = t;
   return f;
}

const char* MoleculeRenderInternal::_getStereoGroupText (int type)
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

void MoleculeRenderInternal::_checkSettings ()
{
   _data.labelMode = _opt.labelMode;
   switch (_opt.implHMode)
   {
   case IHM_NONE:
      break;
   case IHM_TERMINAL:
   case IHM_TERMINAL_HETERO:
      if (_opt.labelMode == LABEL_MODE_HIDETERMINAL)
         _data.labelMode = LABEL_MODE_NORMAL;
   case IHM_HETERO:
      if (_opt.labelMode == LABEL_MODE_FORCEHIDE)
         _data.labelMode = LABEL_MODE_NORMAL;
      break;
   case IHM_ALL:
      _data.labelMode = LABEL_MODE_FORCESHOW;
      break;
   }
}

void MoleculeRenderInternal::_initRGroups()
{
   QUERY_MOL_BEGIN(_mol);
      if (qmol.isRGroupFragment()) {
         MoleculeRGroupFragment& rfragment = qmol.getRGroupFragment();
         for (int i = 0; i < rfragment.attachmentPointCount(); ++i)
            for (int j = 0, k; (k = rfragment.getAttachmentPoint(i, j)) >= 0; ++j)
               _ad(k).isRGroupAttachmentPoint = true;
      }
   QUERY_MOL_END;
}

void MoleculeRenderInternal::_findRings()
{
   for (int i = 0; i < _data.bondends.size(); ++i)
   {
      BondEnd& be = _be(i);
      if (be.lRing >= 0)
         continue;
      int rid = _data.rings.size();
      _data.rings.push();
      Ring& ring = _data.rings[rid];
      ring.bondEnds.push(i);
   
      int j = be.next;
      for (int c = 0; j != i; j = _be(j).next, ++c)
      {             
         if (c > _data.bondends.size() || j < 0)
            break;
         ring.bondEnds.push(j);
      }
      if (i != j)
      {
         ring.bondEnds.pop();
         continue;
      }

      // for the inner loops, sum of the angles should be (n-2)*pi, 
      // for the outer ones (n+2)*pi
      float angleSum = 0;
      for (int j = 0; j < ring.bondEnds.size(); ++j)
         angleSum += _be(ring.bondEnds[j]).lang;
      bool inner = (angleSum < ring.bondEnds.size() * M_PI); 

      if (!inner)
      {
         _data.rings.pop();
         continue;
      }

      for (int j = 0; j < ring.bondEnds.size(); ++j)
         _be(ring.bondEnds[j]).lRing = rid;

      if (_opt.showCycles)
      {
         float cycleLineOffset = _settings.bondLineWidth * 9;
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
         _cw.setLineWidth(_settings.bondLineWidth);
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

   for (int i = 0; i < _data.bonds.size(); ++i)
   {
      BondDescr& bd = _bd(i);
      BondEnd& be1 = _be(bd.be1);
      BondEnd& be2 = _be(bd.be2);
      bd.inRing = (be1.lRing >= 0 || be2.lRing >= 0);
      bd.aromRing = ((be1.lRing >= 0) ? _data.rings[be1.lRing].aromatic : false) || 
         ((be2.lRing >= 0) ? _data.rings[be2.lRing].aromatic : false);
   }
}

void MoleculeRenderInternal::_prepareLabels()
{
   for (int i = _mol->vertexBegin(); i < _mol->vertexEnd(); i = _mol->vertexNext(i))
      _prepareLabelText(i);
}

void MoleculeRenderInternal::_initCoordinates()
{
   Vec2f v;
   for (int i = _mol->vertexBegin(); i < _mol->vertexEnd(); i = _mol->vertexNext(i))
   {
      Vec2f::projectZ(v, _mol->getAtomXyz(i));
      Vec2f& p = _ad(i).pos;
      p.set((v.x - _min.x) * _scale, (_max.y - v.y) * _scale);
   }
}

void MoleculeRenderInternal::_determineStereoGroupsMode()
{
   const MoleculeStereocenters& sc = _mol->stereocenters;

   _lopt.stereoMode = STEREOGROUPS_HIDE;
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

   if (singleAndGroup)
      return;

   if (allAbs && _opt.useOldStereoNotation && !none)
   {
      
      TextItem& tiChiral = _data.textitems[_pushTextItem(RenderItem::RIT_CHIRAL, CWC_BASE, false)];
      bprintf(tiChiral.text, "Chiral");
      tiChiral.fontsize = FONT_SIZE_LABEL;
      _cw.setTextItemSize(tiChiral);
      tiChiral.bbp.set((_max.x - _min.x) * _scale - tiChiral.bbsz.x, -tiChiral.bbsz.y * 2);
      _cw.setSingleSource(CWC_BASE);
      _cw.drawTextItemText(tiChiral);
      return;
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

bool MoleculeRenderInternal::_isSingleHighlighted (int aid)
{
   const Vertex& vertex = _mol->getVertex(aid);
   if (!_vertexIsHighlighted(aid))
      return false;
   for (int j = vertex.neiBegin(); j < vertex.neiEnd(); j = vertex.neiNext(j))
      if (_edgeIsHighlighted(vertex.neiEdge(j)))
         return false;
   return true;
}

bool MoleculeRenderInternal::_vertexIsHighlighted (int aid)
{
   return _highlighting != NULL && _highlighting->numVertices() > 0 && _highlighting->hasVertex(aid);
}

bool MoleculeRenderInternal::_edgeIsHighlighted (int bid)
{
   return _highlighting != NULL && _highlighting->numEdges() > 0 && _highlighting->hasEdge(bid);
}

bool MoleculeRenderInternal::_hasQueryModifiers (int aid)
{
   bool hasConstraints = false;
   QUERY_MOL_BEGIN(_mol);
   QueryMolecule::Atom& qa = qmol.getAtom(aid);
   hasConstraints = qa.hasConstraint(QueryMolecule::ATOM_RING_BONDS) ||
      qa.hasConstraint(QueryMolecule::ATOM_SUBSTITUENTS) ||
      qa.hasConstraint(QueryMolecule::ATOM_UNSATURATION) ||
      qa.hasConstraint(QueryMolecule::ATOM_TOTAL_H);
   QUERY_MOL_END;
   return hasConstraints ||
      _ad(aid).fixed;
}

void MoleculeRenderInternal::_initAtomData ()
{
   QUERY_MOL_BEGIN(_mol);
   for (int i = 0; i < qmol.fixed_atoms.size(); ++i)
      _ad(i).fixed = true;
   QUERY_MOL_END;

   for (int i = _mol->vertexBegin(); i < _mol->vertexEnd(); i = _mol->vertexNext(i))
   {
      AtomDesc& ad = _ad(i);
      BaseMolecule& bm = *_mol;
      const Vertex& vertex = bm.getVertex(i);

      //QS_DEF(Array<char>, buf);
      //buf.clear();
      //bm.getAtomDescription(i, buf);
      //printf("%s\n", buf.ptr());

      int atomNumber = bm.getAtomNumber(i);
      if (_mol->isPseudoAtom(i))
      {
         ad.type = AtomDesc::TYPE_PSEUDO;
         ad.pseudo.readString(_mol->getPseudoAtom(i), true);
      }
      else if (atomNumber < 0 || atomNumber == ELEM_RSITE)
         ad.type = AtomDesc::TYPE_QUERY;
      else
         ad.type = AtomDesc::TYPE_REGULAR;

      ad.label = -1;
      if (ad.type == AtomDesc::TYPE_REGULAR)
         ad.label = atomNumber;

      ad.queryLabel = -1;
      if (ad.type == AtomDesc::TYPE_QUERY) {
         if (!bm.isRSite(i)) {
            QUERY_MOL_BEGIN(_mol);
            ad.queryLabel = QueryMolecule::parseQueryAtom(qmol, i, ad.list);
            if (ad.queryLabel < 0) {
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
         float dot = Vec2f::dot(d, h);
         if (dot > 0.7)
            hasBondOnRight = true;
         else if (dot < -0.7)
            hasBondOnLeft = true;
      }

      int charge = bm.getAtomCharge(i);
      int isotope = bm.getAtomIsotope(i);
      int radical = (!bm.isRSite(i) && !bm.isPseudoAtom(i)) ? bm.getAtomRadical(i) : 0;
      int valence = bm.getExplicitValence(i);
      bool query = bm.isQueryMolecule();
      bool plainCarbon =
         ad.label == ELEM_C && 
         charge == (query ? CHARGE_UNKNOWN : 0) &&
         isotope == (query ? -1 : 0) &&
         radical == (query ? -1 : 0) &&
         valence == -1 &&
         !_hasQueryModifiers(i);


      ad.showLabel = true;
      if (_data.labelMode == LABEL_MODE_FORCESHOW)
         ;
      else if (_data.labelMode == LABEL_MODE_FORCEHIDE)
         ad.showLabel = false;
      else if (plainCarbon && 
         (_data.labelMode == LABEL_MODE_HIDETERMINAL || vertex.degree() > 1) && 
         !_isSingleHighlighted(i))
      {
         if (vertex.degree() == 2)
         {
            int k1 = vertex.neiBegin();
            int k2 = vertex.neiNext(k1);
            if (_bd(vertex.neiEdge(k1)).type == _bd(vertex.neiEdge(k2)).type)
            {
               float dot = Vec2f::dot(_getBondEnd(i, k1).dir, _getBondEnd(i, k2).dir);
               if (dot < -0.97)
                  continue;
            }
         }
         ad.showLabel = false;
      }
      ad.shiftLeft = (hasBondOnRight && !hasBondOnLeft) || (ad.label > 0 && ElementHygrodenOnLeft[ad.label] && !(hasBondOnLeft && !hasBondOnRight));
   }
}

void MoleculeRenderInternal::_findAnglesOverPi ()
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
            if (_bd(rbe.bid).type == BOND_DOUBLE && 
               _bd(lbe.bid).type == BOND_DOUBLE && rbe.lang < M_PI)
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
      BondEnd& rmbe = _be(rightmost);
      BondEnd& lmbe = _be(leftmost);
      float dot = Vec2f::dot(rmbe.dir, lmbe.dir);
      if (dot > 0 || 1 + dot < 1e-4)
         continue;
      float ahs = sqrt((1 + dot)/(1 - dot));
      lmbe.offset = rmbe.offset = -ahs *  
         __min(_bd(lmbe.bid).thickness, _bd(rmbe.bid).thickness) / 2; 
   }
}

void MoleculeRenderInternal::_renderBondIds ()
{
   // show bond ids
   if (_opt.showBondIds)
   {
      for (int i = _mol->edgeBegin(); i < _mol->edgeEnd(); i = _mol->edgeNext(i))
      {
         TextItem ti;
         ti.fontsize = FONT_SIZE_INDICES;
         ti.color = CWC_DARKGREEN;
         bprintf(ti.text, "%i", i);
         Vec2f v;
         v.sum(_be(_bd(i).be1).p, _be(_bd(i).be2).p);
         v.scale(0.5);
         _cw.setTextItemSize(ti, v);
         _extendRenderItem(ti, _settings.boundExtent);
         _cw.drawItemBackground(ti);
         _cw.drawTextItemText(ti);
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
         _cw.drawTextItemText(ti);
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
         _cw.drawArc(_ad(be.aid).pos, _settings.bondSpace * 3 + _settings.bondLineWidth, a3, a2);

      }
   }
}

void MoleculeRenderInternal::_renderAtomIds ()
{
   // show atom ids
   if (_opt.showAtomIds)
   {
      for (int i = _mol->vertexBegin(); i < _mol->vertexEnd(); i = _mol->vertexNext(i))
      {
         const AtomDesc &desc = _ad(i);
         for (int i = 0; i < desc.ticount; ++i)
         {
            const TextItem &ti = _data.textitems[i + desc.tibegin];
            if (ti.ritype == RenderItem::RIT_ATOMID)
            {
               _cw.drawItemBackground(ti);
               _cw.drawTextItemText(ti);
            }
         }
      }
   }
}


void MoleculeRenderInternal::_renderLabels ()
{
   // draw data.atoms
   for (int i = _mol->vertexBegin(); i < _mol->vertexEnd(); i = _mol->vertexNext(i))
      _drawAtom(_ad(i));
}

void MoleculeRenderInternal::_renderRings ()
{
   // draw data.rings
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
            float a0 = ring.angles[k],
               a1 = ring.angles[(k + 1) % ring.bondEnds.size()];
            if (fabs(a1 - a0) > PI)
               _cw.drawArc(ring.center, r, __max(a0,a1), __min(a0,a1));
            else
               _cw.drawArc(ring.center, r, __min(a0,a1), __max(a0,a1));
            if (_edgeIsHighlighted(be.bid))
               _cw.resetHighlight();
         }
      }
   }
}


void MoleculeRenderInternal::_renderBonds ()
{
   // draw bonds
   for (int i = _mol->edgeBegin(); i < _mol->edgeEnd(); i = _mol->edgeNext(i))
      _drawBond(i);
}

void MoleculeRenderInternal::_applyBondOffset ()
{
   // apply offset to data.bondends
   for (int i = 0; i < _data.bondends.size(); ++i)
   {
      BondEnd& be = _be(i);
      be.p.addScaled(be.dir, be.offset);
   }
}

void MoleculeRenderInternal::_setBondCenter ()
{
   // find bond center
   for (int i = 0; i < _data.bonds.size(); ++i)
   {
      BondDescr& bd = _bd(i);
      bd.center.lineCombin2(_be(bd.be1).p, 0.5f, _be(bd.be2).p, 0.5f);
   }
}

void MoleculeRenderInternal::_findNeighbors ()
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

void MoleculeRenderInternal::_findCenteredCase ()
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

         if (((bdl.type == BOND_SINGLE && (bdl.stereodir == 0 || _opt.centerDoubleBondWhenStereoAdjacent))
            && (bdr.type == BOND_SINGLE && (bdr.stereodir == 0 || _opt.centerDoubleBondWhenStereoAdjacent))) 
            || (bdr.type == BOND_SINGLE && bdl.type == BOND_SINGLE && bdl.stereodir != 0 && bdr.stereodir != 0))
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

void MoleculeRenderInternal::_calculateBondOffset ()
{
   // calculate offset for bonds
   float offset;
   for (int i = _mol->vertexBegin(); i < _mol->vertexEnd(); i = _mol->vertexNext(i))
   {
      const Vertex& vertex = _mol->getVertex(i);
      for (int j = vertex.neiBegin(); j < vertex.neiEnd(); j = vertex.neiNext(j))
      {
         if (!_ad(i).showLabel)
            continue;
         BondEnd& be1 = _getBondEnd(i, j);

         for (int k = 0; k < _ad(i).ticount; ++k)
         {
            TextItem& item = _data.textitems[_ad(i).tibegin + k];
            if (_clipRayBox(offset, be1.p, be1.dir, item.bbp, 
                  item.bbsz, be1.width))
               be1.offset = __max(be1.offset, offset);
         }
         for (int k = 0; k < _ad(i).gicount; ++k)
         {
            GraphItem& item = _data.graphitems[_ad(i).gibegin + k];
            if (_clipRayBox(offset, be1.p, be1.dir, item.bbp, 
                  item.bbsz, be1.width))
               be1.offset = __max(be1.offset, offset);
         }
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

void MoleculeRenderInternal::_initBondData ()
{
   float thicknessHighlighted = _cw.highlightedBondLineWidth();

   for (int i = _mol->edgeBegin(); i < _mol->edgeEnd(); i = _mol->edgeNext(i))
   {
      BondDescr &d = _bd(i);
      d.type = _mol->getBondOrder(i);
      d.thickness = _edgeIsHighlighted(i) ? thicknessHighlighted : _settings.bondLineWidth;
      d.queryType = -1;
      QUERY_MOL_BEGIN(_mol);
      {
         QueryMolecule::Bond& qb = qmol.getBond(i);
         d.queryType = QueryMolecule::getQueryBondType(qb);
         d.stereoCare = qmol.bondStereoCare(i);
         if (qb.hasConstraint(QueryMolecule::BOND_TOPOLOGY)) {
            bool chainPossible = qb.possibleValue(QueryMolecule::BOND_TOPOLOGY, TOPOLOGY_CHAIN);
            bool ringPossible = qb.possibleValue(QueryMolecule::BOND_TOPOLOGY, TOPOLOGY_RING);
            d.topology = 0;
            if (chainPossible && !ringPossible) {
               d.topology = TOPOLOGY_CHAIN;
            }
            if (ringPossible && !chainPossible) {
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
      d.isShort = d.length < (_settings.bondSpace + _settings.bondLineWidth) * 2;

      d.stereodir = _mol->stereocenters.getBondDirection(i);
   }
}

void MoleculeRenderInternal::_initBondEndData ()
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
      else if (bd.type == BOND_DOUBLE ||
         bd.type == BOND_AROMATIC ||
         bd.type == BOND_TRIPLE ||
         bd.queryType >= 0)
         be.width = 4 * _settings.bondSpace + _settings.bondLineWidth;
      else {
         Array<char> buf;
         _mol->getBondDescription(be.bid, buf);
         throw Error("Unknown bond type %s. Can not determine bond width.", buf.ptr());
      }

   }
}

void MoleculeRenderInternal::_extendRenderItems ()
{
   for (int i = 0; i < _data.textitems.size(); ++i)
      _extendRenderItem(_data.textitems[i], _settings.boundExtent);
   for (int i = 0; i < _data.graphitems.size(); ++i)
      _extendRenderItem(_data.graphitems[i], _settings.boundExtent);
}

BondEnd& MoleculeRenderInternal::_getBondEnd (int aid, int nei)
{
   return _be(_getBondEndIdx(aid, nei));
}

int MoleculeRenderInternal::_getBondEndIdx (int aid, int nei)
{
   int ne = _mol->getVertex(aid).neiEdge(nei);
   int be = _bd(ne).getBondEnd(aid);
   return be;
}

void MoleculeRenderInternal::_drawAtom (const AtomDesc& desc)
{
#ifdef RENDER_SHOW_BACKGROUND
   for (int i = 0; i < desc.ticount; ++i)
      _cw.drawItemBackground(_data.textitems[i + desc.tibegin]);
   for (int i = 0; i < desc.gicount; ++i)
      _cw.drawItemBackground(_data.graphitems[i + desc.gibegin]);
#endif

   _cw.setSingleSource(desc.color);
   for (int i = 0; i < desc.ticount; ++i)
      _cw.drawTextItemText(_data.textitems[i + desc.tibegin]);
   for (int i = 0; i < desc.gicount; ++i)
      _cw.drawGraphItem(_data.graphitems[i + desc.gibegin]);
}

void MoleculeRenderInternal::_writeQueryAtomToString (Output& output, int aid)
{
   BaseMolecule& bm = *_mol;
   AtomDesc& ad = _ad(aid);

   if (bm.isRSite(aid))
   {
      QS_DEF(Array<int>, rg);
      bm.getAllowedRGroups(aid, rg);

      if (rg.size() == 0)
         output.printf("R");
      else for (int i = 0; i < rg.size(); ++i)
      {
         if (i > 0)
            output.printf(",");
         output.printf("R%i", rg[i]);
      }
   } else {
      if (!bm.isQueryMolecule())
         throw Error("Atom type %d not supported in non-queries", ad.queryLabel);

      if (ad.queryLabel == QueryMolecule::QUERY_ATOM_A) {
         output.printf("A");
      } else if (ad.queryLabel == QueryMolecule::QUERY_ATOM_X) {
         output.printf("X");
      } else if (ad.queryLabel == QueryMolecule::QUERY_ATOM_Q) {
         output.printf("Q");
      } else if (ad.queryLabel == QueryMolecule::QUERY_ATOM_LIST || ad.queryLabel == QueryMolecule::QUERY_ATOM_NOTLIST) {
         if (ad.queryLabel == QueryMolecule::QUERY_ATOM_NOTLIST)
            output.printf("!");
         output.printf("[");
         for (int i = 0; i < ad.list.size(); ++i)
         {
            if (i > 0)
               output.printf(",");
            output.printf("%s", Element::toString(ad.list[i]));
         }
         output.printf("]");
      } else {
         throw Error("Query atom type %d not supported", ad.queryLabel);
      }
   }
}

bool MoleculeRenderInternal::_writeDelimiter (bool needDelimiter, Output &output)
{
   if (needDelimiter)
      output.printf(",");
   else
      output.printf("(");
   return true;
}

QueryMolecule::Atom* atomNodeInConjunction (QueryMolecule::Atom& qa, int type) {
   if (qa.type != QueryMolecule::OP_AND)
      return NULL;
   for (int i = 0; i < qa.children.size(); ++i)
      if (qa.child(i)->type == type)
         return qa.child(i);
   return NULL;
}

void MoleculeRenderInternal::_writeQueryModifier (Output& output, int aid)
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

      if (qa.hasConstraint(QueryMolecule::ATOM_RING_BONDS))
      {
         int ringBondCount = qmol.getAtomRingBondsCount(aid);
         needDelimiter = _writeDelimiter(needDelimiter, output);
         if (ringBondCount >= 0)
            output.printf("rb%i", ringBondCount);
      }

      if (qa.hasConstraint(QueryMolecule::ATOM_UNSATURATION))
      {
         needDelimiter = _writeDelimiter(needDelimiter, output);
         output.printf("u");
      }

      if (qa.hasConstraint(QueryMolecule::ATOM_TOTAL_H)) {
         QueryMolecule::Atom *qc = atomNodeInConjunction(qa, QueryMolecule::ATOM_TOTAL_H);
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
   }
   QUERY_MOL_END;
}

static void _expandBoundRect (AtomDesc& ad, const RenderItem& item)
{
   Vec2f min, max;
   min.diff(item.bbp, ad.pos);
   max.sum(min, item.bbsz);
   ad.boundBoxMin.min(min);
   ad.boundBoxMax.max(max);
}

int MoleculeRenderInternal::_findClosestBox (Vec2f& p, int aid, const Vec2f& sz, float mrg, int skip)
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

      float factor = fabs(sin(ang/2));
      Vec2f rightNorm(rightDir), leftNorm(leftDir);
      rightNorm.rotateL((float)M_PI/2);
      leftNorm.rotateL(-(float)M_PI/2);

      float rightOffset = 0, leftOffset = 0;
      rightOffset = w2 * fabs(leftNorm.x) + h2 * fabs(leftNorm.y);
      leftOffset = w2 * fabs(rightNorm.x) + h2 * fabs(rightNorm.y);

      float t = __max(leftShift + rightOffset, leftOffset + rightShift) / factor;
      Vec2f median(rightDir);
      median.rotateL(ang / 2);
      q.addScaled(median, t);
      if (iMin < 0 || q.lengthSqr() < p.lengthSqr()) {
         iMin = i;
         p.copy(q);
      }
   }
   p.add(ad.pos);
   return iMin;
}

int MoleculeRenderInternal::_findClosestCircle (Vec2f& p, int aid, float radius, int skip)
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
      const BondEnd& lbe = _be(rbe.lnei);;
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

      float factor = __max(fabs(si), 0.01f);
      if (rightShift > 0)
         origin.addScaled(leftDir, rightShift / factor);
      if (leftShift > 0)
         origin.addScaled(rightDir, leftShift / factor);

      q.copy(rightDir);
      q.rotate(ang/2);

      double dst = radius / sin2c(Vec2f::dot(rightDir, leftDir));
      q.scale((float)dst);
      q.add(origin);

      if (iMin < 0 || q.lengthSqr() < p.lengthSqr())
      {
         iMin = i;
         p.copy(q);
      }
      q.add(ad.pos);
      //_cw.setLineWidth(_settings.bondLineWidth);
      //_cw.setSingleSource(CWC_BLUE);
      //_cw.drawCircle(q, radius);

   }
   p.add(ad.pos);
   return iMin;
}

enum CHARCAT {DIGIT, LETTER, SUPERSCRIPT, SIGN, WHITESPACE};
enum SCRIPT {MAIN, SUB, SUPER};

void MoleculeRenderInternal::_preparePseudoAtom (int aid, int color, bool highlighted)
{
   AtomDesc& ad = _ad(aid);
   const char* str = ad.pseudo.ptr();

   int cnt = 0;
   CHARCAT a = WHITESPACE, b;
   int i0 = 0, i1;
   SCRIPT script = MAIN, newscript = MAIN;
   int len = (int)strlen(str);
   // TODO: replace remembering item ids and shifting each of them with single offset value for an atom
   Array<int> tis, gis;

   TextItem fake;
   fake.fontsize = FONT_SIZE_LABEL;
   fake.text.push('C'); 
   fake.text.push((char)0);
   _cw.setTextItemSize(fake, ad.pos);
   float xpos = fake.bbp.x, 
      width = fake.bbsz.x, 
      offset = _settings.bondLineWidth/2, 
      space = _settings.bondLineWidth, 
      totalwdt = 0,
      upshift = -0.4f, 
      downshift = 0.4f;
   ad.ypos = fake.bbp.y;
   ad.height = fake.bbsz.y;
   ad.leftMargin = ad.rightMargin = xpos;

   if (ad.pseudoAtomStringVerbose) {
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
   } else {
      for (int i = 0; i <= len; ++i) {
         char c = (i == len ? ' ' : str[i]);
         if (isspace(c))
            b = WHITESPACE;
         else if (isalpha(c) || c == '*' || c == '(' || c == ')' || c == '|' || c == '!' || c == '&' || c == '<' || c == '>' || c == '?')
            b = LETTER;
         else if (isdigit(c))
            b = DIGIT;
         else if (c == '+' || c == '-')
            b = SIGN;
         else if (c == '\\' && tolower(str[i+1]) == 's')
            b = SUPERSCRIPT, ++i;
         else if (c == '\\' && tolower(str[i+1]) == 'n')
            b = WHITESPACE, ++i;
         else
            throw Error("Unexpected symbol %c in pseudoatom string %s", c, str);
         bool stop = false;
         i1 = i;
         if (b == SUPERSCRIPT) {
            i1 = i-1;
            stop = true;
            newscript = SUPER;
            stop = true;
         } else if (b == WHITESPACE && a != WHITESPACE) {
            newscript = MAIN;
            stop = true;
         } else if (b == LETTER && a != LETTER) {
            newscript = MAIN;
            stop = true;
         } else if (b == DIGIT && a != DIGIT && a != SUPERSCRIPT) {
            newscript = SUB;
            stop = true;
         } else if (b == SIGN) {
            newscript = SUB;
            stop = true;
         }

         if (stop) {
            if (i1 > i0) {
               if (a == SIGN) {
                  GraphItem::TYPE type = (c == '+') ? GraphItem::PLUS : GraphItem::MINUS;
                  int id = _pushGraphItem(ad, RenderItem::RIT_CHARGESIGN, color, highlighted);
                  gis.push(id);
                  GraphItem& sign = _data.graphitems[id];
                  _cw.setGraphItemSizeSign(sign, type);

                  totalwdt += offset;
                  sign.bbp.set(xpos + totalwdt, ad.ypos + upshift * ad.height);
                  _expandBoundRect(ad, sign);
                  totalwdt += sign.bbsz.x;
               } else if (a == WHITESPACE) {
                  totalwdt += space;
               } else {
                  int id = _pushTextItem(ad, RenderItem::RIT_PSEUDO, color, highlighted);
                  tis.push(id);
                  TextItem& item = _data.textitems[id];
                  item.fontsize = (script == MAIN) ? FONT_SIZE_LABEL : FONT_SIZE_ATTR;
                  item.text.copy(str + i0, i1 - i0); 
                  item.text.push((char)0);
                  _cw.setTextItemSize(item);

                  if (cnt > 0)
                     totalwdt += offset;
                  float shift = (script == SUB) ? downshift : ((script == SUPER) ? upshift : 0);
                  item.bbp.set(xpos + totalwdt, ad.ypos + shift * ad.height);
                  _expandBoundRect(ad, item);
                  totalwdt += item.bbsz.x;
               }
               cnt++;
            }
            i0 = i;
            if (b == SUPERSCRIPT)
               ++i0;
         }
         a = b;
         script = newscript;
      }
   }
   ad.rightMargin += totalwdt;
   if (ad.shiftLeft) {
      float dx = totalwdt - width;
      for (int i = 0; i < tis.size(); ++i)
         _data.textitems[tis[i]].bbp.x -= dx;
      for (int i = 0; i < gis.size(); ++i)
         _data.graphitems[gis[i]].bbp.x -= dx;
      ad.leftMargin -= dx;
      ad.rightMargin -= dx;
   }
}

void MoleculeRenderInternal::_prepareLabelText (int aid)
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


   if (ad.type == AtomDesc::TYPE_PSEUDO) {
      _preparePseudoAtom(aid, CWC_BASE, highlighted);
   } else if (ad.showLabel) {
      tilabel = _pushTextItem(ad, RenderItem::RIT_LABEL, color, highlighted);
      {
         TextItem& label = _data.textitems[tilabel];
         label.fontsize = FONT_SIZE_LABEL;
         ArrayOutput output(label.text);
         if (ad.type == AtomDesc::TYPE_REGULAR)
            if (ad.label == ELEM_H && isotope == 2)
               output.printf("D");
            else if (ad.label == ELEM_H && isotope == 3)
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

      // isotope
      if (isotope > 0 && (ad.label != ELEM_H || 
         isotope > 3 || isotope < 2)) {
         tiIsotope = _pushTextItem(ad, RenderItem::RIT_ISOTOPE, color, highlighted);

         TextItem& itemIsotope = _data.textitems[tiIsotope];
         itemIsotope.fontsize = FONT_SIZE_ATTR;
         bprintf(itemIsotope.text, "%i", isotope);
         _cw.setTextItemSize(itemIsotope);

         ad.leftMargin -= _settings.labelInternalOffset + itemIsotope.bbsz.x;
         itemIsotope.bbp.set(ad.leftMargin, ad.ypos + 
            _settings.upperIndexShift * ad.height);
         _expandBoundRect(ad, itemIsotope);
      }

      // implicit hydrogens preparation
      bool isTerminal = bm.getVertex(aid).degree() <= 1;
      bool isHetero = ad.label != ELEM_C;
      bool showImplHydrogens = false;
      switch (_opt.implHMode)
      {
      case IHM_NONE:
         break;
      case IHM_TERMINAL:
         showImplHydrogens = isTerminal;
         break;
      case IHM_HETERO:
         showImplHydrogens = isHetero;
         break;
      case IHM_TERMINAL_HETERO:
         showImplHydrogens = isHetero || isTerminal;
         break;
      case IHM_ALL:
         showImplHydrogens = true;
         break;
      }

      // hydrogen drawing
      ad.showHydro = false;
      if (!bm.isQueryMolecule()) {
         int implicit_h = 0;

         if (!bm.isRSite(aid) && !bm.isPseudoAtom(aid))
            implicit_h = bm.asMolecule().getImplicitH(aid);
         if (implicit_h > 0 && showImplHydrogens)
         {
            ad.showHydro = true;

            tihydro = _pushTextItem(ad, RenderItem::RIT_HYDROGEN, color, highlighted);
            float hydrogenGroupWidth = 0;
            {

               TextItem& itemHydrogen = _data.textitems[tihydro];
               itemHydrogen.fontsize = FONT_SIZE_LABEL;
               bprintf(itemHydrogen.text, "H");
               _cw.setTextItemSize(itemHydrogen);
               hydrogenGroupWidth = itemHydrogen.bbsz.x + _settings.labelInternalOffset;
            }

            if (implicit_h > 1)
            {
               tiHydroIndex = _pushTextItem(ad, RenderItem::RIT_HYDROINDEX, 
                  color, highlighted);

               TextItem& itemHydroIndex = _data.textitems[tiHydroIndex];
               itemHydroIndex.fontsize = FONT_SIZE_ATTR;
               bprintf(itemHydroIndex.text, "%i", implicit_h);
               _cw.setTextItemSize(itemHydroIndex);
               hydrogenGroupWidth += itemHydroIndex.bbsz.x + _settings.labelInternalOffset;
            }

            // take new reference, old one may be corrupted after adding 'tiHydroIndex'
            TextItem& itemHydrogen = _data.textitems[tihydro];
            if (ad.shiftLeft)
            {
               ad.leftMargin -= hydrogenGroupWidth;
               itemHydrogen.bbp.set(ad.leftMargin, ad.ypos);
            }
            else
            {
               ad.rightMargin += _settings.labelInternalOffset;
               itemHydrogen.bbp.set(ad.rightMargin, ad.ypos);
               ad.rightMargin += hydrogenGroupWidth;
            }
            _expandBoundRect(ad, itemHydrogen);
            if (tiHydroIndex > 0)
            {
               _data.textitems[tiHydroIndex].bbp.set(itemHydrogen.bbp.x + itemHydrogen.bbsz.x + _settings.labelInternalOffset, itemHydrogen.bbp.y + _settings.lowerIndexShift * itemHydrogen.bbsz.y);
               _expandBoundRect(ad, _data.textitems[tiHydroIndex]);
            }
         }
      }

      // charge
      int charge = bm.getAtomCharge(aid);
      if (charge != CHARGE_UNKNOWN && charge != 0) {
         ad.rightMargin += _settings.labelInternalOffset;
         if (abs(charge) != 1)
         {
            tiChargeValue = _pushTextItem(ad, RenderItem::RIT_CHARGEVAL, color, highlighted);

            TextItem& itemChargeValue = _data.textitems[tiChargeValue];
            itemChargeValue.fontsize = FONT_SIZE_ATTR;
            bprintf(itemChargeValue.text, "%i", abs(charge));
            _cw.setTextItemSize(itemChargeValue);

            itemChargeValue.bbp.set(ad.rightMargin, ad.ypos + _settings.upperIndexShift * ad.height);
            _expandBoundRect(ad, itemChargeValue);
            ad.rightMargin += itemChargeValue.bbsz.x;
         }

         GraphItem::TYPE type = charge > 0 ? GraphItem::PLUS : GraphItem::MINUS;
         giChargeSign = _pushGraphItem(ad, RenderItem::RIT_CHARGESIGN, color, highlighted);

         GraphItem& itemChargeSign = _data.graphitems[giChargeSign];
         _cw.setGraphItemSizeSign(itemChargeSign, type);

         itemChargeSign.bbp.set(ad.rightMargin, ad.ypos + _settings.upperIndexShift * ad.height);
         _expandBoundRect(ad, itemChargeSign);
         ad.rightMargin += itemChargeSign.bbsz.x;
      }

      // valence
      int valence = bm.getExplicitValence(aid);
      if (_opt.showValences && valence >= 0) {
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
      int radical = bm.getAtomRadical(aid);
      if (radical > 0)
      {
         const TextItem& label = _data.textitems[tilabel];
         Vec2f ltc(label.bbp);

         if (radical == RADICAL_DOUPLET)
         {
            giRadical = _pushGraphItem(ad, RenderItem::RIT_RADICAL, color, highlighted);
            GraphItem& itemRadical = _data.graphitems[giRadical];
            _cw.setGraphItemSizeDot(itemRadical);

            if (!(ad.showHydro && !ad.shiftLeft) && giChargeSign < 0 && tiValence < 0)
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
            else //if (radical == RADICAL_TRIPLET)
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
   if ((ad.stereoGroupType > 0 && ad.stereoGroupType != MoleculeStereocenters::ATOM_ANY) || (_data.inversions.size() > 0 && _data.inversions[aid] != STEREO_UNMARKED)) {
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
      if (_data.inversions.size() > 0 && _data.inversions[aid] != STEREO_UNMARKED)
      {
         if (itemOutput.tell() > 0)
            itemOutput.printf(",");
         itemOutput.printf("%s", _data.inversions[aid] == STEREO_INVERTS ? "Inv" : "Ret");
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
            ad.pos.y + ad.boundBoxMin.y - itemStereoGroup.bbsz.y - 
            _settings.stereoGroupLabelOffset);
         else
            itemStereoGroup.bbp.set(ad.pos.x - itemStereoGroup.bbsz.x / 2, 
            ad.pos.y + ad.boundBoxMax.y + 
            _settings.stereoGroupLabelOffset);
      }
      else
      {
         // label hidden - position stereo group label independently
         Vec2f p;
         bondEndRightToStereoGroupLabel = _findClosestBox(p, aid, 
            itemStereoGroup.bbsz, _settings.bondLineWidth);

         p.addScaled(itemStereoGroup.bbsz, -0.5);
         itemStereoGroup.bbp.copy(p);
      }
      _expandBoundRect(ad, itemStereoGroup);
   }

   // prepare AAM labels
   if (_data.aam.size() > 0 && _data.aam[aid] > 0) {
      //_opt.aamColor
      int tiAAM = _pushTextItem(ad, RenderItem::RIT_AAM, CWC_BASE, false);

      TextItem& itemAAM = _data.textitems[tiAAM];
      itemAAM.fontsize = FONT_SIZE_ATTR;
      bprintf(itemAAM.text, "%i", abs(_data.aam[aid]));
      _cw.setTextItemSize(itemAAM);

      if (ad.showLabel)
      {
         ad.leftMargin -= itemAAM.bbsz.x + _settings.labelInternalOffset;
         itemAAM.bbp.set(ad.leftMargin, ad.ypos + 
            _settings.lowerIndexShift * ad.height);
      }
      else
      {
         Vec2f p;
         _findClosestBox(p, aid, itemAAM.bbsz, _settings.bondLineWidth, 
            bondEndRightToStereoGroupLabel);

         p.addScaled(itemAAM.bbsz, -0.5);
         itemAAM.bbp.copy(p);
      }
      _expandBoundRect(ad, itemAAM);
   }

   // prepare R-group attachment point labels
   QS_DEF(Array<int>, rGroupAttachmentIndices);
   QUERY_MOL_BEGIN(_mol);
   if (ad.isRGroupAttachmentPoint) {
      rGroupAttachmentIndices.clear();
      MoleculeRGroupFragment& rfragment = qmol.getRGroupFragment();
      for (int i = 0; i < rfragment.attachmentPointCount(); ++i)
         for (int j = 0, k; (k = rfragment.getAttachmentPoint(i, j)) >= 0; ++j)
            if (k == aid)
               rGroupAttachmentIndices.push(i);

      int tiAttachmentPoint = _pushTextItem(ad, 
         RenderItem::RIT_ATTACHMENTPOINT, CWC_BASE, false);
      TextItem& itemAttachmentPoint = _data.textitems[tiAttachmentPoint];
      itemAttachmentPoint.fontsize = FONT_SIZE_LABEL;
      ArrayOutput output(itemAttachmentPoint.text);
      for (int i = 0; i < rGroupAttachmentIndices.size(); ++i)
      {
         if (i != 0)
            output.printf(" ");
         output.printf("*");
         for (int j = 0; j < rGroupAttachmentIndices[i]; ++j)
            output.printf("'");
      }
      output.writeChar(0);
      _cw.setTextItemSize(itemAttachmentPoint);

      const Vertex& v = bm.getVertex(aid);
      Vec2f d;
      if (v.degree() == 0) {
         d.set(0, -1);
      } else {
         int di = -1;
         float angle = -1;
         for (int i = v.neiBegin(); i < v.neiEnd(); i = v.neiNext(i))
         {
            float a = _getBondEnd(aid, i).lang;
            if (a > angle)
            {
               angle = a;
               di = i;
            }
         }
         BondEnd& be = _getBondEnd(aid, di);
         d.copy(be.dir);
         d.rotateL(be.lang/2);
      }
      d.scale(0.3f);
      d.add(ad.pos);
      d.addScaled(itemAttachmentPoint.bbsz, -0.5);
      itemAttachmentPoint.bbp.copy(d);
   }
   QUERY_MOL_END;

   // prepare atom id's
   if (_opt.showAtomIds)
   {
      tiindex = _pushTextItem(ad, RenderItem::RIT_ATOMID, CWC_BLUE, false);

      TextItem& index = _data.textitems[tiindex];
      index.fontsize = FONT_SIZE_INDICES;

      bprintf(index.text, "%i", aid);
      _cw.setTextItemSize(index, ad.pos);

      if (ad.showLabel)
         index.bbp.set(ad.rightMargin + _settings.labelInternalOffset, ad.ypos + 0.5f * ad.height);
   }
}

int MoleculeRenderInternal::_pushTextItem (RenderItem::TYPE ritype, int color, bool highlighted)
{
   _data.textitems.push();
   _data.textitems.top().clear();
   _data.textitems.top().ritype = ritype;
   _data.textitems.top().color = color;
   _data.textitems.top().highlighted = highlighted;
   return _data.textitems.size() - 1;
}

int MoleculeRenderInternal::_pushTextItem (AtomDesc& ad, RenderItem::TYPE ritype, int color, bool highlighted)
{
   int res = _pushTextItem(ritype, color, highlighted);
   if (ad.tibegin < 0)
      ad.tibegin = res;
   ad.ticount++;
   return res;
}

int MoleculeRenderInternal::_pushGraphItem (RenderItem::TYPE ritype, int color, bool highlighted)
{
   _data.graphitems.push();
   _data.graphitems.top().clear();
   _data.graphitems.top().ritype = ritype;
   _data.graphitems.top().color = color;
   _data.graphitems.top().highlighted = highlighted;
   return _data.graphitems.size() - 1;
}

int MoleculeRenderInternal::_pushGraphItem (AtomDesc& ad, RenderItem::TYPE ritype, int color, bool highlighted)
{
   int res = _pushGraphItem(ritype, color, highlighted);
   if (ad.gibegin < 0)
      ad.gibegin = res;
   ad.gicount++;
   return res;
}

const char* MoleculeRenderInternal::_valenceText (const int valence)
{
   const char* vt[] = {"(I)", "(II)", "(III)", "(IV)", "(V)", "(VI)", "(VII)", "(VIII)", "(IX)", "(X)", "(XI)", "(XII)", "(XIII)"};
   if (valence < 1 || valence - 1 >= NELEM(vt))
      throw Error("valence value %i out of range", valence);
   return vt[valence-1];
}

void MoleculeRenderInternal::_drawBond (int b)
{
   BondDescr& bd = _bd(b);
   const BondEnd& be1 = _be(bd.be1);
   const BondEnd& be2 = _be(bd.be2);

   _cw.setLineWidth(_settings.bondLineWidth);

   _cw.setSingleSource(CWC_BASE);
   if (_edgeIsHighlighted(b))
      _cw.setHighlight();

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
   default:
      switch (bd.queryType)
      {
      case QueryMolecule::QUERY_BOND_ANY:
         _bondAny(bd, be1, be2);
         break;
      case QueryMolecule::QUERY_BOND_SINGLE_OR_DOUBLE:
         _bondSingleOrDouble(bd, be1, be2);
         break;
      case QueryMolecule::QUERY_BOND_DOUBLE_OR_AROMATIC:
         _bondDoubleOrAromatic(bd, be1, be2);
         break;
      case QueryMolecule::QUERY_BOND_SINGLE_OR_AROMATIC:
         _bondSingleOrAromatic(bd, be1, be2);
         break;
      default:
         throw Error("Unknown type");
      }
   }

   _cw.resetHighlightThickness();     

   if (_data.reactingCenters.size() > 0 && _data.reactingCenters[b] != RC_UNMARKED)
   {                          
      int rc = _data.reactingCenters[b];
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

   if (bd.topology > 0)
      _drawTopology(bd);
}

void MoleculeRenderInternal::_drawTopology (BondDescr& bd)
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
   float shift = (fabs(bd.norm.x * ti.bbsz.x)+fabs(bd.norm.y * ti.bbsz.y)) / 2 + _settings.bondLineWidth;

   if (bd.extP < bd.extN)
      shift = shift + bd.extP;
   else
      shift = -shift - bd.extN;
   Vec2f c;
   c.copy(bd.center);
   c.addScaled(bd.norm, shift);
   c.addScaled(ti.bbsz, -0.5f);
   ti.bbp.copy(c);
   _cw.drawTextItemText(ti);
}

void MoleculeRenderInternal::_drawReactingCenter (BondDescr& bd, int rc)
{
   const int rcNumPnts = 8;
   Vec2f p[rcNumPnts];
   for (int i = 0; i < rcNumPnts; ++i)
      p[i].copy(bd.center);
   float alongIntRc = _settings.bondLineWidth, // half interval along for RC_CENTER
      alongIntMadeBroken = 2 * _settings.bondLineWidth, // half interval between along for RC_MADE_OR_BROKEN
      alongSz = 1.5f * _settings.bondSpace, // half size along for RC_CENTER
      acrossInt = 1.5f * _settings.bondSpace, // half interval across for RC_CENTER
      acrossSz = 3.0f * _settings.bondSpace, // half size across for all
      tiltTan = 0.2f, // tangent of the tilt angle
      radius = _settings.bondSpace; // radius of the circle for RC_UNCHANGED
   int numLines = 0;

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
   case RC_CENTER:  // #
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
   case RC_UNCHANGED:  // o   
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
      bd.extN = __max(bd.extN, radius);
      bd.extP = __max(bd.extP, radius);
   }
   else
   {
      bd.extN = __max(bd.extN, acrossSz);
      bd.extP = __max(bd.extP, acrossSz);
   }                                     
}

void MoleculeRenderInternal::_bondSingle (BondDescr& bd, const BondEnd& be1, const BondEnd& be2)
{
   Vec2f l(be2.p), r(be2.p);
   float w = _settings.bondSpace + _settings.bondLineWidth;
   l.addScaled(bd.norm, -w);
   r.addScaled(bd.norm, w);
   bd.extP = bd.extN = w;

   Vec2f dd;
   dd.diff(be2.p, be1.p);
   float len = dd.length();

   float lw = _cw.currentLineWidth();

   int stripeCnt = (int)((len) / lw / 2);

   if (bd.stereodir == 0)
   {
      _cw.drawLine(be1.p, be2.p);
      bd.extP = bd.extN = lw / 2;
   }
   else if (bd.stereodir == MoleculeStereocenters::BOND_UP)
   {
      if (_ad(be2.aid).showLabel == false && !bd.isShort)
      {
         float tgb = w / len;
         float csb = sqrt(1 / (1 + tgb * tgb));
         float snb = tgb * csb;
         float tga, ttl=0.0, ttr=0.0;
         bool adjustRight = be2.lsin > 0 && be2.lcos < 0.9f && be2.lcos > -0.9f && fabs(be2.lcos) > 0.001f,
            adjustLeft = be2.rsin > 0 && be2.rcos < 0.9f && be2.rcos > -0.9f && fabs(be2.rcos) > 0.001f;

         if (adjustRight && !_bd(_be(be2.lnei).bid).isShort)
         {
            tga = be2.lsin / be2.lcos;
            const BondDescr& nbd = _bd(_be(be2.lnei).bid);
            if (nbd.type == BOND_DOUBLE && nbd.centered)
               ttl = (len * be2.lsin - _settings.bondSpace) / (snb * be2.lcos + csb * be2.lsin);
            else
               ttl = len * csb * (1 + tgb * (tgb * tga - 1) / (tgb + tga));
            r.copy(be1.dir);
            r.scale(ttl);
            r.rotateL(-snb, csb);
            r.add(be1.p);
         }

         if (adjustLeft && !_bd(_be(be2.rnei).bid).isShort)
         {
            tga = be2.rsin / be2.rcos;
            const BondDescr& nbd = _bd(_be(be2.rnei).bid);
            if (nbd.type == BOND_DOUBLE && nbd.centered)
               ttr = (len * be2.rsin - _settings.bondSpace) / (snb * be2.rcos + csb * be2.rsin);
            else
               ttr = len * csb * (1 + tgb * (tgb * tga - 1) / (tgb + tga));
            l.copy(be1.dir);
            l.scale(ttr);
            l.rotateL(snb, csb);
            l.add(be1.p);
         }
         _cw.fillQuad(be1.p, r, be2.p, l);
      }
      else
      {
         _cw.drawTriangle(be1.p, r, l);
      }
   }
   else if (bd.stereodir == MoleculeStereocenters::BOND_DOWN)
      _cw.drawTriangleStripes(be1.p, r, l, stripeCnt);
   else if (bd.stereodir == MoleculeStereocenters::BOND_EITHER)
      _cw.drawTriangleZigzag(be1.p, r, l, stripeCnt);
   else
      throw Error("Unknown single bond stereo type");
}

void MoleculeRenderInternal::_bondAny (BondDescr& bd, const BondEnd& be1, const BondEnd& be2)
{
   _cw.setDash(_settings.bondDashAny, Vec2f::dist(be1.p, be2.p));
   _cw.drawLine(be1.p, be2.p);
   _cw.resetDash();
   bd.extP = bd.extN = _settings.bondLineWidth / 2;
}

float MoleculeRenderInternal::_ctghalf (float cs)
{
   return sqrt(1 - cs * cs) / (1 - cs);
}

float MoleculeRenderInternal::_doubleBondShiftValue (const BondEnd& be, bool right, bool centered)
{
   const BondDescr& bd = _bd(_be(right ? be.rnei : be.lnei).bid);
   float 
      si = right ? be.rsin : be.lsin, 
      co = right ? be.rcos : be.lcos;
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

void MoleculeRenderInternal::_prepareDoubleBondCoords (Vec2f* coord, BondDescr& bd, const BondEnd& be1, const BondEnd& be2, bool allowCentered)
{
   Vec2f ns, ds;
   ns.scaled(bd.norm, 2 * _settings.bondSpace);

   if (allowCentered && bd.centered)
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
      bd.extP = _settings.bondSpace * 2 + _settings.bondLineWidth / 2;
      bd.extN = _settings.bondLineWidth / 2;
     
      if (!bd.lineOnTheRight)
      {
         float t;
         __swap(bd.extP, bd.extN, t);
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

void MoleculeRenderInternal::_drawStereoCareBox (BondDescr& bd, const BondEnd& be1, const BondEnd& be2)
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
      bd.extP = bd.extN = _settings.stereoCareBoxSize / 2 + _settings.bondLineWidth / 2;
      if (!bd.centered)
      {
         float shift = Vec2f::dot(ns,bd.norm);
         bd.extP += shift;
         bd.extN -= shift;
         p0.add(ns);
      }
      p1.lineCombin(p0, bd.dir, _settings.stereoCareBoxSize);
      p2.lineCombin(p1, bd.norm, _settings.stereoCareBoxSize);
      p3.lineCombin(p0, bd.norm, _settings.stereoCareBoxSize);

      _cw.drawQuad(p0, p1, p2, p3);

   }
}                            

void MoleculeRenderInternal::_bondDouble (BondDescr& bd, const BondEnd& be1, const BondEnd& be2)
{                  
   Vec2f coord[4];
   _prepareDoubleBondCoords(coord, bd, be1, be2, true);
   _cw.drawLine(coord[0], coord[1]);
   _cw.drawLine(coord[2], coord[3]);
   
   _drawStereoCareBox(bd, be1, be2);
}

void MoleculeRenderInternal::_bondSingleOrAromatic (BondDescr& bd, const BondEnd& be1, const BondEnd& be2)
{                  
   Vec2f coord[4];
   _prepareDoubleBondCoords(coord, bd, be1, be2, true);
   _cw.drawLine(coord[0], coord[1]);
   _cw.setDash(_settings.bondDashSingleOrAromatic);
   _cw.drawLine(coord[2], coord[3]);
   _cw.resetDash();

   _drawStereoCareBox(bd, be1, be2);
}

void MoleculeRenderInternal::_bondDoubleOrAromatic (BondDescr& bd, const BondEnd& be1, const BondEnd& be2)
{                  
   Vec2f coord[4];
   _prepareDoubleBondCoords(coord, bd, be1, be2, true);
   _cw.setDash(_settings.bondDashDoubleOrAromatic);
   _cw.drawLine(coord[0], coord[1]);
   _cw.drawLine(coord[2], coord[3]);
   _cw.resetDash();

   _drawStereoCareBox(bd, be1, be2);
}


void MoleculeRenderInternal::_bondSingleOrDouble (BondDescr& bd, const BondEnd& be1, const BondEnd& be2)
{
   Vec2f ns, ds;
   ns.scaled(bd.norm, 2 * _settings.bondSpace);

   ds.diff(be2.p, be1.p);
   float len = ds.length();
   ds.normalize();

   // Get number of segments of single-or-double bond
   // An average bond in our coordinates has length 1. We want an average bond to have 5 segments, like -=-=-
   // For longer bond more segments may be necessary, for shorter one - less, but not less then 3 segments, like -=-
   int numSegments = __max((int)(len / 0.4f), 1) * 2 + 1;

   Vec2f r0, r1, p0, p1, q0, q1;
   float step = len / numSegments;
   ns.scale(0.5f);
   for (int i = 0; i < numSegments; ++i)   
   {                    
      r0.lineCombin(be1.p, ds, i * step);
      r1.lineCombin(be1.p, ds, (i + 1) * step);
      if (i & 1) {
         p0.sum(r0, ns);
         p1.sum(r1, ns);
         q0.diff(r0, ns);
         q1.diff(r1, ns);
         _cw.drawLine(p0, p1);
         _cw.drawLine(q0, q1);
      } else
         _cw.drawLine(r0, r1);
   }
}

void MoleculeRenderInternal::_bondAromatic (BondDescr& bd, const BondEnd& be1, const BondEnd& be2)
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

void MoleculeRenderInternal::_bondTriple (BondDescr& bd, const BondEnd& be1, const BondEnd& be2)
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
