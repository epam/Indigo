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

#include "base_cpp/array.h"
#include "base_cpp/output.h"
#include "base_cpp/os_sync_wrapper.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "reaction/reaction.h"
#include "reaction/query_reaction.h"
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
   rxns.clear();
   titles.clear();
   refAtoms.clear();
}

void RenderParams::clear ()
{
   relativeThickness = 1.0f;
   rmode = RENDER_NONE;
   mol.reset(NULL);
   rxn.reset(NULL);
   rOpt.clear();
   cnvOpt.clear();
   clearArrays();
}

IMPL_ERROR(RenderParamInterface, "render param interface");

bool RenderParamInterface::needsLayoutSub (BaseMolecule& mol)
{
   QS_DEF(RedBlackSet<int>, atomsToIgnore);
   atomsToIgnore.clear();
   for (int i = mol.multiple_groups.begin(); i < mol.multiple_groups.end(); i = mol.multiple_groups.next(i)) {
      const Array<int>& atoms = mol.multiple_groups[i].atoms;
      const Array<int>& patoms = mol.multiple_groups[i].parent_atoms;
      for (int j = 0; j < atoms.size(); ++j)
         atomsToIgnore.find_or_insert(atoms[j]);
      for (int j = 0; j < patoms.size(); ++j)
         if (atomsToIgnore.find(patoms[j]))
            atomsToIgnore.remove(patoms[j]);
   }
   for (int i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i)) {
      if (atomsToIgnore.find(i))
         continue;
      for (int j = mol.vertexNext(i); j < mol.vertexEnd(); j = mol.vertexNext(j)) {
         if (atomsToIgnore.find(j))
            continue;
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

   MoleculeRGroups& rGroups = mol.rgroups;
   for (int i = 1; i <= rGroups.getRGroupCount(); ++i) {
      PtrPool<BaseMolecule> &frags = rGroups.getRGroup(i).fragments;
      for (int j = frags.begin(); j != frags.end(); j = frags.next(j))
         if (needsLayoutSub(*frags[j]))
            return true;
   }
   return false;
}

void RenderParamInterface::_prepareMolecule (RenderParams& params, BaseMolecule& bm)
{
   if (needsLayout(bm))
   {
      MoleculeLayout ml(bm);
      ml.make();
      bm.clearBondDirections();
      bm.stereocenters.markBonds();
      bm.allene_stereo.markBonds();
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
         mol.clearBondDirections();
         mol.stereocenters.markBonds();
         mol.allene_stereo.markBonds();
      }
   }
}

void RenderParamInterface::render (RenderParams& params)
{
   // Disable multithreaded SVG rendering due to the Cairo issue. See IND-482
   OsLock *render_lock = 0;
   if (params.rOpt.mode == MODE_SVG)
   {
      static ThreadSafeStaticObj<OsLock> svg_lock;
      render_lock = svg_lock.ptr();
   }
   OsLockerNullable locker(render_lock);

   if (params.rmode == RENDER_NONE)
      throw Error("No object to render specified");

   RenderContext rc(params.rOpt, params.relativeThickness);

   bool bondLengthSet = params.cnvOpt.bondLength > 0;
   int bondLength = (int)(bondLengthSet ? params.cnvOpt.bondLength : 100);
   rc.setDefaultScale((float)bondLength); // TODO: fix bondLength type

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
      } else {
         for (int i = 0; i < params.mols.size(); ++i) {
            int mol = factory.addItemMolecule();
            BaseMolecule& bm = *params.mols[i];
            _prepareMolecule(params, bm);
            factory.getItemMolecule(mol).mol = &bm;
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
      } else {
         for (int i = 0; i < params.rxns.size(); ++i) {
            int rxn = factory.addItemReaction();
            BaseReaction& br = *params.rxns[i];
            _prepareReaction(params, br);
            factory.getItemReaction(rxn).rxn = &br;
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
      RenderSingle render(rc, factory, params.cnvOpt, bondLength, bondLengthSet);
      render.obj = obj;
      render.comment = comment;
      render.draw();
   } else {
      RenderGrid render(rc, factory, params.cnvOpt, bondLength, bondLengthSet);
      render.objs.copy(objs);
      render.comment = comment;
      render.titles.copy(titles);
      render.refAtoms.copy(params.refAtoms);
      render.draw();
   }
   rc.closeContext(false);
}