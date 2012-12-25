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

#include "reaction/reaction_fingerprint.h"
#include "molecule/molecule_fingerprint.h"
#include "reaction/base_reaction.h"
#include "base_c/bitarray.h"

using namespace indigo;

IMPL_ERROR(ReactionFingerprintBuilder, "fingerprint builder");

ReactionFingerprintBuilder::ReactionFingerprintBuilder (BaseReaction &reaction,
        const MoleculeFingerprintParameters &parameters) :
_reaction(reaction),
_parameters(parameters),
TL_CP_GET(_fingerprint)
{
   query = false;
   skip_sim = false;
   skip_ord = false;
   skip_ext = false;
}

void ReactionFingerprintBuilder::process ()
{
   int i, one_fp_size = _parameters.fingerprintSizeExtOrdSim();
   
   _fingerprint.clear_resize(one_fp_size * 2);
   _fingerprint.zerofill();
           
   for (i = _reaction.reactantBegin(); i < _reaction.reactantEnd(); i = _reaction.reactantNext(i))
   {
      MoleculeFingerprintBuilder builder(_reaction.getBaseMolecule(i), _parameters);

      builder.query = query;
      builder.skip_tau = true;
      builder.skip_sim = skip_sim;
      builder.skip_ord = skip_ord;
      builder.skip_ext = skip_ext;
      builder.skip_any_atoms = true;
      builder.skip_any_bonds = true;
      builder.skip_any_atoms_bonds = true;
      builder.process();
      bitOr(get(), builder.get(), _parameters.fingerprintSizeExtOrd());
      bitOr(getSim(), builder.getSim(), _parameters.fingerprintSizeSim());
   }
   for (i = _reaction.productBegin(); i < _reaction.productEnd(); i = _reaction.productNext(i))
   {
      MoleculeFingerprintBuilder builder(_reaction.getBaseMolecule(i), _parameters);

      builder.query = query;
      builder.skip_tau = true;
      builder.skip_sim = skip_sim;
      builder.skip_ord = skip_ord;
      builder.skip_ext = skip_ext;
      builder.skip_any_atoms = true;
      builder.skip_any_bonds = true;
      builder.skip_any_atoms_bonds = true;
      builder.process();
      bitOr(get() + _parameters.fingerprintSizeExtOrd(),
            builder.get(), _parameters.fingerprintSizeExtOrd());
      bitOr(getSim() + + _parameters.fingerprintSizeSim(),
            builder.getSim(), _parameters.fingerprintSizeSim());
   }
}

byte * ReactionFingerprintBuilder::get ()
{
   return _fingerprint.ptr();
}

byte * ReactionFingerprintBuilder::getSim ()
{
   return _fingerprint.ptr() + _parameters.fingerprintSizeExtOrd() * 2;
}

void ReactionFingerprintBuilder::parseFingerprintType(const char *type, bool query) {
   this->query = query;

   if (type == 0 || *type == 0 || strcasecmp(type, "sim") == 0)
   {
      // similarity
      this->skip_ext = true;
      this->skip_ord = true;
   }
   else if (strcasecmp(type, "sub") == 0)
      // substructure
      this->skip_sim = true;
   else if (strcasecmp(type, "full") == 0)
   {
      if (query)
         throw Error("there can not be 'full' fingerprint of a query reaction");
      // full (non-query) fingerprint, do not skip anything
   }
   else
      throw Error("unknown molecule fingerprint type: %s", type);
}
