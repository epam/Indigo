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
#include "indigo_renderer_internal.h"
#include "base_cpp/scanner.h"
#include "base_cpp/output.h"
#include "graph/graph_highlighting.h"
#include "option_manager.h"

using namespace indigo;

TL_DEF_EXT(IndigoRenderer, indigo_renderer_self);

#define CHECKRGB(r, g, b) \
if (__min3(r, g, b) < 0 || __max3(r, g, b) > 1.0 + 1e-6) \
   throw IndigoError("Some of the color components are out of range [0..1]")

#define INDIGO_RENDER_BEGIN { \
      Indigo &self = indigoGetInstance(); \
      TL_GET2(IndigoRenderer, renderer_self, indigo_renderer_self); \
      try { self.error_message.clear(); \
            RenderParams& rp = renderer_self.renderParams

#define INDIGO_RENDER_BEGIN_2 { \
      Indigo &self = indigoGetInstance(); \
      TL_GET2(IndigoRenderer, renderer_self, indigo_renderer_self); \
      try { self.error_message.clear();


#define INDIGO_RENDER_END INDIGO_END(1, -1)

typedef RedBlackStringMap<int,false> StringIntMap;

IndigoRenderer::IndigoRenderer ()
{
}

IndigoRenderer::~IndigoRenderer ()
{
}

int indigoRenderSetOutputFormat (const char *format)
{
   TL_DECL_GET(StringIntMap, outFmtMap);
   if (outFmtMap.size() == 0) {
      outFmtMap.insert("pdf", MODE_PDF);
      outFmtMap.insert("png", MODE_PNG);
      outFmtMap.insert("svg", MODE_SVG);
      outFmtMap.insert("emf", MODE_EMF);
   }

   INDIGO_RENDER_BEGIN;
   rp.mode = (DINGO_MODE)outFmtMap.at(format);
   INDIGO_RENDER_END;
}

int indigoRenderSetImageSize (int width, int height)
{
   INDIGO_RENDER_BEGIN;
   rp.cnvOpt.width = width;
   rp.cnvOpt.height = height;
   INDIGO_RENDER_END;

}

int indigoRenderSetHDCOffset (int x, int y)
{
   INDIGO_RENDER_BEGIN;
   rp.cnvOpt.xOffset = x;
   rp.cnvOpt.yOffset = y;
   INDIGO_RENDER_END;
}

int indigoRenderSetMargins (int x, int y)
{
   INDIGO_RENDER_BEGIN;
   rp.cnvOpt.marginX = x;
   rp.cnvOpt.marginY = y;
   INDIGO_RENDER_END;
}

int indigoRenderSetCommentMargins (int x, int y)
{
   INDIGO_RENDER_BEGIN;
   rp.cnvOpt.commentMarginX = x;
   rp.cnvOpt.commentMarginY = y;
   INDIGO_RENDER_END;
}

int indigoRenderSetBondLength (float length)
{
   INDIGO_RENDER_BEGIN;
   rp.cnvOpt.bondLength = length;
   INDIGO_RENDER_END;
}

int indigoRenderSetRelativeThickness (float rt)
{
   INDIGO_RENDER_BEGIN;
   if (rt <= 0.0f)
      throw IndigoError("relative thickness must be positive");
   rp.relativeThickness = rt;
   INDIGO_RENDER_END;
}

int indigoRenderSetBackgroundColor (float r, float g, float b)
{
   INDIGO_RENDER_BEGIN;
   rp.backgroundColor.set((float)r, (float)g, (float)b);
   INDIGO_RENDER_END;
}

int indigoRenderSetBaseColor (float r, float g, float b)
{
   INDIGO_RENDER_BEGIN;
   rp.baseColor.set((float)r, (float)g, (float)b);
   INDIGO_RENDER_END;
}

int indigoRenderSetImplicitHydrogenMode (const char* mode)
{
   TL_DECL_GET(StringIntMap, implHydroMap);
   if (implHydroMap.size() == 0) {
      implHydroMap.insert("none", IHM_NONE);
      implHydroMap.insert("terminal", IHM_TERMINAL);
      implHydroMap.insert("hetero", IHM_HETERO);
      implHydroMap.insert("terminalhetero", IHM_TERMINAL_HETERO);
      implHydroMap.insert("all", IHM_ALL);
   }
   INDIGO_RENDER_BEGIN;
   rp.rOpt.implHMode = (IMPLICIT_HYDROGEN_MODE)implHydroMap.at(mode);
   INDIGO_RENDER_END;
}

int indigoRenderSetColoring (int enabled)
{
   INDIGO_RENDER_BEGIN;
   rp.rOpt.atomColoring = enabled != 0;
   INDIGO_RENDER_END;
}

int indigoRenderSetValencesVisible (int enabled)
{
   INDIGO_RENDER_BEGIN;
   rp.rOpt.showValences = enabled != 0;
   INDIGO_RENDER_END;
}

int indigoRenderSetAtomIdsVisible (int enabled)
{
   INDIGO_RENDER_BEGIN;
   rp.rOpt.showAtomIds = enabled != 0;
   INDIGO_RENDER_END;
}

int indigoRenderSetBondIdsVisible (int enabled)
{
   INDIGO_RENDER_BEGIN;
   rp.rOpt.showBondIds = enabled != 0;
   INDIGO_RENDER_END;
}

int indigoRenderSetHighlightThicknessEnabled (int enabled)
{
   INDIGO_RENDER_BEGIN;
   rp.hlOpt.highlightThicknessEnable = enabled != 0;
   INDIGO_RENDER_END;
}

int indigoRenderSetHighlightColorEnabled (int enabled)
{
   INDIGO_RENDER_BEGIN;
   rp.hlOpt.highlightColorEnable = enabled != 0;
   INDIGO_RENDER_END;
}

int indigoRenderSetHighlightColor (float r, float g, float b)
{
   INDIGO_RENDER_BEGIN;
   CHECKRGB(r, g, b);
   rp.hlOpt.highlightColor.set(r, g, b);
   INDIGO_RENDER_END;
}

int indigoRenderSetStereoOldStyle (int enabled)
{
   INDIGO_RENDER_BEGIN;
   rp.rOpt.useOldStereoNotation = enabled != 0;
   INDIGO_RENDER_END;
}

int indigoRenderSetLabelMode (const char* mode)
{
   TL_DECL_GET(StringIntMap, labelMap);
   if (labelMap.size() == 0) {
      labelMap.insert("normal", LABEL_MODE_NORMAL);
      labelMap.insert("forceshow", LABEL_MODE_FORCESHOW);
      labelMap.insert("hideterminal", LABEL_MODE_HIDETERMINAL);
      labelMap.insert("forcehide", LABEL_MODE_FORCEHIDE);
   }
   INDIGO_RENDER_BEGIN;
   rp.rOpt.labelMode = (LABEL_MODE)labelMap.at(mode);
   INDIGO_RENDER_END;
}

int indigoRenderSetAAMColor (float r, float g, float b)
{
   INDIGO_RENDER_BEGIN;
   CHECKRGB(r, g, b);
   rp.rcOpt.aamColor.set(r, g, b);
   INDIGO_RENDER_END;
}

int indigoRenderSetCommentColor (float r, float g, float b)
{
   INDIGO_RENDER_BEGIN;
   CHECKRGB(r, g, b);
   rp.rOpt.commentColor.set(r, g, b);
   INDIGO_RENDER_END;
}

int indigoRenderSetCenterDoubleBondWhenStereoAdjacent (int enabled)
{
   INDIGO_RENDER_BEGIN;
   rp.rOpt.centerDoubleBondWhenStereoAdjacent = enabled != 0;
   INDIGO_RENDER_END;
}

int indigoRenderSetComment (const char* comment)
{
   INDIGO_RENDER_BEGIN;
   rp.rOpt.comment.clear();
   rp.rOpt.comment.appendString(comment, true);
   INDIGO_RENDER_END;
}

int indigoRenderSetCommentFontSize (float fontSize)
{
   INDIGO_RENDER_BEGIN;
   rp.rcOpt.commentFontFactor = fontSize;
   INDIGO_RENDER_END;
}

int indigoRenderSetCommentPosition (const char* pos)
{
   TL_DECL_GET(StringIntMap, map);
   if (map.size() == 0) {
      map.insert("top", COMMENT_POS_TOP);
      map.insert("bottom", COMMENT_POS_BOTTOM);
   }
   INDIGO_RENDER_BEGIN;
   rp.rOpt.commentPos = (COMMENT_POS)map.at(pos);
   INDIGO_RENDER_END;
}

int indigoRenderSetCommentAlignment (const char* align)
{
   TL_DECL_GET(StringIntMap, map);
   if (map.size() == 0) {
      map.insert("left", ALIGNMENT_LEFT);
      map.insert("center", ALIGNMENT_CENTER);
      map.insert("right", ALIGNMENT_RIGHT);
   }
   INDIGO_RENDER_BEGIN;
   rp.rOpt.commentAlign = (ALIGNMENT)map.at(align);
   INDIGO_RENDER_END;
}

CEXPORT int indigoRender (int object, int output)
{
   INDIGO_RENDER_BEGIN;
   {
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
            rp.molhl.copy(*hl, mapping);
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
      }
      else
         throw IndigoError("The object provided is neither a molecule, nor a reaction");

      IndigoObject& out = self.getObject(output);
      if (out.type == IndigoHDCOutput::HDC_OUTPUT) {
         IndigoHDCOutput& hdcOut = (IndigoHDCOutput&)self.getObject(output);
         rp.hdc = hdcOut.dc;
         rp.mode = hdcOut.prn ? MODE_PRN : MODE_HDC;
      } else if (out.type == IndigoObject::OUTPUT) {
         rp.output = &out.getOutput();
      } else {
         throw IndigoError("Invalid output object type");
      }
      RenderParamInterface::render(rp);
   }
   INDIGO_END(1, -1)
}

CEXPORT int indigoRenderToFile (int object, const char *filename)
{
   int f = indigoWriteFile(filename);
   int res;

   if (f == -1)
      return -1;

   res = indigoRender(object, f);

   indigoFree(f);
   return res;
}

CEXPORT int indigoRenderReset (int render)
{
   INDIGO_RENDER_BEGIN;
   rp.clear();
   INDIGO_RENDER_END;
}

CEXPORT int indigoRenderWriteHDC (void* hdc, int printingHdc)
{
   INDIGO_RENDER_BEGIN_2;
   {
      return self.addObject(new IndigoHDCOutput(hdc, printingHdc != 0));
   }
   INDIGO_RENDER_END;
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

   mgr.setOptionHandlerString("render-output-format", indigoRenderSetOutputFormat);
   mgr.setOptionHandlerString("render-implicit-hydrogen-mode", indigoRenderSetImplicitHydrogenMode);
   mgr.setOptionHandlerString("render-label-mode", indigoRenderSetLabelMode);
   mgr.setOptionHandlerString("render-comment", indigoRenderSetComment);
   mgr.setOptionHandlerString("render-comment-position", indigoRenderSetCommentPosition);
   mgr.setOptionHandlerString("render-comment-alignment", indigoRenderSetCommentAlignment);

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

   mgr.setOptionHandlerColor("render-background-color", indigoRenderSetBackgroundColor);
   mgr.setOptionHandlerColor("render-base-color", indigoRenderSetBaseColor);
   mgr.setOptionHandlerColor("render-highlight-color", indigoRenderSetHighlightColor);
   mgr.setOptionHandlerColor("render-aam-color", indigoRenderSetAAMColor);
   mgr.setOptionHandlerColor("render-comment-color", indigoRenderSetCommentColor);

   mgr.setOptionHandlerXY("render-image-size", indigoRenderSetImageSize);
   mgr.setOptionHandlerXY("render-hdc-offset", indigoRenderSetHDCOffset);
   mgr.setOptionHandlerXY("render-margins", indigoRenderSetMargins);
   mgr.setOptionHandlerXY("render-comment-margins", indigoRenderSetCommentMargins);
}

_IndigoRenderingOptionsHandlersSetter _indigo_rendering_options_handlers_setter;
