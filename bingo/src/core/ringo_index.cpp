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

#include "core/ringo_index.h"
#include "core/mango_index.h"
#include "core/bingo_context.h"
#include "reaction/reaction_fingerprint.h"
#include "reaction/reaction_automapper.h"
#include "reaction/crf_saver.h"
#include "reaction/reaction_auto_loader.h"
#include "base_cpp/output.h"
#include "reaction/reaction.h"

RingoIndex::RingoIndex (BingoContext &context) :
_context(context),
TL_CP_GET(_fp),
TL_CP_GET(_crf)
{
}

void RingoIndex::checkForConsistency (Reaction &rxn)
{
   int i;

   for (i = rxn.begin(); i != rxn.end(); i = rxn.next(i))
      MangoIndex::checkForConsistency(rxn.getMolecule(i));
}

void RingoIndex::prepare (Scanner &rxnfile, Output &output)
{
   QS_DEF(Reaction, reaction);
      
   ReactionAutoLoader rrd(rxnfile);
   rrd.treat_x_as_pseudoatom = _context.treat_x_as_pseudoatom;
   rrd.ignore_closing_bond_direction_mismatch =
           _context.ignore_closing_bond_direction_mismatch;
   rrd.loadReaction(reaction);

   checkForConsistency(reaction);

   ReactionAutomapper ram(reaction);
   ram.correctReactingCenters(true);

   reaction.aromatize();

   ReactionFingerprintBuilder builder(reaction, _context.fp_parameters);

   builder.process();
   _fp.copy(builder.getFingerprint(), _context.fp_parameters.fingerprintSizeExtOrd() * 2);

   ArrayOutput output_crf(_crf);

   CrfSaver saver(_context.cmf_dict, output_crf);

   saver.saveReaction(reaction);

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
