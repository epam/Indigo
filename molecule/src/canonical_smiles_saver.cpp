/****************************************************************************
 * Copyright (C) 2009-2014 GGA Software Services LLC
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

#include "molecule/canonical_smiles_saver.h"

#include "base_cpp/output.h"
#include "base_cpp/tlscont.h"
#include "molecule/molecule.h"
#include "molecule/smiles_saver.h"
#include "molecule/molecule_automorphism_search.h"
#include "molecule/elements.h"
#include "molecule/molecule_dearom.h"

using namespace indigo;

IMPL_ERROR(CanonicalSmilesSaver, "canonical SMILES saver");

CanonicalSmilesSaver::CanonicalSmilesSaver (Output &output) : SmilesSaver(output),
TL_CP_GET(_actual_atom_atom_mapping),
TL_CP_GET(_initial_to_actual)
{
   find_invalid_stereo = true;
   ignore_invalid_hcount = false;
   ignore_hydrogens = true;
   canonize_chiralities = true;
   initial_atom_atom_mapping = 0;
   _initial_to_actual.clear();
   _initial_to_actual.insert(0, 0);
   _aam_counter = 0;
}

CanonicalSmilesSaver::~CanonicalSmilesSaver ()
{
}

void CanonicalSmilesSaver::saveMolecule (Molecule &mol_)
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
   // We can store this infromation in the original structure mol_.
   mol_.restoreAromaticHydrogens();

   mol.clone(mol_, 0, 0);

   // TODO: canonicalize allenes properly
   mol.allene_stereo.clear();

   ignored.clear_resize(mol.vertexEnd());
   ignored.zerofill();

   for (i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
      if (mol.convertableToImplicitHydrogen(i))
         ignored[i] = 1;

   // Try to save into ordinary smiles and find what cis-trans bonds were used
   NullOutput null_output;
   SmilesSaver saver_cistrans(null_output);
   saver_cistrans.ignore_hydrogens = true;
   saver_cistrans.saveMolecule(mol);
   // Then reset cis-trans infromation that is not saved into SMILES
   const Array<int>& parities = saver_cistrans.getSavedCisTransParities();
   for (i = mol.edgeBegin(); i < mol.edgeEnd(); i = mol.edgeNext(i))
   {
      if (mol.cis_trans.getParity(i) != 0 && parities[i] == 0)
         mol.cis_trans.setParity(i, 0);
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

   vertex_ranks = ranks.ptr();

   if (initial_atom_atom_mapping) {
      _actual_atom_atom_mapping.clear_resize(initial_atom_atom_mapping->size());
      _actual_atom_atom_mapping.fill(0);

      for (int i = 0; i < order.size(); ++i) {
         int aam = initial_atom_atom_mapping->at(order[i]);
         if (aam) {
            if (!_initial_to_actual.find(aam)) {
               _initial_to_actual.insert(aam, ++_aam_counter);
               _actual_atom_atom_mapping[order[i]] = _aam_counter;
            }
            else {
               _actual_atom_atom_mapping[order[i]] = _initial_to_actual.at(aam);
            }
         }
      }
      atom_atom_mapping = _actual_atom_atom_mapping.ptr();
   }
   else {
      atom_atom_mapping = 0;
   }

   SmilesSaver::saveMolecule(mol);
}
