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

#include "molecule/molecule.h"
#include "molecule/canonical_smiles_saver.h"
#include "molecule/smiles_saver.h"
#include "molecule/molecule_automorphism_search.h"
#include "molecule/elements.h"
#include "molecule/molecule_dearom.h"

using namespace indigo;

IMPL_ERROR(CanonicalSmilesSaver, "canonical SMILES saver");

CanonicalSmilesSaver::CanonicalSmilesSaver (Output &output) : _output(output)
{
   find_invalid_stereo = true;
}

CanonicalSmilesSaver::~CanonicalSmilesSaver ()
{
}

void CanonicalSmilesSaver::saveMolecule (Molecule &mol_) const
{
   if (mol_.vertexCount() < 1)
      return;

   QS_DEF(Array<int>, ignored);
   QS_DEF(Array<int>, order);
   QS_DEF(Array<int>, ranks);
   QS_DEF(Molecule, mol);
   int i;

   if (mol_.repeating_units.size() > 0)
      throw Error("can not canonicalize a polymer");

   // Detect hydrogens configuration if aromatic but not ambiguous
   bool found_invalid_h = false;
   for (i = mol_.vertexBegin(); i != mol_.vertexEnd(); i = mol_.vertexNext(i))
   {
      if (mol_.isRSite(i) || mol_.isPseudoAtom(i))
         continue;

      if (mol_.getImplicitH_NoThrow(i, -1) == -1)
         found_invalid_h = true;
   }
   if (found_invalid_h)
   {
      AromaticityOptions options;
      options.method = AromaticityOptions::GENERIC;
      options.unique_dearomatization = true;
      MoleculeDearomatizer::restoreHydrogens(mol_, options);
   }

   mol.clone(mol_, 0, 0);

   // TODO: canonicalize allenes properly
   mol.allene_stereo.clear();

   ignored.clear_resize(mol.vertexEnd());
   ignored.zerofill();

   for (i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
      if (mol.convertableToImplicitHydrogen(i))
         ignored[i] = 1;

   for (i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
      if (mol.getBondTopology(i) == TOPOLOGY_RING && mol.cis_trans.getParity(i) != 0)
      {
         // we save cis/trans ring bonds into SMILES, but only those who
         // do not participate in bigger ring systems
         const Edge &edge = mol.getEdge(i);

         if (mol.getAtomRingBondsCount(edge.beg) != 2 ||
             mol.getAtomRingBondsCount(edge.end) != 2)
         {
            mol.cis_trans.setParity(i, 0);
            continue;
         }

         // also, discard the cis-trans bonds that have been converted to aromatic
         const Vertex &beg = mol.getVertex(edge.beg);
         const Vertex &end = mol.getVertex(edge.end);
         bool have_singlebond_beg = false;
         bool have_singlebond_end = false;
         int j;
         
         for (j = beg.neiBegin(); j != beg.neiEnd(); j = beg.neiNext(j))
            if (mol.getBondOrder(beg.neiEdge(j)) == BOND_SINGLE)
               have_singlebond_beg = true;

         for (j = end.neiBegin(); j != end.neiEnd(); j = end.neiNext(j))
            if (mol.getBondOrder(end.neiEdge(j)) == BOND_SINGLE)
               have_singlebond_end = true;

         if (!have_singlebond_beg || !have_singlebond_end)
         {
            mol.cis_trans.setParity(i, 0);
            continue;
         }
      }
         
   MoleculeAutomorphismSearch of;

   of.detect_invalid_cistrans_bonds = find_invalid_stereo;
   of.detect_invalid_stereocenters = find_invalid_stereo;
   of.find_canonical_ordering = true;
   of.ignored_vertices = ignored.ptr();
   of.process(mol);
   of.getCanonicalNumbering(order);

   for (i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
      if (mol.cis_trans.getParity(i) != 0 && of.invalidCisTransBond(i))
         mol.cis_trans.setParity(i, 0);

   for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
      if (mol.stereocenters.getType(i) > MoleculeStereocenters::ATOM_ANY && of.invalidStereocenter(i))
         mol.stereocenters.remove(i);

   ranks.clear_resize(mol.vertexEnd());

   for (i = 0; i < order.size(); i++)
      ranks[order[i]] = i;

   SmilesSaver saver(_output);

   saver.ignore_invalid_hcount = false;
   saver.vertex_ranks = ranks.ptr();
   saver.ignore_hydrogens = true;
   saver.canonize_chiralities = true;
   saver.saveMolecule(mol);
}
