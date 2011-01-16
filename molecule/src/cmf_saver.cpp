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
#include "base_cpp/tlscont.h"
#include "base_cpp/output.h"

#include "molecule/molecule.h"
#include "molecule/molecule_stereocenters.h"
#include "molecule/molecule_cis_trans.h"
#include "molecule/cmf_saver.h"
#include "molecule/cmf_symbol_codes.h"

#include "graph/dfs_walk.h"

using namespace indigo;

CmfSaver::CmfSaver (LzwDict &dict, Output &output) :
TL_CP_GET(_atom_sequence)
{
   _init();

   if (!dict.isInitialized())
      dict.init(CMF_ALPHABET_SIZE, CMF_BIT_CODE_SIZE);

   _encoder_obj.create(dict, output);
   _encoder = _encoder_obj.get();
}

CmfSaver::CmfSaver (LzwEncoder &encoder) :
TL_CP_GET(_atom_sequence)
{
   _init();
   _encoder = &encoder;
}

CmfSaver::CmfSaver (Output &output) :
TL_CP_GET(_atom_sequence)
{
   _init();
   _output = &output;
}

void CmfSaver::_init ()
{
   atom_flags = 0;
   bond_flags = 0;
   _encoder = 0;
   _output = 0;
   _mol = 0;
   skip_implicit_h = false;
}

void CmfSaver::saveMolecule (Molecule &mol)
{
   /* Walk molecule */
   DfsWalk walk(mol);
   QS_DEF(Array<int>, mapping);

   if (_encoder != 0)
      _encoder->start();

   walk.walk();

   /* Get walking sequence */
   const Array<DfsWalk::SeqElem> &v_seq = walk.getSequence();

   /* Calculate mapping to the encoded molecule */
   walk.calcMapping(mapping);
   
   QS_DEF(Array<int>, branch_counters);
   QS_DEF(Array<int>, cycle_numbers);

   branch_counters.clear_resize(mol.vertexEnd());
   branch_counters.zerofill();
   cycle_numbers.clear();

   _atom_sequence.clear();

   /* Encode first atom */
   if (v_seq.size() > 0)
   {
      _encodeAtom(mol, v_seq[0].idx, mapping.ptr());
      _atom_sequence.push(v_seq[0].idx);
      
      int j, openings = walk.numOpenings(v_seq[0].idx);

      for (j = 0; j < openings; j++)
      {
         cycle_numbers.push(v_seq[0].idx);
         _encodeCycleNumer(j);
      }
   }

   /* Main cycle */
   int i, j, k;

   for (i = 1; i < v_seq.size(); i++)
   {
      int v_idx = v_seq[i].idx;
      int e_idx = v_seq[i].parent_edge;
      int v_prev_idx = v_seq[i].parent_vertex;
      bool write_atom = true;

      if (v_prev_idx >= 0)
      {
         if (walk.numBranches(v_prev_idx) > 1)
            if (branch_counters[v_prev_idx] > 0)
               _encode(CMF_CLOSE_BRACKET);

         int branches = walk.numBranches(v_prev_idx);

         if (branches > 1)
            if (branch_counters[v_prev_idx] < branches - 1)
               _encode(CMF_OPEN_BRACKET);

         branch_counters[v_prev_idx]++;

         if (branch_counters[v_prev_idx] > branches)
            throw Error("unexpected branch");

         _encodeBond(mol, e_idx, mapping.ptr());
         
         if (walk.isClosure(e_idx))
         {
            for (j = 0; j < cycle_numbers.size(); j++)
               if (cycle_numbers[j] == v_idx)
                  break;

            if (j == cycle_numbers.size())
               throw Error("cycle number not found");

            _encodeCycleNumer(j);

            cycle_numbers[j] = -1;
            write_atom = false;
         }
      }
      else
         _encode(CMF_SEPARATOR);

      if (write_atom)
      {
         _encodeAtom(mol, v_idx, mapping.ptr());
         _atom_sequence.push(v_idx);
         
         int openings = walk.numOpenings(v_idx);

         for (j = 0; j < openings; j++)
         {
            for (k = 0; k < cycle_numbers.size(); k++)
               if (cycle_numbers[k] == -1)
                  break;
            if (k == cycle_numbers.size())
               cycle_numbers.push(v_idx);
            else
               cycle_numbers[k] = v_idx;

            _encodeCycleNumer(k);
         }
      }
   }

   _encode(CMF_TERMINATOR);
   
   // if have internal encoder, finish it
   if (_encoder_obj.get() != 0)
      _encoder_obj->finish();

   // for saveXyz()
   _mol = &mol;
}

void CmfSaver::_encodeAtom (Molecule &mol, int idx, const int *mapping)
{
   int number = 0;
   
   if (mol.isPseudoAtom(idx))
   {
      const char *str = mol.getPseudoAtom(idx);

      int len = strlen(str);

      if (len < 1)
         throw Error("empty pseudo-atom");
      if (len > 255)
         throw Error("pseudo-atom labels %d characters long are not supported (255 is the limit)", len);

      _encode(CMF_PSEUDOATOM);
      _encode(len);
      
      do
      {
         _encode(*str);
      } while (*(++str) != 0);
   }
   else if (mol.isRSite(idx))
   {
      _encode(CMF_RSITE);
      int bits = mol.getRSiteBits(idx);
      if (bits > 255)
         throw Error("R-site numbers higher than 7 are not supported");
      _encode(bits);
   }
   else
   {
      number = mol.getAtomNumber(idx);

      if (number <= 0 || number >= ELEM_MAX)
         throw Error("unexpected atom label");

      _encode(number);
   }

   int charge = mol.getAtomCharge(idx);

   if (charge != 0)
   {
      int charge2 = charge - CMF_MIN_CHARGE;
         
      if (charge2 < 0 || charge2 >= CMF_NUM_OF_CHARGES)
         throw Error("unexpected atom charge: %d", charge);

      _encode(charge2 + CMF_CHARGES);
   }

   int isotope = mol.getAtomIsotope(idx);

   if (isotope > 0)
   {
      int deviation = isotope - Element::getDefaultIsotope(number);

      deviation -= CMF_MIN_MASS_DIFF;

      if (deviation < 0 || deviation >= CMF_NUM_OF_ISOTOPES)
         throw Error("unexpected %s isotope: %d", Element::toString(number), isotope);

      _encode(deviation + CMF_ISOTOPES);
   }

   int radical = 0;
   if (!mol.isPseudoAtom(idx) && !mol.isRSite(idx))
      radical = mol.getAtomRadical(idx);

   if (radical > 0)
   {
      if (radical == RADICAL_SINGLET)
         _encode(CMF_RADICAL_SINGLET);
      else if (radical == RADICAL_DOUPLET)
         _encode(CMF_RADICAL_DOUPLET);
      else if (radical == RADICAL_TRIPLET)
         _encode(CMF_RADICAL_TRIPLET);
      else
         throw Error("bad radical value: %d", radical);
   }
   
   MoleculeStereocenters &stereo = mol.stereocenters;
   
   int stereo_type = stereo.getType(idx);
   
   if (stereo_type == MoleculeStereocenters::ATOM_ANY)
      _encode(CMF_STEREO_ANY);
   else if (stereo_type != 0)
   {
      bool rigid;
      int code;
      const int *pyramid = stereo.getPyramid(idx);
      
      if (pyramid[3] == -1)
         rigid = MoleculeStereocenters::isPyramidMappingRigid(pyramid, 3, mapping);
      else
         rigid = MoleculeStereocenters::isPyramidMappingRigid(pyramid, 4, mapping);
      
      if (stereo_type == MoleculeStereocenters::ATOM_ABS)
         code = CMF_STEREO_ABS_0;
      else 
      {
         int group = stereo.getGroup(idx);

         if (group < 1 || group > CMF_MAX_STEREOGROUPS)
            throw Error("stereogroup number %d out of range", group);

         if (stereo_type == MoleculeStereocenters::ATOM_AND)
            code = CMF_STEREO_AND_0 + group - 1;
         else // stereo_type == MoleculeStereocenters::ATOM_OR
            code = CMF_STEREO_OR_0 + group - 1;
      }
      
      if (!rigid)
         // CMF_STEREO_*_0 -> CMF_STEREO_*_1
         code += CMF_MAX_STEREOGROUPS * 2 + 1;
      
      _encode(code);
   }


   int impl_h = 0;

   if (!mol.isPseudoAtom(idx) && !mol.isRSite(idx))
      impl_h = mol.getImplicitH(idx);

   if (impl_h != 0)
   {
      if (impl_h < 0 || impl_h > CMF_MAX_IMPLICIT_H)
         throw Error("implicit hydrogens count %d out of range", impl_h);

      _encode(CMF_IMPLICIT_H + impl_h - 1);
   }

   int valence = -1;
   
   // explicit valence?
   if (mol.getExplicitValence(idx) > 0)
      valence = mol.getExplicitValence(idx);
   else if (!mol.isRSite(idx) && !mol.isPseudoAtom(idx))
   {
      // valence that can not be trivially restored by atom number and charge?
      if (Element::calcValenceByCharge(number, charge) == -1)
      {
         int conn = mol.calcAtomConnectivity_noImplH(idx);
         int normal_val = -1, normal_h = -1;

         if (conn != -1)
            Element::calcValence(number, charge, radical, conn, normal_val, normal_h, true);

         // valence that can not be restored from bonds?
         if (mol.getAtomValence(idx) != normal_val)
            valence = mol.getAtomValence(idx);
      }
   }

   if (valence > 0 && valence <= CMF_MAX_VALENCE)
      _encode(CMF_VALENCE + valence - 1);
   
   if (atom_flags != 0)
   {
      int i, flags = atom_flags[idx];

      for (i = 0; i < CMF_NUM_OF_ATOM_FLAGS; i++)
         if (flags & (1 << i))
            _encode(CMF_ATOM_FLAGS + i);
   }
}

void CmfSaver::_encodeBond (Molecule &mol, int idx, const int *mapping)
{
   int order = mol.getBondOrder(idx);

   if (order == BOND_SINGLE)
   {
      if (mol.getBondTopology(idx) == TOPOLOGY_RING)
         _encode(CMF_BOND_SINGLE_RING);
      else
         _encode(CMF_BOND_SINGLE_CHAIN);
   }
   else if (order == BOND_DOUBLE)
   {
      int parity = mol.cis_trans.getParity(idx);

      if (parity != 0)
      {
         parity = mol.cis_trans.applyMapping(parity, mol.cis_trans.getSubstituents(idx), mapping);

         if (parity == MoleculeCisTrans::CIS)
         {
            if (mol.getBondTopology(idx) == TOPOLOGY_RING)
               _encode(CMF_BOND_DOUBLE_RING_CIS);
            else
               _encode(CMF_BOND_DOUBLE_CHAIN_CIS);
         }
         else // parity == MoleculeCisTrans::TRANS
         {
            if (mol.getBondTopology(idx) == TOPOLOGY_RING)
               _encode(CMF_BOND_DOUBLE_RING_TRANS);
            else
               _encode(CMF_BOND_DOUBLE_CHAIN_TRANS);
         }
      }
      else
      {
         if (mol.getBondTopology(idx) == TOPOLOGY_RING)
            _encode(CMF_BOND_DOUBLE_RING);
         else
            _encode(CMF_BOND_DOUBLE_CHAIN);
      }
   }
   else if (order == BOND_TRIPLE)
   {
      if (mol.getBondTopology(idx) == TOPOLOGY_RING)
         _encode(CMF_BOND_TRIPLE_RING);
      else
         _encode(CMF_BOND_TRIPLE_CHAIN);
   }
   else if (order == BOND_AROMATIC)
      _encode(CMF_BOND_AROMATIC);
   else
      throw Error("bad bond order: %d", order);

   if (bond_flags != 0)
   {
      int i, flags = bond_flags[idx];

      for (i = 0; i < CMF_NUM_OF_BOND_FLAGS; i++)
         if (flags & (1 << i))
            _encode(CMF_BOND_FLAGS + i);
   }
}

void CmfSaver::_encodeCycleNumer (int n)
{
   while (n >= CMF_NUM_OF_CYCLES)
   {
      _encode(CMF_CYCLES_PLUS);
      n -= CMF_NUM_OF_CYCLES;
   }
   _encode(CMF_CYCLES + n);
}

void CmfSaver::_encode (int symbol)
{
   if (_output != 0)
      _output->writeByte(symbol);
   else if (_encoder != 0)
      _encoder->send(symbol);
   else
      throw Error("no _output, no _encoder");
}

void CmfSaver::saveXyz (Output &output)
{
   int i;

   if (_mol == 0)
      throw Error("saveMolecule() must be called prior to saveXyz()");

   if (!_mol->have_xyz)
      throw Error("saveXyz(): molecule has no XYZ");

   Vec3f xyz_min( 10000,  10000,  10000);
   Vec3f xyz_max(-10000, -10000, -10000);
   Vec3f xyz_range;
   bool  have_z;

   for (i = 0; i < _atom_sequence.size(); i++)
   {
      const Vec3f &pos = _mol->getAtomXyz(_atom_sequence[i]);

      xyz_min.min(pos);
      xyz_max.max(pos);
   }

   xyz_range.diff(xyz_max, xyz_min);

   output.writeBinaryFloat(xyz_min.x);
   output.writeBinaryFloat(xyz_min.y);
   output.writeBinaryFloat(xyz_min.z);
   output.writeBinaryFloat(xyz_range.x);
   output.writeBinaryFloat(xyz_range.y);
   output.writeBinaryFloat(xyz_range.z);
   
   if (xyz_range.z < EPSILON)
   {
      have_z = false;
      output.writeByte(0);
   }
   else
   {
      have_z = true;
      output.writeByte(1);
   }

   for (i = 0; i < _atom_sequence.size(); i++)
   {
      const Vec3f &pos = _mol->getAtomXyz(_atom_sequence[i]);

      if (xyz_range.x > EPSILON)
         output.writeBinaryWord((word)(((pos.x - xyz_min.x) / xyz_range.x) * 65535));
      else
         output.writeBinaryWord(0);

      if (xyz_range.y > EPSILON)
         output.writeBinaryWord((word)(((pos.y - xyz_min.y) / xyz_range.y) * 65535));
      else
         output.writeBinaryWord(0);

      if (have_z)
         output.writeBinaryWord((word)(((pos.z - xyz_min.z) / (xyz_max.z - xyz_min.z)) * 65535));
   }
}

const Array<int> & CmfSaver::getAtomSequence ()
{
   return _atom_sequence;
}
