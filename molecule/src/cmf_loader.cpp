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

#include "base_cpp/scanner.h"

#include "molecule/molecule.h"
#include "molecule/cmf_loader.h"
#include "molecule/cmf_symbol_codes.h"

using namespace indigo;

CmfLoader::CmfLoader (LzwDict &dict, Scanner &scanner) :
TL_CP_GET(_atoms), TL_CP_GET(_bonds), TL_CP_GET(_pseudo_labels)
{
   _init();
   _decoder_obj.create(dict, scanner);
   _decoder = _decoder_obj.get();
//   _decoder->setScanner(&scanner);
}

CmfLoader::CmfLoader (Scanner &scanner) :
TL_CP_GET(_atoms), TL_CP_GET(_bonds), TL_CP_GET(_pseudo_labels)
{
   _init();
   _scanner = &scanner;
}

CmfLoader::CmfLoader (LzwDecoder &decoder) :
TL_CP_GET(_atoms), TL_CP_GET(_bonds), TL_CP_GET(_pseudo_labels)
{
   _init();
   _decoder = &decoder;
}

void CmfLoader::_init ()
{
   skip_cistrans = false;
   skip_stereocenters = false;
   skip_valence = false;
   _decoder = 0;
   _scanner = 0;
   atom_flags = 0;
   bond_flags = 0;
}

bool CmfLoader::_getNextCode (int &code)
{
   if (_decoder != 0)
   {
      if (_decoder->isEOF())
         return false;
      code = _decoder->get();
      return true;
   }

   if (_scanner != 0)
   {
      if (_scanner->isEOF())
         return false;
      code = _scanner->readByte();
      return true;
   }

   throw Error("no _decoder, no _scanner");
}

bool CmfLoader::_readAtom (int &code, _AtomDesc &atom)
{
   memset(&atom, 0, sizeof(atom));

   atom.pseudo_atom_idx = -1;
   atom.rsite = false;
   
   if (code > 0 && code < ELEM_MAX)
      atom.label = code;
   else if (code == CMF_PSEUDOATOM)
   {
      int len;

      if (!_getNextCode(len))
         throw Error("pseudo-atom identifier must be followed by length");

      if (len < 1)
         throw Error("empty pseudo-atom");

      atom.pseudo_atom_idx = _pseudo_labels.add(len + 1);
      char *label = _pseudo_labels.at(atom.pseudo_atom_idx);

      for (int i = 0; i < len; i++)
      {
         int c;
         if (!_getNextCode(c))
            throw Error("pseudo-atom label is incomplete");
         label[i] = c;
      }

      label[len] = 0;
   }
   else if (code == CMF_RSITE)
   {
      atom.label = ELEM_RSITE;
      _getNextCode(atom.rsite_bits);
   }
   else
      throw Error("bad atom number: %d", code);

   if (!_getNextCode(code))
      return false;

   if (code >= CMF_CHARGES &&
       code <  CMF_CHARGES + CMF_NUM_OF_CHARGES && code != CMF_SEPARATOR)
   {
      int charge = code - CMF_CHARGES;

      charge += CMF_MIN_CHARGE;

      atom.charge = charge;

      if (!_getNextCode(code))
         return false;
   }

   if (code >= CMF_ISOTOPES && code < CMF_ISOTOPES + CMF_NUM_OF_ISOTOPES)
   {
      int deviation = code - CMF_ISOTOPES;

      deviation += CMF_MIN_MASS_DIFF;

      atom.isotope = Element::getDefaultIsotope(atom.label) + deviation;

      if (!_getNextCode(code))
         return false;
   }
   
   if (code >= CMF_RADICAL_SINGLET && code <= CMF_RADICAL_TRIPLET)
   {
      if (code == CMF_RADICAL_SINGLET)
         atom.radical = RADICAL_SINGLET;
      else if (code == CMF_RADICAL_DOUPLET)
         atom.radical = RADICAL_DOUPLET;
      else // code == CMF_RADICAL_TRIPLET
         atom.radical = RADICAL_TRIPLET;
      
      if (!_getNextCode(code))
         return false;
   }
   
   if (code >= CMF_STEREO_ANY && code <= CMF_STEREO_ABS_1)
   {
      if (code >= CMF_STEREO_AND_1)
      {
         /* CMF_STEREO_*_1 -> CMF_STEREO_*_0 */
         code -= CMF_MAX_STEREOGROUPS * 2 + 1;
         atom.stereo_invert_pyramid = true;
      }
      
      if (code == CMF_STEREO_ANY)
         atom.stereo_type = MoleculeStereocenters::ATOM_ANY;
      else if (code == CMF_STEREO_ABS_0)
         atom.stereo_type = MoleculeStereocenters::ATOM_ABS;
      else if (code < CMF_STEREO_OR_0)
      {
         atom.stereo_type = MoleculeStereocenters::ATOM_AND;
         atom.stereo_group = code - CMF_STEREO_AND_0 + 1;
      }
      else
      {
         atom.stereo_type = MoleculeStereocenters::ATOM_OR;
         atom.stereo_group = code - CMF_STEREO_OR_0 + 1;
      }
      if (!_getNextCode(code))
         return false;
   }

   if (code >= CMF_IMPLICIT_H && code <= CMF_IMPLICIT_H + CMF_MAX_IMPLICIT_H)
   {
      atom.hydrogens = code - CMF_IMPLICIT_H;
      if (!_getNextCode(code))
         return false;
   }

   if (code >= CMF_VALENCE && code <= CMF_VALENCE + CMF_MAX_VALENCE)
   {
      atom.valence = code - CMF_VALENCE;
      if (!_getNextCode(code))
         return false;
   }
   
   while (code >= CMF_ATOM_FLAGS && code < CMF_ATOM_FLAGS + CMF_NUM_OF_ATOM_FLAGS)
   {
      atom.flags |= (1 << (code - CMF_ATOM_FLAGS));
      if (!_getNextCode(code))
         return false;
   }
   
   return true;
}

void CmfLoader::_readBond (int &code, _BondDesc &bond)
{
   bond.cis_trans = 0;
   bond.flags = 0;
   
   if (code == CMF_BOND_SINGLE_CHAIN)
   {
      bond.type = BOND_SINGLE;
      bond.in_ring = false;
   }
   else if (code == CMF_BOND_SINGLE_RING)
   {
      bond.type = BOND_SINGLE;
      bond.in_ring = true;
   }
   else if (code == CMF_BOND_DOUBLE_CHAIN)
   {
      bond.type = BOND_DOUBLE;
      bond.in_ring = false;
   }
   else if (code == CMF_BOND_DOUBLE_RING)
   {
      bond.type = BOND_DOUBLE;
      bond.in_ring = true;
   }
   else if (code == CMF_BOND_DOUBLE_CHAIN_CIS)
   {
      bond.type = BOND_DOUBLE;
      bond.in_ring = false;
      bond.cis_trans = MoleculeCisTrans::CIS;
   }
   else if (code == CMF_BOND_DOUBLE_CHAIN_TRANS)
   {
      bond.type = BOND_DOUBLE;
      bond.in_ring = false;
      bond.cis_trans = MoleculeCisTrans::TRANS;
   }
   else if (code == CMF_BOND_DOUBLE_RING_CIS)
   {
      bond.type = BOND_DOUBLE;
      bond.in_ring = true;
      bond.cis_trans = MoleculeCisTrans::CIS;
   }
   else if (code == CMF_BOND_DOUBLE_RING_TRANS)
   {
      bond.type = BOND_DOUBLE;
      bond.in_ring = true;
      bond.cis_trans = MoleculeCisTrans::TRANS;
   }
   else if (code == CMF_BOND_TRIPLE_CHAIN)
   {
      bond.type = BOND_TRIPLE;
      bond.in_ring = false;
   }
   else if (code == CMF_BOND_TRIPLE_RING)
   {
      bond.type = BOND_TRIPLE;
      bond.in_ring = true;
   }
   else if (code == CMF_BOND_AROMATIC)
   {
      bond.type = BOND_AROMATIC;
      bond.in_ring = true;
   }
   else
      throw Error("cannot decode bond: code %d", code);

   while (true)
   {
      if (!_getNextCode(code))
         throw Error("nothing is after the bond code");
   
      if (code >= CMF_BOND_FLAGS && code < CMF_BOND_FLAGS + CMF_NUM_OF_BOND_FLAGS)
         bond.flags |= (1 << (code - CMF_BOND_FLAGS));
      else
         break;
   }
}

bool CmfLoader::_readCycleNumber (int &code, int &n)
{
   n = 0;

   while (code == CMF_CYCLES_PLUS)
   {
      n += CMF_NUM_OF_CYCLES;
      if (!_getNextCode(code))
         throw Error("CYCLES_PLUS symbol must not be the last one");
   }

   if (code >= CMF_CYCLES && code < CMF_CYCLES + CMF_NUM_OF_CYCLES)
   {
      n += code - CMF_CYCLES;
      return true;
   }
   else if (n > 0)
      throw Error("CYCLES_PLUS symbol must be followed by a cycle number");

   return false;
}

void CmfLoader::loadMolecule (Molecule &mol)
{
   int code;

   mol.clear();

   QS_DEF(Array<int>, cycle_numbers);
   QS_DEF(Array<int>, atom_stack);

   _atoms.clear();
   _bonds.clear();
   _pseudo_labels.clear();
   cycle_numbers.clear();
   atom_stack.clear();

   bool first_atom = true;

   if (!_getNextCode(code))
      return;

   /* Main loop */
   do 
   {
      _BondDesc *bond = 0;

      if (code > CMF_ALPHABET_SIZE)
         throw Error("unexpected code");

      if (code == CMF_TERMINATOR)
      {
         break;
      }

      if (!first_atom)
      {
         int number;

         while (_readCycleNumber(code, number))
         {
            while (cycle_numbers.size() <= number)
               cycle_numbers.push(-1);

            if (cycle_numbers[number] >= 0)
               throw Error("cycle #%d already in use", number);
            
            cycle_numbers[number] = atom_stack.top();

            if (!_getNextCode(code))
               break;
         }
      }

      if (code == CMF_SEPARATOR)
      {
         atom_stack.pop();
         first_atom = true;

         if (!_getNextCode(code))
            break;

         continue;
      }

      if (code == CMF_OPEN_BRACKET)
      {
         atom_stack.push(atom_stack.top());

         if (!_getNextCode(code))
            break;

         continue;
      }

      if (code == CMF_CLOSE_BRACKET)
      {
         atom_stack.pop();

         if (!_getNextCode(code))
            break;

         continue;
      }

      if (!first_atom)
      {
         bond = &_bonds.push();
         bond->beg = atom_stack.top();
      }

      if (bond != 0)
      {
         _readBond(code, *bond);

         int number;

         if (_readCycleNumber(code, number))
         {
            if (cycle_numbers[number] < 0)
               throw Error("bad cycle number after bond symbol");

            bond->end = cycle_numbers[number];
            cycle_numbers[number] = -1;

            if (!_getNextCode(code))
               break;

            continue;
         }
      }

      _AtomDesc &atom = _atoms.push();

      if (!first_atom)
         atom_stack.pop();

      atom_stack.push(_atoms.size() - 1);

      first_atom = false;

      if (bond != 0)
         bond->end = _atoms.size() - 1;

      memset(&atom, 0, sizeof(_AtomDesc));
      atom.hydrogens = -1;
      atom.valence = -1;

      if (code > 0 && (code < ELEM_MAX || code == CMF_PSEUDOATOM || code == CMF_RSITE))
      {
         if (!_readAtom(code, atom))
            break;
         continue;
      }

      if (!_getNextCode(code))
         break;

   } while (true); 

   // if have internal decoder, finish it
/*   if (_decoder_obj.get() != 0)
      _decoder_obj->finish(); */

   /* Reading finished, filling molecule */

   int i;

   for (i = 0; i < _atoms.size(); i++)
   {
      mol.addAtom(_atoms[i].label);

      if (_atoms[i].pseudo_atom_idx >= 0)
         mol.setPseudoAtom(i, _pseudo_labels.at(_atoms[i].pseudo_atom_idx));

      if (_atoms[i].rsite_bits > 0)
         mol.setRSiteBits(i, _atoms[i].rsite_bits);

      mol.setAtomCharge(i, _atoms[i].charge);
      mol.setAtomIsotope(i, _atoms[i].isotope);
      if (_atoms[i].hydrogens >= 0)
         mol.setImplicitH(i, _atoms[i].hydrogens);
      mol.setAtomRadical(i, _atoms[i].radical);
   }

   for (i = 0; i < _bonds.size(); i++)
   {
      int type = _bonds[i].type;
      int beg = _bonds[i].beg;
      int end = _bonds[i].end;

      int idx = mol.addBond_Silent(beg, end, type);
      
      if (_bonds[i].in_ring)
         mol.setEdgeTopology(idx, TOPOLOGY_RING);
      else
         mol.setEdgeTopology(idx, TOPOLOGY_CHAIN);
   }

   mol.validateEdgeTopologies();

   if (atom_flags != 0)
   {
      atom_flags->clear();

      for (i = 0; i < _atoms.size(); i++)
         atom_flags->push(_atoms[i].flags);
   }

   if (bond_flags != 0)
   {
      bond_flags->clear();

      for (i = 0; i < _bonds.size(); i++)
         bond_flags->push(_bonds[i].flags);
   }

   if (!skip_cistrans)
   {
      for (i = 0; i < _bonds.size(); i++)
      {
         if (_bonds[i].cis_trans != 0)
         {
            mol.cis_trans.setParity(i, _bonds[i].cis_trans);
            mol.cis_trans.restoreSubstituents(i);
         }
      }
   }

   if (!skip_valence)
   {
      for (i = 0; i < _atoms.size(); i++)
      {
         if (_atoms[i].valence >= 0)
            mol.setValence(i, _atoms[i].valence);
      }
   }

   if (!skip_stereocenters)
   {
      for (i = 0; i < _atoms.size(); i++)
         if (_atoms[i].stereo_type != 0)
            mol.stereocenters.add(i, _atoms[i].stereo_type, _atoms[i].stereo_group, _atoms[i].stereo_invert_pyramid);
   }

   // for loadXyz()
   _mol = &mol;
}

void CmfLoader::loadXyz (Scanner &scanner)
{
   if (_mol == 0)
      throw Error("loadMolecule() must be called prior to loadXyz()");

   int i;
   Vec3f xyz_min, xyz_range;

   xyz_min.x = scanner.readBinaryFloat();
   xyz_min.y = scanner.readBinaryFloat();
   xyz_min.z = scanner.readBinaryFloat();

   xyz_range.x = scanner.readBinaryFloat();
   xyz_range.y = scanner.readBinaryFloat();
   xyz_range.z = scanner.readBinaryFloat();

   bool have_z = (scanner.readByte() != 0);

   for (i = 0; i < _atoms.size(); i++)
   {
      Vec3f pos;

      pos.x = xyz_min.x + ((float)scanner.readBinaryWord() / 65535) * xyz_range.x;
      pos.y = xyz_min.y + ((float)scanner.readBinaryWord() / 65535) * xyz_range.y;

      if (have_z)
         pos.z = xyz_min.z + ((float)scanner.readBinaryWord() / 65535) * xyz_range.z;
      else
         pos.z = 0;

      _mol->setAtomXyz(i, pos.x, pos.y, pos.z);
   }

   _mol->have_xyz = true;
}
