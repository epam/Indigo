/****************************************************************************
 * Copyright (C) 2009-2011 GGA Software Services LLC
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

using namespace indigo;

CanonicalSmilesSaver::CanonicalSmilesSaver (Output &output) : _output(output)
{
   find_invalid_stereo = true;
   ignore_invalid_hcount = false;
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

   mol.clone(mol_, 0, 0);

   ignored.clear_resize(mol.vertexEnd());
   ignored.zerofill();

   for (i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
      if (mol.convertableToImplicitHydrogen(i))
         ignored[i] = 1;

   for (i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
      if (mol.getBondTopology(i) == TOPOLOGY_RING)
         mol.cis_trans.setParity(i, 0);

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

   saver.ignore_invalid_hcount = ignore_invalid_hcount;
   saver.vertex_ranks = ranks.ptr();
   saver.ignore_hydrogens = true;
   saver.canonize_chiralities = true;
   saver.saveMolecule(mol);
}
