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
#include "render_grid.h"

using namespace indigo;

RenderParams::RenderParams ()
{
   clear();
}

RenderParams::~RenderParams ()
{
}

void RenderParams::clearArrays ()
{
   mols.clear();
   molhls.clear();
   rxns.clear();
   rxnhls.clear();
   titles.clear();
   refAtoms.clear();
}

void RenderParams::clear ()
{
   relativeThickness = 1.0f;
   rmode = RENDER_NONE;
   mol.reset(NULL);
   molhl.clear();
   rxn.reset(NULL);
   rhl.clear();
   rOpt.clear();
   cnvOpt.clear();
   clearArrays();
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

void RenderParamInterface::_prepareMolecule (RenderParams& params, BaseMolecule& bm)
{
   if (needsLayout(bm))
   {
      MoleculeLayout ml(bm);
      ml.make();
      bm.stereocenters.markBonds();
   }
}

void RenderParamInterface::_prepareReaction (RenderParams& params, BaseReaction& rxn)
{
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
}

void RenderParamInterface::render (RenderParams& params)
{
   if (params.rmode == RENDER_NONE)
      throw Error("No object to render specified");

   RenderContext rc(params.rOpt, params.relativeThickness);
   rc.setDefaultScale(params.cnvOpt.bondLength);
   
   RenderItemFactory factory(rc); 
   int obj = -1;
   Array<int> objs;
   Array<int> titles;
   if (params.rmode == RENDER_MOL) {
      if (params.mols.size() == 0) {
         obj = factory.addItemMolecule();
         BaseMolecule& bm = params.mol.ref();
         _prepareMolecule(params, bm);
         factory.getItemMolecule(obj).mol = &bm;
         factory.getItemMolecule(obj).highlighting = &params.molhl;
      } else {
         for (int i = 0; i < params.mols.size(); ++i) {
            int mol = factory.addItemMolecule();
            BaseMolecule& bm = *params.mols[i];
            _prepareMolecule(params, bm);
            factory.getItemMolecule(mol).mol = &bm;
            factory.getItemMolecule(mol).highlighting = &params.molhls[i];
            objs.push(mol);

            if (params.titles.size() > 0) {
               int title = factory.addItemAuxiliary();
               factory.getItemAuxiliary(title).type = RenderItemAuxiliary::AUX_TITLE;
               factory.getItemAuxiliary(title).text.copy(params.titles[i]);
               titles.push(title);
            }
         }
      }
   } else if (params.rmode == RENDER_RXN) {
      if (params.rxns.size() == 0) {
         obj = factory.addItemReaction();
         factory.getItemReaction(obj);
         BaseReaction& rxn = params.rxn.ref();
         _prepareReaction(params, rxn);
         factory.getItemReaction(obj).rxn = &rxn;
         factory.getItemReaction(obj).highlighting = &params.rhl;
      } else {
         for (int i = 0; i < params.rxns.size(); ++i) {
            int rxn = factory.addItemReaction();
            BaseReaction& br = *params.rxns[i];
            _prepareReaction(params, br);
            factory.getItemReaction(rxn).rxn = &br;
            factory.getItemReaction(rxn).highlighting = &params.rxnhls[i];
            objs.push(rxn);

            if (params.titles.size() > 0) {
               int title = factory.addItemAuxiliary();
               factory.getItemAuxiliary(title).type = RenderItemAuxiliary::AUX_TITLE;
               factory.getItemAuxiliary(title).text.copy(params.titles[i]);
               titles.push(title);
            }
         }
      }
   } else {
      throw Error("Invalid rendering mode: %i", params.rmode);
   }

   int comment = -1;
   if (params.cnvOpt.comment.size() > 0) {
      comment = factory.addItemAuxiliary();
      factory.getItemAuxiliary(comment).type = RenderItemAuxiliary::AUX_COMMENT;
      factory.getItemAuxiliary(comment).text.copy(params.cnvOpt.comment);
   }

   if (obj >= 0) {
      RenderSingle render(rc, factory, params.cnvOpt);
      render.obj = obj;
      render.comment = comment;
      render.draw();
   } else {
      RenderGrid render(rc, factory, params.cnvOpt);
      render.objs.copy(objs);
      render.comment = comment;
      render.titles.copy(titles);
      render.refAtoms.copy(params.refAtoms);
      render.draw();
   }
   rc.closeContext();
}