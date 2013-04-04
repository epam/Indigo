/****************************************************************************
 * Copyright (C) 2010-2011 GGA Software Services LLC
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

#include "indigo_internal.h"
#include "indigo_io.h"
#include "indigo_array.h"
#include "indigo_renderer_internal.h"
#include "base_cpp/scanner.h"
#include "base_cpp/output.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "reaction/reaction.h"
#include "reaction/query_reaction.h"
#include "option_manager.h"
#include "indigo-renderer.h"
#include "indigo_molecule.h"
#include "indigo_reaction.h"

using namespace indigo;

_SessionLocalContainer<IndigoRenderer> indigo_renderer_self;

IndigoRenderer &indigoRendererGetInstance ()
{
   return indigo_renderer_self.getLocalCopy();
}

#define CHECKRGB(r, g, b) \
if (__min3(r, g, b) < 0 || __max3(r, g, b) > 1.0 + 1e-6) \
   throw IndigoError("Some of the color components are out of range [0..1]")

typedef RedBlackStringMap<int,false> StringIntMap;

IndigoRenderer::IndigoRenderer ()
{
}

IndigoRenderer::~IndigoRenderer ()
{
}

void indigoRenderSetCommentOffset (int offset)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.cnvOpt.commentOffset = offset;
}

void indigoRenderSetImageWidth (int v)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.cnvOpt.width = v;
}

void indigoRenderSetImageHeight (int v)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.cnvOpt.height = v;
}

void indigoRenderSetImageMaxWidth (int v)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.cnvOpt.maxWidth = v;
}

void indigoRenderSetImageMaxHeight (int v)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.cnvOpt.maxHeight = v;
}

void indigoRenderSetTitleOffset (int offset)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.cnvOpt.titleOffset = offset;
}                    

DINGO_MODE indigoRenderMapOutputFormat (const char *format)
{
   TL_DECL_GET(StringIntMap, outFmtMap);
   if (outFmtMap.size() == 0) {
      outFmtMap.insert("pdf", MODE_PDF);
      outFmtMap.insert("png", MODE_PNG);
      outFmtMap.insert("svg", MODE_SVG);
      outFmtMap.insert("emf", MODE_EMF);
   }
   return outFmtMap.find(format) ? (DINGO_MODE)outFmtMap.at(format) : MODE_NONE;
}

void indigoRenderSetOutputFormat (const char *format)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.rOpt.mode = indigoRenderMapOutputFormat(format);
}

void indigoRenderSetImageSize (int width, int height)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.cnvOpt.width = width;
   rp.cnvOpt.height = height;
}

void indigoRenderSetHDCOffset (int x, int y)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.cnvOpt.xOffset = x;
   rp.cnvOpt.yOffset = y;
}

void indigoRenderSetMargins (int x, int y)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.cnvOpt.marginX = x;
   rp.cnvOpt.marginY = y;
}

void indigoRenderSetGridMargins (int x, int y)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.cnvOpt.gridMarginX = x;
   rp.cnvOpt.gridMarginY = y;
}

void indigoRenderSetBondLength (float length)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.cnvOpt.bondLength = length;
}

void indigoRenderSetRelativeThickness (float rt)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   if (rt <= 0.0f)
      throw IndigoError("relative thickness must be positive");
   rp.relativeThickness = rt;
}

void indigoRenderSetBackgroundColor (float r, float g, float b)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.rOpt.backgroundColor.set((float)r, (float)g, (float)b);
}

void indigoRenderSetBaseColor (float r, float g, float b)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.rOpt.baseColor.set((float)r, (float)g, (float)b);
}

void indigoRenderSetImplicitHydrogenVisible (int enabled)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.rOpt.implHVisible = enabled != 0;
}

void indigoRenderSetHighlightedLabelsVisible (int enabled)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.rOpt.highlightedLabelsVisible = enabled != 0;
}

void indigoRenderSetColoring (int enabled)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.rOpt.atomColoring = enabled != 0;
}

void indigoRenderSetValencesVisible (int enabled)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.rOpt.showValences = enabled != 0;
}

void indigoRenderSetAtomIdsVisible (int enabled)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.rOpt.showAtomIds = enabled != 0;
}

void indigoRenderSetBondIdsVisible (int enabled)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.rOpt.showBondIds = enabled != 0;
}

void indigoRenderSetAtomBondIdsFromOne (int enabled)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.rOpt.atomBondIdsFromOne = enabled != 0;
}

void indigoRenderSetHighlightThicknessEnabled (int enabled)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.rOpt.highlightThicknessEnable = enabled != 0;
}

void indigoRenderSetHighlightColorEnabled (int enabled)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.rOpt.highlightColorEnable = enabled != 0;
}

void indigoRenderSetHighlightColor (float r, float g, float b)
{
   CHECKRGB(r, g, b);
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.rOpt.highlightColor.set(r, g, b);
}

void indigoRenderSetStereoStyle (const char* mode)
{
   TL_DECL_GET(StringIntMap, stereoStyleMap);
   if (stereoStyleMap.size() == 0) {
      stereoStyleMap.insert("ext", STEREO_STYLE_EXT);
      stereoStyleMap.insert("old", STEREO_STYLE_OLD);
      stereoStyleMap.insert("none", STEREO_STYLE_NONE);
   }
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.rOpt.stereoMode = (STEREO_STYLE)stereoStyleMap.at(mode);
}

void indigoRenderSetLabelMode (const char* mode)
{
   TL_DECL_GET(StringIntMap, labelMap);
   if (labelMap.size() == 0) {
      labelMap.insert("none", LABEL_MODE_NONE);
      labelMap.insert("hetero", LABEL_MODE_HETERO);
      labelMap.insert("terminal-hetero", LABEL_MODE_TERMINAL_HETERO);
      labelMap.insert("all", LABEL_MODE_ALL);
   }
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.rOpt.labelMode = (LABEL_MODE)labelMap.at(mode);
}

void indigoRenderSetCatalystsPlacement (const char* mode)
{
   TL_DECL_GET(StringIntMap, agentPlacementMap);
   if (agentPlacementMap.size() == 0) {
      agentPlacementMap.insert("above", 0);
      agentPlacementMap.insert("above-and-below", 1);
   }
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.rOpt.agentsBelowArrow = agentPlacementMap.at(mode) != 0;
}

void indigoRenderSetSuperatomMode (const char* mode)
{
   TL_DECL_GET(StringIntMap, stereoAtomMode);
   if (stereoAtomMode.size() == 0) {
      stereoAtomMode.insert("expand", 0);
      stereoAtomMode.insert("collapse", 1);
   }
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.rOpt.collapseSuperatoms = stereoAtomMode.at(mode) != 0;
}

void indigoRenderSetAAMColor (float r, float g, float b)
{
   CHECKRGB(r, g, b);
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.rOpt.aamColor.set(r, g, b);
}

void indigoRenderSetCommentColor (float r, float g, float b)
{
   CHECKRGB(r, g, b);
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.rOpt.commentColor.set(r, g, b);
}

void indigoRenderSetDataSGroupColor (float r, float g, float b)
{
   CHECKRGB(r, g, b);
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.rOpt.dataGroupColor.set(r, g, b);
}

void indigoRenderSetCenterDoubleBondWhenStereoAdjacent (int enabled)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.rOpt.centerDoubleBondWhenStereoAdjacent = enabled != 0;
}

void indigoRenderSetComment (const char* comment)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.cnvOpt.comment.clear();
   rp.cnvOpt.comment.appendString(comment, true);
}

void indigoRenderSetCommentFontSize (float fontSize)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.rOpt.commentFontFactor = fontSize;
}

void indigoRenderSetTitleFontSize (float fontSize)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.rOpt.titleFontFactor = fontSize;
}                                

void indigoRenderSetCommentAlignment (float align)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.cnvOpt.commentAlign = align;
}

void indigoRenderSetTitleAlignment (float align)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.cnvOpt.titleAlign = align;
}

void indigoRenderSetCommentPosition (const char* pos)
{
   TL_DECL_GET(StringIntMap, map);
   if (map.size() == 0) {
      map.insert("top", COMMENT_POS_TOP);
      map.insert("bottom", COMMENT_POS_BOTTOM);
   }
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.cnvOpt.commentPos = (COMMENT_POS)map.at(pos);
}

void indigoRenderSetGridTitleProperty (const char* prop)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.cnvOpt.titleProp.clear();
   rp.cnvOpt.titleProp.appendString(prop, true);
}                                      

CEXPORT int indigoRender (int object, int output)
{
   INDIGO_BEGIN
   {
      RenderParams& rp = indigoRendererGetInstance().renderParams;
      // If there are molecules/reactions in the arrays then current call will 
      // rendere a grid -> needs to clear it
      rp.clearArrays();

      IndigoObject &obj = self.getObject(object);

      if (IndigoBaseMolecule::is(obj))
      {
         if (obj.getBaseMolecule().isQueryMolecule())
            rp.mol.reset(new QueryMolecule());
         else
            rp.mol.reset(new Molecule());
         rp.mol->clone(self.getObject(object).getBaseMolecule(), 0, 0);
         rp.rmode = RENDER_MOL;
      }
      else if (IndigoBaseReaction::is(obj))
      {
         if (obj.getBaseReaction().isQueryReaction())
            rp.rxn.reset(new QueryReaction());
         else
            rp.rxn.reset(new Reaction());
         rp.rxn->clone(self.getObject(object).getBaseReaction(), 0, 0, 0);
         rp.rmode = RENDER_RXN;
      } else {
         throw IndigoError("The object provided should be a molecule, a reaction or an array of such");
      }

      IndigoObject& out = self.getObject(output);
      if (out.type == IndigoHDCOutput::HDC_OUTPUT) {
         IndigoHDCOutput& hdcOut = (IndigoHDCOutput&)self.getObject(output);
         rp.rOpt.hdc = hdcOut.dc;
         rp.rOpt.mode = hdcOut.prn ? MODE_PRN : MODE_HDC;
      } else if (out.type == IndigoObject::OUTPUT) {
         rp.rOpt.output = &IndigoOutput::get(out);
      } else {
         throw IndigoError("Invalid output object type");
      }
      RenderParamInterface::render(rp);
      return 1;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoRenderGrid (int objects, int* refAtoms, int nColumns, int output)
{
   INDIGO_BEGIN
   {
      RenderParams& rp = indigoRendererGetInstance().renderParams;
      rp.clearArrays();

      PtrArray<IndigoObject>& objs = IndigoArray::cast(self.getObject(objects)).objects;
      if (IndigoBaseMolecule::is(*objs[0]))
      {
         for (int i = 0; i < objs.size(); ++i) {
            if (objs[i]->getBaseMolecule().isQueryMolecule())
               rp.mols.add(new QueryMolecule());
            else
               rp.mols.add(new Molecule());
            Array<char>& title = rp.titles.push();
            if (objs[i]->getProperties()->find(rp.cnvOpt.titleProp.ptr()))
               title.copy(objs[i]->getProperties()->at(rp.cnvOpt.titleProp.ptr()));

            rp.mols.top()->clone(objs[i]->getBaseMolecule(), 0, 0, 0);
            rp.rmode = RENDER_MOL;
         }
      }
      else if (IndigoBaseReaction::is(*objs[0]))
      {
         for (int i = 0; i < objs.size(); ++i) {
            if (objs[i]->getBaseReaction().isQueryReaction())
               rp.rxns.add(new QueryReaction());
            else
               rp.rxns.add(new Reaction());
            Array<char>& title = rp.titles.push();
            if (objs[i]->getProperties()->find(rp.cnvOpt.titleProp.ptr()))
               title.copy(objs[i]->getProperties()->at(rp.cnvOpt.titleProp.ptr()));
            
            rp.rxns.top()->clone(objs[i]->getBaseReaction(), 0, 0, 0);
            rp.rmode = RENDER_RXN;
         }
      } else {
         throw IndigoError("The array elements should be molecules or reactions");
      }

      if (refAtoms != NULL) {                    
         rp.refAtoms.copy(refAtoms, objs.size());
      }

      rp.cnvOpt.gridColumnNumber = nColumns;

      bool hasNonemptyTitles = false;
      for (int i = 0; i < rp.titles.size(); ++i) {
         if (rp.titles[i].size() > 0) {
            hasNonemptyTitles = true;
            break;
         }
      }
      if (!hasNonemptyTitles)
         rp.titles.clear();

      IndigoObject& out = self.getObject(output);
      if (out.type == IndigoHDCOutput::HDC_OUTPUT) {
         IndigoHDCOutput& hdcOut = (IndigoHDCOutput&)self.getObject(output);
         rp.rOpt.hdc = hdcOut.dc;
         rp.rOpt.mode = hdcOut.prn ? MODE_PRN : MODE_HDC;
      } else if (out.type == IndigoObject::OUTPUT) {
         rp.rOpt.output = &IndigoOutput::get(out);
      } else {
         throw IndigoError("Invalid output object type");
      }
      RenderParamInterface::render(rp);

      // Release memory for arrays with molecules/reactions
      rp.clearArrays();

      return 1;
   }
   INDIGO_END(-1)
}

DINGO_MODE indigoRenderGuessOutputFormat(const char* filename)
{
   int len = strlen(filename);
   if (len < 4 || filename[len-4] != '.')
      return MODE_NONE;
   return indigoRenderMapOutputFormat(filename + len - 3);
}

CEXPORT int indigoRenderToFile (int object, const char *filename)
{
   int f = indigoWriteFile(filename);

   if (f == -1)
      return -1;

   RenderParams& rp = indigoRendererGetInstance().renderParams;
   DINGO_MODE setMode = rp.rOpt.mode;
   rp.rOpt.mode = (setMode == MODE_NONE) ? indigoRenderGuessOutputFormat(filename) : setMode;
   int res = indigoRender(object, f);
   rp.rOpt.mode = setMode;

   indigoFree(f);
   return res;
}

CEXPORT int indigoRenderGridToFile (int objects, int* refAtoms, int nColumns, const char *filename)
{
   int f = indigoWriteFile(filename);

   if (f == -1)
      return -1;

   RenderParams& rp = indigoRendererGetInstance().renderParams;
   DINGO_MODE setMode = rp.rOpt.mode;
   rp.rOpt.mode = (setMode == MODE_NONE) ? indigoRenderGuessOutputFormat(filename) : setMode;
   int res = indigoRenderGrid(objects, refAtoms, nColumns, f);
   rp.rOpt.mode = setMode;

   indigoFree(f);
   return res;
}

CEXPORT int indigoRenderReset ()
{
   INDIGO_BEGIN
   {
      RenderParams& rp = indigoRendererGetInstance().renderParams;
      rp.clear();
      return 1;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoRenderWriteHDC (void* hdc, int printingHdc)
{
   INDIGO_BEGIN
   {
      return self.addObject(new IndigoHDCOutput(hdc, printingHdc != 0));
   }
   INDIGO_END(-1)
}

class _IndigoRenderingOptionsHandlersSetter
{
public:
   _IndigoRenderingOptionsHandlersSetter ();
};

_IndigoRenderingOptionsHandlersSetter::_IndigoRenderingOptionsHandlersSetter ()
{
   OptionManager &mgr = indigoGetOptionManager();
   OsLocker locker(mgr.lock);

   mgr.setOptionHandlerInt("render-comment-offset", indigoRenderSetCommentOffset);
   mgr.setOptionHandlerInt("render-image-width", indigoRenderSetImageWidth);
   mgr.setOptionHandlerInt("render-image-height", indigoRenderSetImageHeight);
   mgr.setOptionHandlerInt("render-image-max-width", indigoRenderSetImageMaxWidth);
   mgr.setOptionHandlerInt("render-image-max-height", indigoRenderSetImageMaxHeight);

   mgr.setOptionHandlerString("render-output-format", indigoRenderSetOutputFormat);
   mgr.setOptionHandlerString("render-label-mode", indigoRenderSetLabelMode);
   mgr.setOptionHandlerString("render-comment", indigoRenderSetComment);
   mgr.setOptionHandlerString("render-comment-position", indigoRenderSetCommentPosition);
   mgr.setOptionHandlerString("render-stereo-style", indigoRenderSetStereoStyle);
   mgr.setOptionHandlerString("render-catalysts-placement", indigoRenderSetCatalystsPlacement);
   mgr.setOptionHandlerString("render-superatom-mode", indigoRenderSetSuperatomMode);

   mgr.setOptionHandlerBool("render-coloring", indigoRenderSetColoring);
   mgr.setOptionHandlerBool("render-valences-visible", indigoRenderSetValencesVisible);
   mgr.setOptionHandlerBool("render-atom-ids-visible", indigoRenderSetAtomIdsVisible);
   mgr.setOptionHandlerBool("render-bond-ids-visible", indigoRenderSetBondIdsVisible);
   mgr.setOptionHandlerBool("render-atom-bond-ids-from-one", indigoRenderSetAtomBondIdsFromOne);
   mgr.setOptionHandlerBool("render-highlight-thickness-enabled", indigoRenderSetHighlightThicknessEnabled);
   mgr.setOptionHandlerBool("render-highlight-color-enabled", indigoRenderSetHighlightColorEnabled);
   mgr.setOptionHandlerBool("render-center-double-bond-when-stereo-adjacent", indigoRenderSetCenterDoubleBondWhenStereoAdjacent);
   mgr.setOptionHandlerBool("render-implicit-hydrogens-visible", indigoRenderSetImplicitHydrogenVisible);
   mgr.setOptionHandlerBool("render-highlighted-labels-visible", indigoRenderSetHighlightedLabelsVisible);

   mgr.setOptionHandlerFloat("render-bond-length", indigoRenderSetBondLength);
   mgr.setOptionHandlerFloat("render-relative-thickness", indigoRenderSetRelativeThickness);
   mgr.setOptionHandlerFloat("render-comment-font-size", indigoRenderSetCommentFontSize);
   mgr.setOptionHandlerFloat("render-comment-alignment", indigoRenderSetCommentAlignment);

   mgr.setOptionHandlerColor("render-background-color", indigoRenderSetBackgroundColor);
   mgr.setOptionHandlerColor("render-base-color", indigoRenderSetBaseColor);
   mgr.setOptionHandlerColor("render-highlight-color", indigoRenderSetHighlightColor);
   mgr.setOptionHandlerColor("render-aam-color", indigoRenderSetAAMColor);
   mgr.setOptionHandlerColor("render-comment-color", indigoRenderSetCommentColor);
   mgr.setOptionHandlerColor("render-data-sgroup-color", indigoRenderSetDataSGroupColor);

   mgr.setOptionHandlerXY("render-image-size", indigoRenderSetImageSize);
   mgr.setOptionHandlerXY("render-hdc-offset", indigoRenderSetHDCOffset);
   mgr.setOptionHandlerXY("render-margins", indigoRenderSetMargins);

   mgr.setOptionHandlerXY("render-grid-margins", indigoRenderSetGridMargins);
   mgr.setOptionHandlerFloat("render-grid-title-alignment", indigoRenderSetTitleAlignment);
   mgr.setOptionHandlerFloat("render-grid-title-font-size", indigoRenderSetTitleFontSize);
   mgr.setOptionHandlerString("render-grid-title-property", indigoRenderSetGridTitleProperty);
   mgr.setOptionHandlerInt("render-grid-title-offset", indigoRenderSetTitleOffset);
}

_IndigoRenderingOptionsHandlersSetter _indigo_rendering_options_handlers_setter;
