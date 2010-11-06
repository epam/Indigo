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

#include "reaction/reaction.h"
#include "reaction/crf_saver.h"
#include "lzw/lzw_encoder.h"
#include "molecule/cmf_saver.h"
#include "base_cpp/output.h"
#include "molecule/cmf_symbol_codes.h"

using namespace indigo;

CrfSaver::CrfSaver (LzwDict &dict, Output &output) : _output(output)
{
   if (!dict.isInitialized())
      dict.init(CMF_ALPHABET_SIZE, CMF_BIT_CODE_SIZE);
   
   _encoder.create(dict, output);
   xyz_output = 0;
   skip_implicit_h = false;
}

CrfSaver::CrfSaver (Output &output) : _output(output)
{
   xyz_output = 0;
   skip_implicit_h = false;
}

void CrfSaver::saveReaction (Reaction &reaction)
{
   _writeReactionInfo(reaction);

   int i;

   _atom_stereo_flags = 0;
   _bond_rc_flags = 0;
   _aam = 0;

   for (i = reaction.reactantBegin(); i < reaction.reactantEnd(); i = reaction.reactantNext(i))
   {
      _atom_stereo_flags = reaction.getInversionArray(i).ptr();
      _bond_rc_flags = reaction.getReactingCenterArray(i).ptr();
      _aam = reaction.getAAMArray(i).ptr();
      _writeMolecule(reaction.getMolecule(i));
   }

   for (i = reaction.productBegin(); i < reaction.productEnd(); i = reaction.productNext(i))
   {
      _atom_stereo_flags = reaction.getInversionArray(i).ptr();
      _bond_rc_flags = reaction.getReactingCenterArray(i).ptr();
      _aam = reaction.getAAMArray(i).ptr();
      _writeMolecule(reaction.getMolecule(i));
   }

   if (_encoder.get() != 0)
      _encoder->finish();
}

void CrfSaver::_writeMolecule (Molecule &molecule)
{
   Obj<CmfSaver> saver;
   int i;

   if (_encoder.get() != 0)
      saver.create(_encoder.ref());
   else
      saver.create(_output);

   QS_DEF(Array<int>, atom_flags);
   QS_DEF(Array<int>, bond_flags);

   if (_atom_stereo_flags != 0)
   {
      atom_flags.clear_resize(molecule.vertexEnd());
      atom_flags.zerofill();

      for (i = molecule.vertexBegin(); i != molecule.vertexEnd(); i = molecule.vertexNext(i))
         if (_atom_stereo_flags[i] == STEREO_RETAINS)
            atom_flags[i] = 1;
         else if (_atom_stereo_flags[i] == STEREO_INVERTS)
            atom_flags[i] = 2;
      saver->atom_flags = atom_flags.ptr();
   }

   if (_bond_rc_flags != 0)
   {
      bond_flags.clear_resize(molecule.edgeEnd());
      bond_flags.zerofill();

      for (i = molecule.edgeBegin(); i != molecule.edgeEnd(); i = molecule.edgeNext(i))
      {
         if (_bond_rc_flags[i] & RC_UNCHANGED)
            bond_flags[i] |= 1;
         if (_bond_rc_flags[i] & RC_MADE_OR_BROKEN)
            bond_flags[i] |= 2;
         if (_bond_rc_flags[i] & RC_ORDER_CHANGED)
            bond_flags[i] |= 4;
      }
      saver->bond_flags = bond_flags.ptr();
   }

   saver->skip_implicit_h = skip_implicit_h;
   saver->saveMolecule(molecule);

   if (_aam != 0)
      _writeAam(_aam, saver->getAtomSequence());

   if (xyz_output != 0)
   {
      if (xyz_output == &_output && _encoder.get() != 0)
         _encoder->finish();

      saver->saveXyz(*xyz_output);

      if (xyz_output == &_output && _encoder.get() != 0)
         _encoder->start();
   }
}

void CrfSaver::_writeReactionInfo (Reaction &reaction)
{
   _output.writeByte(reaction.reactantsCount());
   _output.writeByte(reaction.productsCount());

   byte have_aam = 1;

   _output.writeByte(have_aam);
}

void CrfSaver::_writeAam (const int *aam, const Array<int> &sequence)
{
   int i;

   for (i = 0; i < sequence.size(); i++)
   {
      int value = aam[sequence[i]] + 1;

      if (value < 1 || value >= CMF_ALPHABET_SIZE)
         throw Error("bad AAM value: %d", value);

      if (_encoder.get() != 0)
         _encoder->send(value);
      else
         _output.writeByte(value);
   }
}
