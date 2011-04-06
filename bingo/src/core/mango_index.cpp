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

#include "core/mango_index.h"
#include "molecule/elements.h"
#include "base_cpp/scanner.h"
#include "base_cpp/os_sync_wrapper.h"
#include "molecule/molecule.h"
#include "molecule/molecule_auto_loader.h"
#include "core/bingo_error.h"
#include "molecule/gross_formula.h"
#include "molecule/molecule_mass.h"
#include "core/bingo_context.h"
#include "core/mango_matchers.h"
#include "molecule/cmf_saver.h"
#include "base_cpp/profiling.h"
#include "molecule/molecule_arom.h"

const int MangoIndex::counted_elements[6] = {ELEM_C, ELEM_N, ELEM_O, ELEM_P, ELEM_S, ELEM_H};

void MangoIndex::prepare (Scanner &molfile, Output &output, 
                          OsLock *lock_for_exclusive_access)
{
   QS_DEF(Molecule, mol);

   QS_DEF(Array<int>, gross);

   MoleculeAutoLoader loader(molfile);

   loader.treat_x_as_pseudoatom = _context->treat_x_as_pseudoatom;
   loader.ignore_closing_bond_direction_mismatch =
           _context->ignore_closing_bond_direction_mismatch;
   loader.skip_3d_chirality = true;
   loader.loadMolecule(mol);

   Molecule::checkForConsistency(mol);

   // Make aromatic molecule
   MoleculeAromatizer::aromatizeBonds(mol);

   MangoExact::calculateHash(mol, _hash);

   if (!skip_calculate_fp)
   {
      MoleculeFingerprintBuilder builder(mol, _context->fp_parameters);
      profTimerStart(tfing, "moleculeIndex.createFingerprint");
      builder.process();
      profTimerStop(tfing);

      _fp.copy(builder.get(), _context->fp_parameters.fingerprintSize());
      _fp_sim_bits_count = builder.countBits_Sim();
      output.writeBinaryWord((word)_fp_sim_bits_count);

      const byte *fp_sim_ptr = builder.getSim();
      int fp_sim_size = _context->fp_parameters.fingerprintSizeSim();

      ArrayOutput fp_sim_output(_fp_sim_str);

      for (int i = 0; i < fp_sim_size; i++)
         fp_sim_output.printf("%02X", fp_sim_ptr[i]);

      fp_sim_output.writeChar(0);
   }

   ArrayOutput output_cmf(_cmf);
   {
      // CmfSaver modifies _context->cmf_dict and 
      // requires exclusive access for this
      OsLockerNullable locker(lock_for_exclusive_access);

      CmfSaver saver(_context->cmf_dict, output_cmf);

      saver.saveMolecule(mol);
      
      if (mol.have_xyz)
      {
         ArrayOutput output_xyz(_xyz);
         saver.saveXyz(output_xyz);
      }
      else
         _xyz.clear();
   }

   output.writeArray(_cmf);

   // Save gross formula
   GrossFormula::collect(mol, gross);
   GrossFormula::toString(gross, _gross_str);

   _counted_elems_str.clear();

   ArrayOutput ce_output(_counted_elems_str);

   for (int i = 0; i < (int)NELEM(counted_elements); i++)
      ce_output.printf(", %d", gross[counted_elements[i]]);

   ce_output.writeByte(0);

   // Calculate molecular mass
   MoleculeMass mass_calulator;
   mass_calulator.relative_atomic_mass_map = &_context->relative_atomic_mass_map;
   _molecular_mass = mass_calulator.molecularWeight(mol);
}


const MangoExact::Hash& MangoIndex::getHash () const
{
   return _hash;
}

const char * MangoIndex::getGrossString () const
{
   return _gross_str.ptr();
}

const char * MangoIndex::getCountedElementsString() const
{
   return (const char *)_counted_elems_str.ptr();
}

const Array<char> & MangoIndex::getCmf () const
{
   return _cmf;
}

const Array<char> & MangoIndex::getXyz () const
{
   return _xyz;
}

const byte * MangoIndex::getFingerprint () const
{
   return _fp.ptr();
}

const char * MangoIndex::getFingerprint_Sim_Str () const
{
   return _fp_sim_str.ptr();
}

float MangoIndex::getMolecularMass () const
{
   return _molecular_mass;
}

int MangoIndex::getFpSimilarityBitsCount () const
{
   return _fp_sim_bits_count;
}

void MangoIndex::clear ()
{
   _cmf.clear();
   _xyz.clear();
   _hash.clear(); 

   _gross.clear();
   _gross_str.clear();

   _fp.clear();
   _fp_sim_str.clear();
           
   _counted_elems_str.clear();
   _molecular_mass = -1;
   _fp_sim_bits_count = -1;
}
