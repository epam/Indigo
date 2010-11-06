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

#include "reaction/reaction_fingerprint.h"
#include "molecule/molecule_fingerprint.h"
#include "reaction/base_reaction.h"
#include "base_c/bitarray.h"

using namespace indigo;

ReactionFingerprintBuilder::ReactionFingerprintBuilder (BaseReaction &reaction,
        const MoleculeFingerprintParameters &parameters) :
_reaction(reaction),
_parameters(parameters),
TL_CP_GET(_fingerprint)
{
   query = false;
}

void ReactionFingerprintBuilder::process ()
{
   int i, one_fp_size = _parameters.fingerprintSizeExtOrd();
   
   _fingerprint.clear_resize(one_fp_size * 2);
   _fingerprint.zerofill();
           
   for (i = _reaction.reactantBegin(); i < _reaction.reactantEnd(); i = _reaction.reactantNext(i))
   {
      MoleculeFingerprintBuilder builder(_reaction.getBaseMolecule(i), _parameters);

      builder.query = query;
      builder.skip_tau = true;
      builder.skip_sim = true;
      builder.process();
      bitOr(_fingerprint.ptr(), builder.get(), one_fp_size);
   }
   for (i = _reaction.productBegin(); i < _reaction.productEnd(); i = _reaction.productNext(i))
   {
      MoleculeFingerprintBuilder builder(_reaction.getBaseMolecule(i), _parameters);

      builder.query = query;
      builder.skip_tau = true;
      builder.skip_sim = true;
      builder.process();
      bitOr(_fingerprint.ptr() + one_fp_size, builder.get(), one_fp_size);
   }
}

const byte * ReactionFingerprintBuilder::getFingerprint ()
{
   return _fingerprint.ptr();
}
