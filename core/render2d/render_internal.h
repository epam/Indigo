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

#ifndef __render_internal_h__
#define __render_internal_h__

#include "base_cpp/tree.h"
#include "molecule/ket_commons.h"

#include "render_common.h"

namespace indigo
{

    class RenderContext;

    class MoleculeRenderInternal
    {
    public:
        MoleculeRenderInternal(const RenderOptions& opt, const RenderSettings& settings, RenderContext& cw, bool idle);
        ~MoleculeRenderInternal();
        void setMolecule(BaseMolecule* mol);
        void setIsRFragment(bool isRFragment);
        void setScaleFactor(const float scaleFactor, const Vec2f& min, const Vec2f& max);
        void render();

        void setReactionComponentProperties(const Array<int>* aam, const Array<int>* reactingCenters, const Array<int>* inversions);
        void setQueryReactionComponentProperties(const Array<int>* exactChanges);

        DECL_ERROR;

    private:
        bool _idle = false;

        enum STEREOGROUPS_MODE
        {
            STEREOGROUPS_SHOW,
            STEREOGROUPS_HIDE
        };
        struct LocalOptions
        {
            STEREOGROUPS_MODE stereoMode;
        };

        BondEnd& _be(int beid);
        const BondEnd& _be(int beid) const;
        BondDescr& _bd(int bid);
        const BondDescr& _bd(int bid) const;
        AtomDesc& _ad(int aid);
        const AtomDesc& _ad(int aid) const;
        void _checkSettings();
        void _extendRenderItem(RenderItem& item, const float extent);
        bool _clipRaySegment(float& offset, const Vec2f& p, const Vec2f& d, const Vec2f& n0, const Vec2f& a, const Vec2f& b, const float w);
        bool _clipRayBox(float& offset, const Vec2f& p, const Vec2f& d, const Vec2f& rp, const Vec2f& sz, const float w);
        void _findMinMax();
        void _objCoordTransform(Vec2f& p, const Vec2f& v) const;
        void _objDistTransform(Vec2f& p, const Vec2f& v) const;
        void _initCoordinates();
        void _determineDoubleBondShift();
        void _determineStereoGroupsMode();
        static const char* _getStereoGroupText(int type);
        bool _ringHasSelfIntersectionsSimple(const Ring& ring);
        bool _ringHasSelfIntersections(const Ring& ring);
        void _findRings();
        void _prepareLabels();
        void _rotateHalfCenteredBonds();
        bool _isSingleHighlighted(int aid);
        bool _vertexIsHighlighted(int aid);
        bool _edgeIsHighlighted(int bid);
        bool _hasQueryModifiers(int aid);
        void _findNearbyAtoms();
        void _initHydroPos(int aid);
        int _hydroPosFindConflict(int i);
        bool _hydroPosCorrectGreedy();
        void _hydroPosCorrectRepulse();
        void _initAtomData();
        void _initRGroups();
        void _loadBrackets(Sgroup& sg, const Array<Vec2f[2]>& coord);
        void _placeBrackets(Sgroup& sg, const Array<int>& atoms, Array<Vec2f[2]>& brackets);
        void _positionIndex(Sgroup& sg, int ti, bool lower);
        void _loadBracketsAuto(const SGroup& group, Sgroup& sg);
        void _convertCoordinate(const Array<Vec2f[2]>& original, Array<Vec2f[2]>& converted);
        void _adjustBrackets(const Array<Vec2f[2]>& converted, Array<Vec2f[2]>& placed);

        void _prepareSGroups(bool collapseAtLeastOneSuperatom = false);
        void _initSGroups(Tree& sgroups, Rect2f parent);
        void _initSGroups();

        void _findAnglesOverPi();
        void _renderBondIds();
        void _renderAtomIds();
        void _renderEmptyRFragment();
        void _renderLabels();
        void _renderRings();
        void _renderSGroups();
        void _setHighlightOpt();
        void _resetHighlightOpt();
        void _renderBonds();
        void _applyBondOffset();
        void _setBondCenter();
        float _getBondOffset(int aid, const Vec2f& pos, const Vec2f& dir, const float bondWidth);
        void _calculateBondOffset();
        void _findNeighbors();
        void _findCenteredCase();
        void _initBondData();
        void _initBondEndData();
        void _initBoldStereoBonds();
        void _extendRenderItems();
        BondEnd& _getBondEnd(int aid, int nei);
        int _getBondEndIdx(int aid, int nei);
        int _getOpposite(int beid) const;
        void _drawAtom(const AtomDesc& desc);
        void _writeQueryAtomToString(Output& output, int aid);
        bool _writeDelimiter(bool needDelimiter, Output& output);
        void _writeQueryModifier(Output& output, int aid);
        int _findClosestCircle(Vec2f& p, int aid, float radius, int skip = -1);
        int _findClosestBox(Vec2f& p, int aid, const Vec2f& sz, float mrg, int skip = -1);
        void _preparePseudoAtom(int aid, int color, bool highlighted);
        void _prepareChargeLabel(int aid, int color, bool highlighted);
        void _prepareLabelText(int aid);
        void _prepareAAM();
        int _pushTextItem(RenderItem::TYPE type, int color, bool highlighted);
        int _pushTextItem(AtomDesc& ad, RenderItem::TYPE type, int color, bool highlighted);
        int _pushTextItem(Sgroup& sg, RenderItem::TYPE ritype, int color = CWC_BASE);
        int _pushGraphItem(RenderItem::TYPE type, int color, bool highlighted);
        int _pushGraphItem(AtomDesc& ad, RenderItem::TYPE type, int color, bool highlighted);
        int _pushGraphItem(Sgroup& ad, RenderItem::TYPE type, int color = CWC_BASE);
        const char* _valenceText(const int valence);
        float _ctghalf(float cs);
        void _drawBond(int b);
        void _drawTopology(BondDescr& bd);
        void _drawReactingCenter(BondDescr& bd, int rc);
        float _doubleBondShiftValue(const BondEnd& be, bool right, bool centered);
        void _prepareDoubleBondCoords(Vec2f* coord, BondDescr& bd, const BondEnd& be1, const BondEnd& be2, bool allowCentered);
        void _drawStereoCareBox(BondDescr& bd, const BondEnd& be1, const BondEnd& be2);
        double _getAdjustmentFactor(const int aid, const int anei, const double acos, const double asin, const double tgb, const double csb, const double snb,
                                    const double len, const double w, double& csg, double& sng);
        void _adjustAngle(Vec2f& l, const BondEnd& be1, const BondEnd& be2, bool left);
        void _bondBoldStereo(BondDescr& bd, const BondEnd& be1, const BondEnd& be2);
        void _bondSingle(BondDescr& bd, const BondEnd& be1, const BondEnd& be2);
        void _bondHydrogen(BondDescr& bd, const BondEnd& be1, const BondEnd& be2);
        void _bondCoordination(BondDescr& bd, const BondEnd& be1, const BondEnd& be2);
        void _bondDouble(BondDescr& bd, const BondEnd& be1, const BondEnd& be2);
        void _bondSingleOrAromatic(BondDescr& bd, const BondEnd& be1, const BondEnd& be2);
        void _bondDoubleOrAromatic(BondDescr& bd, const BondEnd& be1, const BondEnd& be2);
        void _bondSingleOrDouble(BondDescr& bd, const BondEnd& be1, const BondEnd& be2);
        void _bondAromatic(BondDescr& bd, const BondEnd& be1, const BondEnd& be2);
        void _bondTriple(BondDescr& bd, const BondEnd& be1, const BondEnd& be2);
        void _bondAny(BondDescr& bd, const BondEnd& be1, const BondEnd& be2);
        int _parseColorString(Scanner& str, float& r, float& g, float& b);

        void _cloneAndFillMappings();
        void _precalcScale();

        // TODO: remove dublicate with _placeBrackets(..)
        inline Rect2f _bound(Array<int>& atoms) const
        {
            const int n = atoms.size();
            if (n <= 0)
            {
                return Rect2f(Vec2f(0, 0), Vec2f(0, 0));
            }
            Array<Vec2f> points;
            points.resize(n);
            for (int i = 0; i < n; i++)
            {
                points[i] = _ad(atoms[i]).pos;
            }
            return _bound(points, 0, n - 1);
        }

        Rect2f _bound(Array<Vec2f>& points, int l, int r) const
        {
            if (r == l || r == l + 1)
            {
                return Rect2f(points[l], points[r]);
            }
            int m = (l + r) / 2;
            return Rect2f(_bound(points, l, m), _bound(points, m + 1, r));
        }

        inline Vec2f _firstPosition(Array<int>& atoms)
        {
            return _ad(atoms[0]).pos;
        }

        inline static Vec2f ILLEGAL_POINT()
        {
            return Vec2f(nanf(""), nanf(""));
        }

        // TODO: eliminate
        inline static Rect2f ILLEGAL_RECT()
        {
            return Rect2f(ILLEGAL_POINT(), ILLEGAL_POINT());
        }

        inline static bool IS_NAN(float x)
        {
            return x != x;
        }
        inline static bool IS_ILLEGAL(Vec2f point)
        {
            return IS_NAN(point.x) && IS_NAN(point.y);
        }
        inline static bool IS_ILLEGAL(Rect2f rect)
        {
            return IS_ILLEGAL(rect.leftBottom()) && IS_ILLEGAL(rect.rightTop());
        }

        // local
        void* _hdc;
        BaseMolecule* _mol;
        bool _own_mol = false;
        RenderContext& _cw;
        float _scale;
        Vec2f _min, _max;
        LocalOptions _lopt;
        bool isRFragment;
        const RenderSettings& _settings;
        const RenderOptions& _opt;
        CP_DECL;
        TL_CP_DECL(MoleculeRenderData, _data);
        TL_CP_DECL(Array<int>, _atomMapping);
        TL_CP_DECL(Array<int>, _atomMappingInv);
        TL_CP_DECL(BaseMolecule::Mapping, _bondMappingInv);
    };

} // namespace indigo

#endif //__render_internal_h__
