/****************************************************************************
 * Copyright (C) 2010 GGA Software Services LLC
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
#include "graph/graph_highlighting.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "reaction/reaction.h"
#include "reaction/query_reaction.h"
#include "option_manager.h"
#include "indigo-renderer.h"

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

void indigoRenderSetTitleOffset (int offset)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.cnvOpt.titleOffset = offset;
}                    

void indigoRenderSetOutputFormat (const char *format)
{
   TL_DECL_GET(StringIntMap, outFmtMap);
   if (outFmtMap.size() == 0) {
      outFmtMap.insert("pdf", MODE_PDF);
      outFmtMap.insert("png", MODE_PNG);
      outFmtMap.insert("svg", MODE_SVG);
      outFmtMap.insert("emf", MODE_EMF);
   }
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.mode = (DINGO_MODE)outFmtMap.at(format);
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

void indigoRenderSetImplicitHydrogenMode (const char* mode)
{
   TL_DECL_GET(StringIntMap, implHydroMap);
   if (implHydroMap.size() == 0) {
      implHydroMap.insert("none", IHM_NONE);
      implHydroMap.insert("terminal", IHM_TERMINAL);
      implHydroMap.insert("hetero", IHM_HETERO);
      implHydroMap.insert("terminalhetero", IHM_TERMINAL_HETERO);
      implHydroMap.insert("all", IHM_ALL);
   }
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.rOpt.implHMode = (IMPLICIT_HYDROGEN_MODE)implHydroMap.at(mode);
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

void indigoRenderSetStereoOldStyle (int enabled)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.rOpt.useOldStereoNotation = enabled != 0;
}

void indigoRenderSetLabelMode (const char* mode)
{
   TL_DECL_GET(StringIntMap, labelMap);
   if (labelMap.size() == 0) {
      labelMap.insert("normal", LABEL_MODE_NORMAL);
      labelMap.insert("forceshow", LABEL_MODE_FORCESHOW);
      labelMap.insert("hideterminal", LABEL_MODE_HIDETERMINAL);
      labelMap.insert("forcehide", LABEL_MODE_FORCEHIDE);
   }
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.rOpt.labelMode = (LABEL_MODE)labelMap.at(mode);
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

void indigoRenderSetCenterDoubleBondWhenStereoAdjacent (int enabled)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.rOpt.centerDoubleBondWhenStereoAdjacent = enabled != 0;
}

void indigoRenderSetComment (const char* comment)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.rOpt.comment.clear();
   rp.rOpt.comment.appendString(comment, true);
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
   rp.rOpt.commentAlign = align;
}

void indigoRenderSetTitleAlignment (float align)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.rOpt.titleAlign = align;
}

void indigoRenderSetCommentPosition (const char* pos)
{
   TL_DECL_GET(StringIntMap, map);
   if (map.size() == 0) {
      map.insert("top", COMMENT_POS_TOP);
      map.insert("bottom", COMMENT_POS_BOTTOM);
   }
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.rOpt.commentPos = (COMMENT_POS)map.at(pos);
}

void indigoRenderSetGridTitleProperty (const char* prop)
{
   RenderParams& rp = indigoRendererGetInstance().renderParams;
   rp.titleProp.clear();
   rp.titleProp.appendString(prop, true);
}                                      

CEXPORT int indigoRender (int object, int output)
{
   INDIGO_BEGIN
   {
      RenderParams& rp = indigoRendererGetInstance().renderParams;
      IndigoObject &obj = self.getObject(object);

      if (obj.isBaseMolecule())
      {
         Array<int> mapping;
         if (obj.getBaseMolecule().isQueryMolecule())
            rp.mol.reset(new QueryMolecule());
         else
            rp.mol.reset(new Molecule());
         rp.mol->clone(self.getObject(object).getBaseMolecule(), &mapping, 0);
         GraphHighlighting* hl = self.getObject(object).getMoleculeHighlighting();
         if (hl != 0 && hl->numVertices() > 0) {
            rp.molhl.init(*rp.mol.get());
            rp.molhl.copy(*hl, &mapping);
         }
         rp.rmode = RENDER_MOL;
      }
      else if (obj.isBaseReaction())
      {
         if (obj.getBaseReaction().isQueryReaction())
            rp.rxn.reset(new QueryReaction());
         else
            rp.rxn.reset(new Reaction());
         ObjArray< Array<int> > mapping;
         rp.rxn->clone(self.getObject(object).getBaseReaction(), &mapping, 0);
         ReactionHighlighting *hl = self.getObject(object).getReactionHighlighting();
         if (hl != 0 && hl->getCount() > 0) {
            rp.rhl.init(*rp.rxn.get());
            rp.rhl.copy(*hl, mapping);
         }
         rp.rmode = RENDER_RXN;
      } else {
         throw IndigoError("The object provided should be a molecule, a reaction or an array of such");
      }

      IndigoObject& out = self.getObject(output);
      if (out.type == IndigoHDCOutput::HDC_OUTPUT) {
         IndigoHDCOutput& hdcOut = (IndigoHDCOutput&)self.getObject(output);
         rp.hdc = hdcOut.dc;
         rp.mode = hdcOut.prn ? MODE_PRN : MODE_HDC;
      } else if (out.type == IndigoObject::OUTPUT) {
         rp.output = &IndigoOutput::get(out);
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
      if (objs[0]->isBaseMolecule())
      {
         for (int i = 0; i < objs.size(); ++i) {
            if (objs[i]->getBaseMolecule().isQueryMolecule())
               rp.mols.add(new QueryMolecule());
            else
               rp.mols.add(new Molecule());
            GraphHighlighting& molhl = rp.molhls.push();
            Array<char>& title = rp.titles.push();
            if (objs[i]->getProperties()->find(rp.titleProp.ptr()))
               title.copy(objs[i]->getProperties()->at(rp.titleProp.ptr()));

            QS_DEF(Array<int>, mapping);
            rp.mols.top()->clone(objs[i]->getBaseMolecule(), &mapping, 0);
            GraphHighlighting* hl = objs[i]->getMoleculeHighlighting();
            if (hl != 0 && hl->numVertices() > 0) {
               molhl.init(*rp.mols.top());
               molhl.copy(*hl, &mapping);
            }
            rp.rmode = RENDER_MOL;
         }
      }
      else if (objs[0]->isBaseReaction())
      {
         for (int i = 0; i < objs.size(); ++i) {
            if (objs[i]->getBaseReaction().isQueryReaction())
               rp.rxns.add(new QueryReaction());
            else
               rp.rxns.add(new Reaction());
            ReactionHighlighting& rxnhl = rp.rxnhls.push();
            Array<char>& title = rp.titles.push();
            if (objs[i]->getProperties()->find(rp.titleProp.ptr()))
               title.copy(objs[i]->getProperties()->at(rp.titleProp.ptr()));
            
            QS_DEF(ObjArray< Array<int> >, mapping);
            rp.rxns.top()->clone(objs[i]->getBaseReaction(), &mapping, 0);
            ReactionHighlighting *hl = objs[i]->getReactionHighlighting();
            if (hl != 0) {
               rxnhl.init(*rp.rxns.top());
               rxnhl.copy(*hl, mapping);
            }
            rp.rmode = RENDER_RXN;
         }
      } else {
         throw IndigoError("The array elements should be molecules or reactions");
      }

      if (refAtoms != NULL) {                    
         rp.refAtoms.copy(refAtoms, objs.size());
      }

      rp.rOpt.gridColumnNumber = nColumns;

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
         rp.hdc = hdcOut.dc;
         rp.mode = hdcOut.prn ? MODE_PRN : MODE_HDC;
      } else if (out.type == IndigoObject::OUTPUT) {
         rp.output = &IndigoOutput::get(out);
      } else {
         throw IndigoError("Invalid output object type");
      }
      RenderParamInterface::render(rp);
      return 1;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoRenderToFile (int object, const char *filename)
{
   int f = indigoWriteFile(filename);

   if (f == -1)
      return -1;

   int res = indigoRender(object, f);

   indigoFree(f);
   return res;
}

CEXPORT int indigoRenderGridToFile (int objects, int* refAtoms, int nColumns, const char *filename)
{
   int f = indigoWriteFile(filename);

   if (f == -1)
      return -1;

   int res = indigoRenderGrid(objects, refAtoms, nColumns, f);

   indigoFree(f);
   return res;
}

CEXPORT int indigoRenderReset (int render)
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

   mgr.setOptionHandlerString("render-output-format", indigoRenderSetOutputFormat);
   mgr.setOptionHandlerString("render-implicit-hydrogen-mode", indigoRenderSetImplicitHydrogenMode);
   mgr.setOptionHandlerString("render-label-mode", indigoRenderSetLabelMode);
   mgr.setOptionHandlerString("render-comment", indigoRenderSetComment);
   mgr.setOptionHandlerString("render-comment-position", indigoRenderSetCommentPosition);

   mgr.setOptionHandlerBool("render-coloring", indigoRenderSetColoring);
   mgr.setOptionHandlerBool("render-valences-visible", indigoRenderSetValencesVisible);
   mgr.setOptionHandlerBool("render-atom-ids-visible", indigoRenderSetAtomIdsVisible);
   mgr.setOptionHandlerBool("render-bond-ids-visible", indigoRenderSetBondIdsVisible);
   mgr.setOptionHandlerBool("render-highlight-thickness-enabled", indigoRenderSetHighlightThicknessEnabled);
   mgr.setOptionHandlerBool("render-highlight-color-enabled", indigoRenderSetHighlightColorEnabled);
   mgr.setOptionHandlerBool("render-stereo-old-style", indigoRenderSetStereoOldStyle);
   mgr.setOptionHandlerBool("render-center-double-bond-when-stereo-adjacent", indigoRenderSetCenterDoubleBondWhenStereoAdjacent);

   mgr.setOptionHandlerFloat("render-bond-length", indigoRenderSetBondLength);
   mgr.setOptionHandlerFloat("render-relative-thickness", indigoRenderSetRelativeThickness);
   mgr.setOptionHandlerFloat("render-comment-font-size", indigoRenderSetCommentFontSize);
   mgr.setOptionHandlerFloat("render-comment-alignment", indigoRenderSetCommentAlignment);

   mgr.setOptionHandlerColor("render-background-color", indigoRenderSetBackgroundColor);
   mgr.setOptionHandlerColor("render-base-color", indigoRenderSetBaseColor);
   mgr.setOptionHandlerColor("render-highlight-color", indigoRenderSetHighlightColor);
   mgr.setOptionHandlerColor("render-aam-color", indigoRenderSetAAMColor);
   mgr.setOptionHandlerColor("render-comment-color", indigoRenderSetCommentColor);

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
