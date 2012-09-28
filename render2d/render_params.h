/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
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

   AutoPtr<BaseMolecule> mol;
   AutoPtr<BaseReaction> rxn;
   
   PtrArray<BaseMolecule> mols;
   PtrArray<BaseReaction> rxns;
   
   ObjArray<Array <char> > titles;
   Array<int> refAtoms;

   RenderOptions rOpt;
   CanvasOptions cnvOpt;
};
     
class RenderParamInterface {
public:
   DECL_ERROR;
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
