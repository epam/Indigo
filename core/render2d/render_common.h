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

#ifndef __render_common_h__
#define __render_common_h__

#include "base_cpp/tlscont.h"
#include "graph/graph.h"
#include "layout/metalayout.h"
#include "molecule/elements.h"
#include "reaction/base_reaction.h"
#include "render_cdxml.h"

typedef void* PVOID;

namespace indigo
{

    struct Vec2f;
    struct Edge;
    class BaseMolecule;
    class BaseReaction;
    class BaseMolecule;
    class Reaction;
    class QueryMolecule;
    class QueryReaction;
    class Output;
    class RenderOptions;
    struct AcsOptions;

    enum DINGO_MODE
    {
        MODE_NONE,
        MODE_PDF,
        MODE_PNG,
        MODE_SVG,
        MODE_EMF,
        MODE_HDC,
        MODE_PRN,
        MODE_CDXML
    };
    enum STEREO_STYLE
    {
        STEREO_STYLE_EXT,
        STEREO_STYLE_OLD,
        STEREO_STYLE_NONE
    };
    enum
    {
        CWC_BASE = -2,
        CWC_WHITE = 0,
        CWC_BLACK,
        CWC_RED,
        CWC_GREEN,
        CWC_BLUE,
        CWC_DARKGREEN,
        CWC_COUNT
    };
    enum FONT_SIZE
    {
        FONT_SIZE_LABEL = 0,
        FONT_SIZE_ATTR,
        FONT_SIZE_RGROUP_LOGIC,
        FONT_SIZE_RGROUP_LOGIC_INDEX,
        FONT_SIZE_INDICES,
        FONT_SIZE_ATTACHMENT_POINT_INDEX,
        FONT_SIZE_RSITE_ATTACHMENT_INDEX,
        FONT_SIZE_COMMENT,
        FONT_SIZE_TITLE,
        FONT_SIZE_DATA_SGROUP,
        FONT_SIZE_COUNT /*must be the last*/
    };
    enum COMMENT_POS
    {
        COMMENT_POS_TOP,
        COMMENT_POS_BOTTOM
    };
    enum HYDRO_POS
    {
        HYDRO_POS_RIGHT = 0,
        HYDRO_POS_UP,
        HYDRO_POS_DOWN,
        HYDRO_POS_LEFT
    };

    // cos(a) to cos(a/2)
    double cos2c(const double cs);
    // cos(a) to sin(a/2)
    double sin2c(const double cs);
    // cos(a) to tg(a/2)
    double tg2c(const double cs);
    // cos(a) to ctg(a/2)
    double ctg2c(const double cs);

    struct RenderItem
    {
        enum TYPE
        {
            RIT_NULL = 0,
            RIT_LABEL,
            RIT_PSEUDO,
            RIT_HYDROGEN,
            RIT_HYDROINDEX,
            RIT_ISOTOPE,
            RIT_CHARGESIGN,
            RIT_CHARGEVAL,
            RIT_RADICAL,
            RIT_STEREOGROUP,
            RIT_VALENCE,
            RIT_AAM,
            RIT_CHIRAL,
            RIT_ATTACHMENTPOINT,
            RIT_ATOMID,
            RIT_TOPOLOGY,
            RIT_SGROUP,
            RIT_DATASGROUP,
            RIT_COMMENT,
            RIT_TITLE
        };

        RenderItem();
        RenderItem(const RenderItem& ri);

        void clear();

        TYPE ritype;
        Vec2f bbp;    // a point to draw the text from
        Vec2f bbsz;   // width and height of the text
        Vec2f relpos; // text bearing (positive if text is entirely on the
                      // right and bottom from the bbp point)
        int color;
        bool highlighted;
        bool noBondOffset;
    };

    struct TextItem : public RenderItem
    {
        TextItem() : size(-0.0), bold(false), italic(false), script_type(0)
        {
            clear();
        }

        TextItem(const TextItem& ti) : RenderItem(ti)
        {
            text.copy(ti.text);
            fontsize = ti.fontsize;
            size = ti.size;
            bold = ti.bold;
            italic = ti.italic;
            script_type = ti.script_type;
        }

        void clear();
        Array<char> text;
        FONT_SIZE fontsize;
        double size;
        bool bold;
        bool italic;
        int script_type;
    };

    struct GraphItem : public RenderItem
    {
        enum TYPE
        {
            DOT,
            CAP,
            PLUS,
            MINUS
        };
        GraphItem()
        {
            clear();
        }
        void clear();
        TYPE type;
    };

    struct RenderItemBracket : public RenderItem
    {
        RenderItemBracket()
        {
            clear();
        }
        void clear();
        Vec2f p0, p1, q0, q1, d, n;
        float width, length;
        bool invertUpperLowerIndex;
    };

    struct RenderItemAttachmentPoint : public RenderItem
    {
        RenderItemAttachmentPoint()
        {
            clear();
        }
        void clear();
        int number;
        Vec2f p0, p1, dir;
    };

    struct RenderItemRSiteAttachmentIndex : public RenderItem
    {
        RenderItemRSiteAttachmentIndex()
        {
            clear();
        }
        void clear();
        int number;
        float radius;
    };

    struct AtomDesc
    {
        enum TYPE
        {
            TYPE_REGULAR,
            TYPE_PSEUDO,
            TYPE_QUERY
        };
        AtomDesc();
        void clear();

        int tibegin, ticount;
        int gibegin, gicount;
        int attachmentPointBegin, attachmentPointCount;
        int rSiteAttachmentIndexBegin, rSiteAttachmentIndexCount;

        int type;
        bool showLabel;
        bool showHydro;
        HYDRO_POS hydroPos;
        bool isRGroupAttachmentPoint;
        bool fixed;
        bool exactChange;
        bool pseudoAtomStringVerbose;
        Vec2f pos;
        Vec2f boundBoxMin;
        Vec2f boundBoxMax;
        Vec3f hcolor;
        bool hcolorSet;
        int label;
        int queryLabel;
        int color;
        int stereoGroupType;
        int stereoGroupNumber;
        int implicit_h;
        Array<int> list;
        Array<char> pseudo;
        Array<int> nearbyAtoms;
        int aam;
        int inversion;
        float implHPosWeights[4];
        float upperSin, lowerSin, rightSin, leftSin;

        float leftMargin, rightMargin, ypos, height;

    private:
        AtomDesc(const AtomDesc& ad);
    };

    struct Sgroup
    {
        Sgroup();
        void clear();

        int tibegin, ticount;
        int gibegin, gicount;
        int bibegin, bicount;
        bool hide_brackets;

    private:
        Sgroup(const SGroup& sg);
    };

    struct BondEnd
    {
        BondEnd();
        void clear();

        Vec2f dir;
        Vec2f lnorm;
        Vec2f p; // corrected position
        float rcos;
        float rsin;

        // these are bond end ids in the _data.bondend array, NOT neighbor indices in the molecule!
        int rnei;
        int lnei;

        float rang;
        float lcos;
        float lsin;
        int next;
        float lang;
        bool centered;
        int aid;
        int bid;
        float offset;
        bool prolong;
        int lRing;
        float width;

    private:
        BondEnd(const BondEnd& be);
    };

    struct BondDescr : public Edge
    {
        BondDescr();

        DECL_ERROR;

        void clear();

        int getBondEnd(int aid) const;

        Vec2f norm, dir, vb, ve, center;
        float thickness;
        float bahs, eahs;

        bool inRing;
        bool aromRing;
        bool stereoCare;
        bool centered;
        bool lineOnTheRight;
        bool isShort;
        int stereodir;
        bool cistrans;
        int type;
        int queryType;
        float length;
        int be1, be2;
        float extP, extN;
        int tiTopology;
        int topology;
        int reactingCenter;

    private:
        BondDescr(const BondDescr& bd);
    };

    struct Ring
    {
        Ring();
        void clear();

        Array<int> bondEnds;
        Array<float> angles;
        int dblBondCount;
        bool aromatic;
        Vec2f center;
        float radius;

    private:
        Ring(const Ring& r);
    };

    struct MoleculeRenderData
    {
        MoleculeRenderData();
        void clear();

        ObjArray<Sgroup> sgroups;
        ObjArray<AtomDesc> atoms;
        ObjArray<BondDescr> bonds;
        ObjArray<Ring> rings;
        ObjArray<BondEnd> bondends;
        ObjArray<TextItem> textitems;
        ObjArray<GraphItem> graphitems;
        ObjArray<RenderItemAttachmentPoint> attachmentPoints;
        ObjArray<RenderItemRSiteAttachmentIndex> rSiteAttachmentIndices;
        ObjArray<RenderItemBracket> brackets;
        Array<int> aam;
        Array<int> reactingCenters;
        Array<int> inversions;
        Array<int> exactChanges;

    private:
        MoleculeRenderData(const MoleculeRenderData& data);
    };

    class RenderSettings
    {
    public:
        RenderSettings();
        void init(float relativeThickness, float bondLineWidthFactor, AcsOptions* acs = nullptr);

        CP_DECL;
        TL_CP_DECL(Array<double>, bondDashAromatic);
        TL_CP_DECL(Array<double>, bondDashAny);
        TL_CP_DECL(Array<double>, bondDashSingleOrAromatic);
        TL_CP_DECL(Array<double>, bondDashDoubleOrAromatic);
        TL_CP_DECL(Array<double>, bondDashHydro);

        float labelInternalOffset;
        float lowerIndexShift;
        float unit;
        float bondLineWidth;
        float bondSpace;
        float boundExtent;
        float upperIndexShift;
        float radicalRightOffset;
        float radicalRightVertShift;
        float radicalTopOffset;
        float radicalTopDistDot;
        float radicalTopDistCap;
        float stereoGroupLabelOffset;
        float dashUnit;
        float eps;
        float stereoCareBoxSize;
        float cosineTreshold;
        float prolongAdjSinTreshold;
        float minBondLength;
        float graphItemDotRadius;
        float graphItemCapSlope;
        float graphItemCapBase;
        float graphItemCapWidth;
        float graphItemDigitWidth;
        float graphItemDigitHeight;
        float graphItemSignLineWidth;
        float graphItemPlusEdge;
        float stereoBondSpace;
        float hashSpacing = -1;

        float fzz[FONT_SIZE_COUNT];

        // Layout params, relative to average bond length units
        float layoutMarginHorizontal;
        float layoutMarginVertical;
        float plusSize;
        float metaLineWidth;
        float arrowLength;
        float arrowHeadWidth;
        float arrowHeadSize;
        float equalityInterval;
        float rGroupIfThenInterval;
        float neighboringLabelTolerance;
        float minSin;
        float neighboringAtomDistanceTresholdA;
        float neighboringAtomDistanceTresholdB;

    private:
        RenderSettings(const RenderSettings& settings);
    };

    struct MultilineTextLayout
    {
        enum Alignment
        {
            Left,
            Right,
            Center
        };

        MultilineTextLayout();
        MultilineTextLayout(Alignment bbox, Alignment inbox);

        // Text can be aligned in different ways: left, right, center. But if text has multiple
        // lines then this lines can also be aligned in different ways relative to each other
        // +-----------------------------------+--------------+
        // | View                              | Type         |
        // +===================================+==============+
        // | Line                              | left         |
        // | Quite Long line                   |              |
        // +-----------------------------------+--------------+
        // |                              Line | right        |
        // |                   Quite Long line |              |
        // +-----------------------------------+--------------+
        // |               Line                | center       |
        // |         Quite a Long line         |              |
        // +-----------------------------------+--------------+
        // |         Line                      | center-left  |
        // |         Quite a Long line         |              |
        // +-----------------------------------+--------------+

        // Alignment of the bounding box
        Alignment bbox_alignment;
        // Text alignment insdie bounding box
        Alignment inbox_alignment;

        void clear();

        // Returns values from 0.0 to 1.0 depending on the title box alignment
        float getBboxRelativeOffset() const;
        float getInboxRelativeOffset() const;
        static float getRelativeOffset(Alignment alignment);

        float getAnchorPoint(float area_x, float area_width, float text_width);
    };

    struct CanvasOptions
    {
        CanvasOptions();
        void clear();

        int width;
        int height;
        int maxWidth;
        int maxHeight;
        int xOffset;
        int yOffset;
        float bondLength; // in pixels
        UnitsOfMeasure::TYPE bondLengthUnit;
        int gridMarginX;
        int gridMarginY;
        int marginX;
        int marginY;
        int commentOffset;
        int titleOffset;
        Array<char> comment;
        Array<char> titleProp;
        COMMENT_POS commentPos;
        MultilineTextLayout commentAlign;
        MultilineTextLayout titleAlign;
        float outputSheetWidth;
        float outputSheetHeight;

        int gridColumnNumber;

    private:
        CanvasOptions(const CanvasOptions&);
    };

    class RenderOptions
    {
    public:
        RenderOptions();
        void clearRenderOptions();

        Vec3f backgroundColor;
        Vec3f baseColor;
        bool highlightThicknessEnable;
        float highlightThicknessFactor;
        bool highlightColorEnable;
        Vec3f highlightColor;
        Vec3f aamColor;
        float commentFontFactor;
        float commentSpacing;
        float titleFontFactor;
        float titleSpacing;
        Vec3f commentColor;
        Vec3f titleColor;
        Vec3f dataGroupColor;
        LABEL_MODE labelMode;
        bool highlightedLabelsVisible;
        bool boldBondDetection;
        bool implHVisible;
        DINGO_MODE mode;
        Output* output;
        PVOID hdc;
        bool showBondIds;
        bool showBondEndIds;
        bool atomBondIdsFromOne;
        bool showNeighborArcs;
        bool showAtomIds;
        bool showValences;
        bool atomColoring;
        STEREO_STYLE stereoMode;
        bool showReactingCenterUnchanged;
        bool centerDoubleBondWhenStereoAdjacent;
        bool showCycles; // for diagnostic purposes
        bool agentsBelowArrow;
        Array<char> atomColorProp;
        std::unique_ptr<RenderCdxmlContext> cdxml_context;
        // ACS settings
        float bond_length_px;
        int32_t ppi;
        float fontSize;
        UnitsOfMeasure::TYPE fontSizeUnit;
        float fontSizeSub;
        UnitsOfMeasure::TYPE fontSizeSubUnit;
        float bondThickness;
        UnitsOfMeasure::TYPE bondThicknessUnit;
        float bondSpacing;
        float stereoBondWidth;
        UnitsOfMeasure::TYPE stereoBondWidthUnit;
        float hashSpacing;
        UnitsOfMeasure::TYPE hashSpacingUnit;

    private:
        RenderOptions(const RenderOptions&);
    };

    struct AcsOptions
    {
        AcsOptions();
        void clear();
        float bondSpacing;
        float fontSizeAngstrom;
        float fontSizeSubAngstrom;
        float bondThicknessAngstrom;
        float stereoBondWidthAngstrom;
        float hashSpacingAngstrom;
    };

} // namespace indigo

#define QUERY_MOL_BEGIN(mol)                                                                                                                                   \
    if (mol->isQueryMolecule())                                                                                                                                \
    {                                                                                                                                                          \
        QueryMolecule& qmol = mol->asQueryMolecule()
#define QUERY_MOL_END }

#define QUERY_RXN_BEGIN1(rxn)                                                                                                                                  \
    if (rxn->isQueryReaction())                                                                                                                                \
    {                                                                                                                                                          \
        QueryReaction& qr = rxn->asQueryReaction()

#define QUERY_RXN_BEGIN                                                                                                                                        \
    if (_r->isQueryReaction())                                                                                                                                 \
    {                                                                                                                                                          \
        QueryReaction& qr = _r->asQueryReaction()
#define QUERY_RXN_END }

#endif //__render_common_h__
