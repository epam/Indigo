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

#include "reaction/crf_loader.h"
#include "base_cpp/scanner.h"
#include "molecule/cmf_loader.h"
#include "reaction/reaction.h"

CrfLoader::CrfLoader (LzwDict &dict, Scanner &scanner) :
_scanner(scanner)
{
   _dict = &dict;
   _decoder.create(dict, scanner);
   xyz_scanner = 0;
}

CrfLoader::CrfLoader (Scanner &scanner) :
_scanner(scanner)
{
   _dict = 0;
   xyz_scanner = 0;
}

void CrfLoader::loadReaction (Reaction &reaction)
{
   int i;
   int nreactants = _scanner.readByte();
   int nproducts = _scanner.readByte();
   byte flags = _scanner.readByte();

   reaction.clear();

   _bond_rc_flags = 0;
   _atom_stereo_flags = 0;
   _aam = 0;

   bool have_aam = (flags != 0);

   for (i = 0; i < nreactants; i++)
   {
      int index = reaction.addReactant();
      _bond_rc_flags = &reaction.getReactingCenterArray(index);
      _atom_stereo_flags = &reaction.getInversionArray(index);
      if (have_aam)
         _aam = &reaction.getAAMArray(index);

      _loadMolecule(reaction.getMolecule(index));
   }

   for (i = 0; i < nproducts; i++)
   {
      int index = reaction.addProduct();
      _bond_rc_flags = &reaction.getReactingCenterArray(index);
      _atom_stereo_flags = &reaction.getInversionArray(index);
      if (have_aam)
         _aam = &reaction.getAAMArray(index);

      _loadMolecule(reaction.getMolecule(index));
   }
}

void CrfLoader::_loadMolecule (Molecule &molecule)
{
   Obj<CmfLoader> loader;
   int i;

   if (_decoder.get() != 0)
      loader.create(_decoder.ref());
   else
      loader.create(_scanner);

   QS_DEF(Array<int>, atom_flags);
   QS_DEF(Array<int>, bond_flags);

   loader->atom_flags = &atom_flags;
   loader->bond_flags = &bond_flags;

   loader->loadMolecule(molecule);

   if (_atom_stereo_flags != 0)
   {
      _atom_stereo_flags->clear_resize(molecule.vertexCount());
      _atom_stereo_flags->zerofill();
      for (i = 0; i < molecule.vertexCount(); i++)
      {
         if (atom_flags[i] & 1)
            _atom_stereo_flags->at(i) |= STEREO_RETAINS;
         if (atom_flags[i] & 2)
            _atom_stereo_flags->at(i) |= STEREO_INVERTS;
      }
   }

   if (_bond_rc_flags != 0)
   {
      _bond_rc_flags->clear_resize(molecule.edgeCount());
      _bond_rc_flags->zerofill();

      for (i = 0; i < molecule.edgeCount(); i++)
      {
         if (bond_flags[i] & 1)
            _bond_rc_flags->at(i) |= RC_UNCHANGED;
         if (bond_flags[i] & 2)
            _bond_rc_flags->at(i) |= RC_MADE_OR_BROKEN;
         if (bond_flags[i] & 4)
            _bond_rc_flags->at(i) |= RC_ORDER_CHANGED;
      }
   }

   if (_aam != 0)
   {
      _aam->clear_resize(molecule.vertexCount());
      _aam->zerofill();
      for (i = 0; i < molecule.vertexCount(); i++)
      {
         int value;

         if (_decoder.get() != 0)
            value = _decoder->get();
         else
            value = _scanner.readByte();

         _aam->at(i) = value - 1;
      }
   }

   if (xyz_scanner != 0)
      loader->loadXyz(*xyz_scanner);
}
