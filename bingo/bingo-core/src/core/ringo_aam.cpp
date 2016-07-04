/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
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

#include "core/ringo_matchers.h"
#include "core/bingo_context.h"
#include "reaction/reaction_auto_loader.h"
#include "reaction/rxnfile_saver.h"
#include "reaction/reaction_automapper.h"
#include "layout/reaction_layout.h"

IMPL_ERROR(RingoAAM, "ringo AAM");

RingoAAM::RingoAAM(BingoContext &context) : _context(context) {
}

void RingoAAM::loadReaction (const Array<char> &buf) {
   BufferScanner scanner(buf);
   loadReaction(scanner);
}

void RingoAAM::loadReaction (const char *str) {
   BufferScanner scanner(str);
   loadReaction(scanner);
}

void RingoAAM::loadReaction (Scanner &scanner) {
   ReactionAutoLoader rxd(scanner);
   _context.setLoaderSettings(rxd);
   rxd.loadReaction(_reaction);
}

void RingoAAM::parse(const char *mode) {
   if (strcasecmp(mode, "CLEAR") == 0){
      _reaction.clearAAM();
      return;
   }
   ReactionAutomapper ram(_reaction);
   if (strcasecmp(mode, "DISCARD") == 0){
      ram.automap(ReactionAutomapper::AAM_REGEN_DISCARD);
   } else if (strcasecmp(mode, "ALTER") == 0){
      ram.automap(ReactionAutomapper::AAM_REGEN_ALTER);
   } else if (strcasecmp(mode, "KEEP") == 0){
      ram.automap(ReactionAutomapper::AAM_REGEN_KEEP);
   }
   else
      throw Error("unknown mode: %s", mode);
}

void RingoAAM::getResult(Array<char> &buf) {
   ArrayOutput output_r(buf);
   RxnfileSaver rcs(output_r);

   if (!Reaction::haveCoord(_reaction)){
      ReactionLayout layout(_reaction);

      layout.make();
      _reaction.markStereocenterBonds();
   }

   rcs.saveReaction(_reaction);
}
