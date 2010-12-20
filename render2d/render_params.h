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

#ifndef __render_params_h__
#define __render_params_h__

#include "base_cpp/auto_ptr.h"
#include "render_common.h"
#include "graph/graph_highlighting.h"
#include "reaction/reaction_highlighting.h"

namespace indigo {

class BaseMolecule;
class Reaction;
class Scanner;
class Output;

enum RENDER_MODE {RENDER_MOL, RENDER_RXN, RENDER_NONE};

class RenderParams {
public:
   RenderParams ();
   ~RenderParams ();

   void clear ();
   void clearArrays ();

   float relativeThickness;
   RENDER_MODE rmode;
   DINGO_MODE mode;
   AutoPtr<BaseMolecule> mol;
   Output* output;
   GraphHighlighting molhl;
   AutoPtr<BaseReaction> rxn;
   ReactionHighlighting rhl;
   PtrArray<BaseMolecule> mols;
   ObjArray<GraphHighlighting> molhls;
   PtrArray<BaseReaction> rxns;
   ObjArray<ReactionHighlighting> rxnhls;
   ObjArray<Array <char> > titles;
   Array<char> titleProp;
   Array<int> refAtoms;

   PVOID hdc;
   RenderOptions rOpt;
   CanvasOptions cnvOpt;
};
     
class RenderParamInterface {
public:
   DEF_ERROR("render param interface");
   static void render (RenderParams& params);

private:
   static void _prepareMolecule (RenderParams& params, BaseMolecule& bm);
   static void _prepareReaction (RenderParams& params, BaseReaction& rxn);
   static bool needsLayoutSub (BaseMolecule& mol);
   static bool needsLayout (BaseMolecule& mol);
   RenderParamInterface ();
   RenderParamInterface (const RenderParamInterface&);
};

}

#endif //__render_params_h__
