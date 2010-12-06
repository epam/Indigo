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

#ifndef __render_common_h__
#define __render_common_h__

#include "molecule/elements.h"
#include "base_cpp/tlscont.h"
#include "layout/metalayout.h"

typedef void* PVOID;

namespace indigo {

struct Vec2f;
struct Edge;
class BaseMolecule;
class BaseReaction;
class BaseMolecule;
class Reaction;
class QueryMolecule;
class QueryReaction;
class GraphHighlighting;

enum DINGO_MODE {MODE_NONE, MODE_PDF, MODE_PNG, MODE_SVG, MODE_EMF, MODE_HDC, MODE_PRN};
enum INPUT_FORMAT {INPUT_FORMAT_UNKNOWN, INPUT_FORMAT_MOLFILE, INPUT_FORMAT_RXNFILE, INPUT_FORMAT_SMILES, INPUT_FORMAT_REACTION_SMILES};
enum LABEL_MODE {LABEL_MODE_NORMAL, LABEL_MODE_FORCESHOW, LABEL_MODE_HIDETERMINAL, LABEL_MODE_FORCEHIDE};
enum IMPLICIT_HYDROGEN_MODE {IHM_NONE, IHM_TERMINAL, IHM_HETERO, IHM_TERMINAL_HETERO, IHM_ALL};
enum {CWC_BASE = -2, CWC_WHITE=0, CWC_BLACK, CWC_RED, CWC_GREEN, CWC_BLUE, CWC_DARKGREEN, CWC_COUNT};
enum FONT_SIZE {FONT_SIZE_LABEL=0, FONT_SIZE_ATTR, FONT_SIZE_RGROUP_LOGIC, FONT_SIZE_RGROUP_LOGIC_INDEX, FONT_SIZE_INDICES, FONT_SIZE_COMMENT, FONT_SIZE_COUNT/*must be the last*/};
enum COMMENT_POS {COMMENT_POS_TOP, COMMENT_POS_BOTTOM};

// cos(a) to cos(a/2) 
double cos2c (const double cs);
// cos(a) to sin(a/2) 
double sin2c (const double cs);
// cos(a) to tg(a/2) 
double tg2c (const double cs);
// cos(a) to ctg(a/2) 
double ctg2c (const double cs); 

struct RenderItem {
   enum TYPE {
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
      RIT_TOPOLOGY
   };

   RenderItem();
   void clear();

   TYPE ritype;
   Vec2f bbp;
   Vec2f bbsz;
   Vec2f relpos;
   int color;
   bool highlighted;
};

struct TextItem : public RenderItem {
   void clear();
   Array<char> text;
   FONT_SIZE fontsize;
};

struct GraphItem : public RenderItem {
   enum TYPE {DOT, CAP, PLUS, MINUS};
   void clear();
   TYPE type;
};

struct AtomDesc {
   enum TYPE {TYPE_REGULAR, TYPE_PSEUDO, TYPE_QUERY};
   AtomDesc();
   void clear ();

   int tibegin, ticount;
   int gibegin, gicount;

   int type;
   bool showLabel;
   bool showHydro;
   bool shiftLeft;
   bool isRGroupAttachmentPoint;
   bool fixed;
   bool pseudoAtomStringVerbose;
   Vec2f pos;
   Vec2f boundBoxMin;
   Vec2f boundBoxMax;
   int label;
   int queryLabel;
   int color;
   int stereoGroupType;
   int stereoGroupNumber;
   Array<int> list;
   Array<char> pseudo;

   float leftMargin, rightMargin, ypos, height;
private:
   AtomDesc(const AtomDesc& ad);
};

struct BondEnd {
   BondEnd ();
   void clear ();

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
   BondEnd (const BondEnd& be);
};

struct BondDescr : public Edge {  
   BondDescr ();

   DEF_ERROR("molrender bond description");

   void clear ();

   int getBondEnd (int aid) const;
         
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
   int type;
   int queryType;
   float length;
   int be1, be2;
   float extP, extN;
   int tiTopology;
   int topology;
private:
   BondDescr (const BondDescr& bd);
};

struct Ring {
   Ring ();
   void clear ();

   Array<int> bondEnds;
   Array<float> angles;
   int dblBondCount;
   bool aromatic;
   Vec2f center;
   float radius;

private:
   Ring (const Ring& r);
};

struct MoleculeRenderData {
   MoleculeRenderData ();
   void clear ();

   ObjArray<AtomDesc> atoms;
   Array<BondDescr> bonds;
   ObjArray<Ring> rings;
   Array<BondEnd> bondends;
   ObjArray<TextItem> textitems;
   Array<GraphItem> graphitems;
   Array<int> aam;
   Array<int> reactingCenters;
   Array<int> inversions;
   Array<int> exactChanges;
private:
   MoleculeRenderData (const MoleculeRenderData& data);
};

class RenderSettings {
public:
   RenderSettings ();   
   void init (float sf);

   TL_CP_DECL(Array<double>, bondDashAromatic);
   TL_CP_DECL(Array<double>, bondDashAny);
   TL_CP_DECL(Array<double>, bondDashSingleOrAromatic);
   TL_CP_DECL(Array<double>, bondDashDoubleOrAromatic);

   float labelInternalOffset;
   float lowerIndexShift;
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

private:
   RenderSettings (const RenderSettings& settings);
};

struct CanvasOptions {
   CanvasOptions ();
   void clear ();
   
   int width;
   int height;
   int xOffset;
   int yOffset;
   float bondLength;
   int gridMarginX;
   int gridMarginY;
   int marginX;
   int marginY;
   float commentOffset;
};

struct HighlightingOptions {
   HighlightingOptions ();
   void clear();

   bool highlightThicknessEnable;
   float highlightThicknessFactor;
   bool highlightColorEnable;
   Vec3f highlightColor;
};

struct RenderContextOptions {
   RenderContextOptions ();
   void clear();

   Vec3f aamColor;
   float commentFontFactor;
};

class RenderOptions {
public:
   RenderOptions ();
   void clear();
   void copy(const RenderOptions& other);

   Array<char> comment;
   COMMENT_POS commentPos;
   ALIGNMENT commentAlign;
   Vec3f commentColor;
   LABEL_MODE labelMode;
   IMPLICIT_HYDROGEN_MODE implHMode;
   bool showBondIds;
   bool showBondEndIds;
   bool showNeighborArcs;
   bool showAtomIds;
   bool showValences;
   bool atomColoring;
   bool useOldStereoNotation;
   bool showReactingCenterUnchanged;
   bool centerDoubleBondWhenStereoAdjacent;
   bool showCycles; // for diagnostic purposes
private:
   RenderOptions (const RenderOptions& );
};

}

#define QUERY_MOL_BEGIN(mol) if (mol->isQueryMolecule()) { QueryMolecule& qmol = mol->asQueryMolecule()
#define QUERY_MOL_END }

#define QUERY_RXN_BEGIN1(rxn) if (rxn->isQueryReaction()) { QueryReaction& qr = rxn->asQueryReaction()

#define QUERY_RXN_BEGIN if (_r->isQueryReaction()) { QueryReaction& qr = _r->asQueryReaction()
#define QUERY_RXN_END }

#endif //__render_common_h__