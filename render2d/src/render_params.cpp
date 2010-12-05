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

#include "base_cpp/array.h"
#include "base_cpp/output.h"
#include "graph/graph_highlighting.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "reaction/reaction.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction_highlighting.h"
#include "layout/metalayout.h"
#include "layout/reaction_layout.h"
#include "layout/molecule_layout.h"
#include "molecule/molecule_arom.h"
#include "molecule/molecule_dearom.h"
#include "molecule/molecule_auto_loader.h"
#include "molecule/molfile_loader.h"
#include "molecule/smiles_loader.h"
#include "molecule/molfile_saver.h"
#include "reaction/reaction_auto_loader.h"
#include "reaction/rxnfile_loader.h"
#include "reaction/rsmiles_loader.h"
#include "reaction/rxnfile_saver.h"

#include "render_context.h"
#include "render_params.h"
#include "render_item_molecule.h"
#include "render_item_factory.h"
#include "render_single.h"

using namespace indigo;

RenderParams::RenderParams ()
{
   clear();
}

RenderParams::~RenderParams ()
{
}

void RenderParams::clear ()
{
   query = false;
   relativeThickness = 1.0f;
   loadHighlighting = true;
   backgroundColor.set(-1, -1, -1);
   aromatization = 0;
   baseColor.set(0, 0, 0);
   rmode = RENDER_NONE;
   mode = MODE_NONE;
   inputFormat = INPUT_FORMAT_UNKNOWN;
   mol.reset(NULL);
   molhl.clear();
   rxn.reset(NULL);
   rhl.clear();
   hdc = 0;
   outfile.clear();
   rOpt.clear();
   cnvOpt.clear();
   hlOpt.clear();
   rcOpt.clear();
}

void RenderParamInterface::loadMol (RenderParams& params, Scanner& scanner)
{
   params.mol.reset(params.query ? (BaseMolecule*)new QueryMolecule() : (BaseMolecule*)new Molecule());
   if (params.inputFormat == INPUT_FORMAT_MOLFILE)
   {
      MolfileLoader loader(scanner);

      if (params.loadHighlighting)
         loader.highlighting = &params.molhl;
      loader.ignore_stereocenter_errors = true;
      if (!params.query)
         loader.loadMolecule(params.mol.ref().asMolecule());
      else
         loader.loadQueryMolecule(params.mol.ref().asQueryMolecule());
   }
   else if (params.inputFormat == INPUT_FORMAT_SMILES)
   {
      SmilesLoader loader(scanner);

      if (params.loadHighlighting)
         loader.highlighting = &params.molhl;
      if (!params.query)
         loader.loadMolecule(params.mol.ref().asMolecule());
      else
         loader.loadQueryMolecule(params.mol.ref().asQueryMolecule());
   }
   else // INPUT_FORMAT_UNKNOWN
   {
      MoleculeAutoLoader loader(scanner);

      if (params.loadHighlighting)
         loader.highlighting = &params.molhl;
      loader.ignore_stereocenter_errors = true;
      if (!params.query)
         loader.loadMolecule(params.mol.ref().asMolecule());
      else
         loader.loadQueryMolecule(params.mol.ref().asQueryMolecule());
   }
   params.rmode = RENDER_MOL;
}

void RenderParamInterface::loadRxn (RenderParams& params, Scanner& scanner)
{
   params.rxn.reset(params.query ? (BaseReaction*)new QueryReaction() : (BaseReaction*)new Reaction());
   if (params.inputFormat == INPUT_FORMAT_RXNFILE)
   {
      RxnfileLoader loader(scanner);

      if (params.loadHighlighting)
         loader.highlighting = &params.rhl;

      loader.ignore_stereocenter_errors = true;
      if (!params.query)
         loader.loadReaction(params.rxn.ref().asReaction());
      else
         loader.loadQueryReaction(params.rxn.ref().asQueryReaction());
   }
   else if (params.inputFormat == INPUT_FORMAT_REACTION_SMILES)
   {
      RSmilesLoader loader(scanner);

      if (params.loadHighlighting)
         loader.highlighting = &params.rhl;

      if (!params.query)
         loader.loadReaction(params.rxn.ref().asReaction());
      else
         loader.loadQueryReaction(params.rxn.ref().asQueryReaction());
   }
   else // INPUT_FORMAT_UNKNOWN
   {
      ReactionAutoLoader loader(scanner);
      if (params.loadHighlighting)
         loader.highlighting = &params.rhl;
      loader.ignore_stereocenter_errors = true;
      if (!params.query)
         loader.loadReaction(params.rxn.ref().asReaction());
      else
         loader.loadQueryReaction(params.rxn.ref().asQueryReaction());
   }
   
   params.rmode = RENDER_RXN;
}

bool RenderParamInterface::needsLayoutSub (BaseMolecule& mol)
{
   for (int i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i)) {
      for (int j = mol.vertexNext(i); j < mol.vertexEnd(); j = mol.vertexNext(j)) {
         const Vec3f& v = mol.getAtomXyz(i);
         const Vec3f& w = mol.getAtomXyz(j);
         Vec3f d;
         d.diff(v, w);
         d.z = 0;
         if (d.length() < 1e-3)
            return true;
      }
   }
   return false;
}

bool RenderParamInterface::needsLayout (BaseMolecule& mol)
{
   if (needsLayoutSub(mol))
      return true;
   if (mol.isQueryMolecule()) {
      QueryMolecule& qmol = mol.asQueryMolecule();
      MoleculeRGroups& rGroups = qmol.rgroups;
      for (int i = 1; i <= rGroups.getRGroupCount(); ++i) {
         RGroup& rg = rGroups.getRGroup(i);
         for (int j = 0; j < rg.fragmentsCount(); ++j)
            if (needsLayoutSub(*rg.fragments[j]))
               return true;
      }
   }
   return false;
}

void RenderParamInterface::render (RenderParams& params)
{
   if (params.rmode == RENDER_NONE)
      throw Error("No object to render specified");

   RenderContext rc;
   rc.setScaleFactor(params.relativeThickness);
   rc.setDefaultScale(params.cnvOpt.bondLength);
   rc.setHighlightingOptions(&params.hlOpt);
   rc.setRenderContextOptions(&params.rcOpt);

   rc.setOutput(params.output);
   rc.setMode(params.mode);
   rc.setHDC(params.hdc);
   rc.setBackground(params.backgroundColor);
   rc.setBaseColor(params.baseColor);
   
   rc.opt.copy(params.rOpt);
   rc.cnvOpt = params.cnvOpt;

   if (params.query)
      params.rOpt.implHMode = IHM_NONE;

   RenderItemFactory factory(rc); 
   int obj = -1;
   if (params.rmode == RENDER_MOL) {
      obj = factory.addItemMolecule();
      BaseMolecule& bm = params.mol.ref();

      if (needsLayout(bm))
      {
         MoleculeLayout ml(bm);
         ml.make();
         bm.stereocenters.markBonds();
      }

      if (params.aromatization > 0)
         bm.aromatize();
      else if (params.aromatization < 0)
         bm.dearomatize();
      factory.getItemMolecule(obj).mol = &bm;
      factory.getItemMolecule(obj).highlighting = &params.molhl;
   } else if (params.rmode == RENDER_RXN) {
      obj = factory.addItemReaction();
      factory.getItemReaction(obj);
      BaseReaction& rxn = params.rxn.ref();

      for (int i = rxn.begin(); i < rxn.end(); i = rxn.next(i))
      {
         BaseMolecule& mol = rxn.getBaseMolecule(i);
         if (needsLayout(mol))
         {
            MoleculeLayout ml(mol);
            ml.make();
            mol.stereocenters.markBonds();
         }
      }

      if (params.aromatization > 0)
         rxn.aromatize();
      else if (params.aromatization < 0)
         rxn.dearomatize();

      factory.getItemReaction(obj).rxn = &rxn;
      factory.getItemReaction(obj).highlighting = &params.rhl;
   } else {
      throw Error("Invalid rendering mode: %i", params.rmode);
   }

   int comment = -1;
   if (rc.opt.comment.size() > 0) {
      comment = factory.addItemAuxiliary();
      factory.getItemAuxiliary(comment).type = RenderItemAuxiliary::AUX_TEXT;
      factory.getItemAuxiliary(comment).text.copy(rc.opt.comment);
      factory.getItemAuxiliary(comment).fontsz = FONT_SIZE_COMMENT;
   }

   RenderSingle render(rc, factory);
   render.obj = obj;
   render.comment = comment;
   render.draw();
   rc.closeContext();
}