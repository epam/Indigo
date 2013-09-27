/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
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

#include "core/ringo_index.h"

#include "base_cpp/output.h"
#include "base_cpp/os_sync_wrapper.h"
#include "core/mango_index.h"
#include "core/bingo_context.h"
#include "reaction/reaction_fingerprint.h"
#include "reaction/reaction_automapper.h"
#include "reaction/crf_saver.h"
#include "reaction/reaction_auto_loader.h"
#include "reaction/reaction.h"
#include "core/ringo_matchers.h"

void RingoIndex::prepare (Scanner &rxnfile, Output &output, OsLock *lock_for_exclusive_access)
{
   QS_DEF(Reaction, reaction);
      
   ReactionAutoLoader rrd(rxnfile);
   rrd.treat_x_as_pseudoatom = _context->treat_x_as_pseudoatom;
   rrd.ignore_closing_bond_direction_mismatch =
           _context->ignore_closing_bond_direction_mismatch;
   rrd.loadReaction(reaction);

   // Skip all SGroups
   for (int mol_idx = reaction.begin(); mol_idx != reaction.end(); mol_idx = reaction.next(mol_idx)) 
      reaction.getBaseMolecule(mol_idx).clearSGroups();

   Reaction::checkForConsistency(reaction);

   ReactionAutomapper ram(reaction);
   ram.correctReactingCenters(true);

   reaction.aromatize(AromaticityOptions::BASIC);

   _hash = RingoExact::calculateHash(reaction);
   {
      ArrayOutput out(_hash_str);
      out.printf("%02X", _hash);
      _hash_str.push(0);
   }

   if (!skip_calculate_fp)
   {
      ReactionFingerprintBuilder builder(reaction, _context->fp_parameters);

      builder.process();
      _fp.copy(builder.get(), _context->fp_parameters.fingerprintSizeExtOrdSim() * 2);
   }

   ArrayOutput output_crf(_crf);
   {
      // CrfSaver modifies _context->cmf_dict and 
      // requires exclusive access for this
      OsLockerNullable locker(lock_for_exclusive_access);
      CrfSaver saver(_context->cmf_dict, output_crf);
      saver.saveReaction(reaction);
   }

   output.writeArray(_crf);
}

const byte * RingoIndex::getFingerprint ()
{
   return _fp.ptr();
}

const Array<char> & RingoIndex::getCrf ()
{
   return _crf;
}

dword RingoIndex::getHash ()
{
   return _hash;
}

const char * RingoIndex::getHashStr ()
{
   return _hash_str.ptr();
}

void RingoIndex::clear ()
{
   _fp.clear();
   _crf.clear();
}
