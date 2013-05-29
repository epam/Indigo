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

#include "core/ringo_matchers.h"
#include "core/bingo_context.h"
#include "base_cpp/scanner.h"
#include "base_cpp/profiling.h"
#include "reaction/rxnfile_loader.h"
#include "reaction/rxnfile_saver.h"
#include "reaction/reaction_substructure_matcher.h"
#include "reaction/reaction_automapper.h"
#include "molecule/molecule_fingerprint.h"
#include "reaction/reaction_fingerprint.h"
#include "reaction/crf_loader.h"
#include "base_cpp/output.h"
#include "reaction/reaction_auto_loader.h"
#include "layout/reaction_layout.h"
#include "reaction/rsmiles_loader.h"

IMPL_ERROR(RingoSubstructure, "reaction substructure");

RingoSubstructure::RingoSubstructure (BingoContext &context) :
_context(context)
{
   preserve_bonds_on_highlighting = false;
   treat_x_as_pseudoatom = false;
   ignore_closing_bond_direction_mismatch = false;
   _smarts = false;
}

RingoSubstructure::~RingoSubstructure ()
{
}

bool RingoSubstructure::parse (const char *params)
{
   preserve_bonds_on_highlighting = false;
   return true;
}

void RingoSubstructure::loadQuery (const Array<char> &buf)
{
   BufferScanner scanner(buf);

   loadQuery(scanner);
}

void RingoSubstructure::loadQuery (const char *str)
{
   BufferScanner scanner(str);

   loadQuery(scanner);
}

void RingoSubstructure::loadQuery (Scanner &scanner)
{
   QS_DEF(QueryReaction, source);

   ReactionAutoLoader loader(scanner);

   loader.treat_x_as_pseudoatom = treat_x_as_pseudoatom;
   loader.ignore_closing_bond_direction_mismatch =
           ignore_closing_bond_direction_mismatch;
   loader.loadQueryReaction(source);

   _initQuery(source, _query_reaction);
   _query_data_valid = false;
   _smarts = false;
}

void RingoSubstructure::loadSMARTS (Scanner &scanner)
{
   RSmilesLoader loader(scanner);
   QS_DEF(QueryReaction, source);

   loader.smarts_mode = true;
   loader.loadQueryReaction(source);

   _initSmartsQuery(source, _query_reaction);
   _query_data_valid = false;
   _smarts = true;
}

void RingoSubstructure::loadSMARTS (const Array<char> &buf)
{
   BufferScanner scanner(buf);

   loadSMARTS(scanner);
}

void RingoSubstructure::loadSMARTS (const char *str)
{
   BufferScanner scanner(str);

   loadSMARTS(scanner);
}

void RingoSubstructure::_initQuery (QueryReaction &query_in, QueryReaction &query_out)
{
   query_out.makeTransposedForSubstructure(query_in);

   ReactionAutomapper ram(query_out);
   ram.correctReactingCenters(true);

   query_out.aromatize(AromaticityOptions::BASIC);

   _nei_query_counters.calculate(query_out);
} 

void RingoSubstructure::_initSmartsQuery (QueryReaction &query_in, QueryReaction &query_out)
{
   query_out.makeTransposedForSubstructure(query_in);

   ReactionAutomapper ram(query_out);
   ram.correctReactingCenters(true);

   _nei_query_counters.calculate(query_out);
} 

void RingoSubstructure::_validateQueryData ()
{
   if (_query_data_valid)
      return;

   ReactionFingerprintBuilder builder(_query_reaction, _context.fp_parameters);
   
   builder.query = true;
   builder.skip_sim = true;
   builder.process();
   
   _query_fp.copy(builder.get(), _context.fp_parameters.fingerprintSizeExtOrdSim() * 2);
   
   _query_data_valid = true;
}

void RingoSubstructure::_initTarget (bool from_database)
{
   if (preserve_bonds_on_highlighting)
      Reaction::saveBondOrders(_target_reaction, _target_bond_types);

   if (!from_database)
   {
      ReactionAutomapper ram(_target_reaction);
      ram.correctReactingCenters(true);
      _target_reaction.aromatize(AromaticityOptions::BASIC);
   }
   _nei_target_counters.calculate(_target_reaction);
} 

bool RingoSubstructure::matchBinary (const Array<char> &buf)
{
   BufferScanner scanner(buf);

   return matchBinary(scanner);
}


bool RingoSubstructure::matchBinary (Scanner &scanner)
{
   CrfLoader loader(_context.cmf_dict, scanner);

   loader.loadReaction(_target_reaction);
   _initTarget(true);

   ReactionSubstructureMatcher rsm(_target_reaction);
   rsm.setQuery(_query_reaction);
   rsm.highlight = true;
   if (_smarts)
      rsm.use_daylight_aam_mode = true;
   //rsm.setNeiCounters(&_nei_query_counters, &_nei_target_counters);

   return rsm.find();
}

void RingoSubstructure::loadTarget (const Array<char> &buf)
{
   BufferScanner scanner(buf);

   return loadTarget(scanner);
}

void RingoSubstructure::loadTarget (const char *str)
{
   BufferScanner scanner(str);

   return loadTarget(scanner);
}

void RingoSubstructure::loadTarget (Scanner &scanner)
{
   ReactionAutoLoader loader(scanner);

   loader.treat_x_as_pseudoatom = treat_x_as_pseudoatom;
   loader.ignore_closing_bond_direction_mismatch =
           ignore_closing_bond_direction_mismatch;
   loader.loadReaction(_target_reaction);
   _initTarget(false);
}

bool RingoSubstructure::matchLoadedTarget ()
{
   ReactionSubstructureMatcher rsm(_target_reaction);

   rsm.highlight = true;

   rsm.setQuery(_query_reaction);
   //rsm.setNeiCounters(&_nei_query_counters, &_nei_target_counters);
   if (_smarts)
      rsm.use_daylight_aam_mode = true;

   return rsm.find();
}

void RingoSubstructure::getHighlightedTarget (Array<char> &buf)
{
   ArrayOutput output(buf);
   RxnfileSaver saver(output);

   if (!Reaction::haveCoord(_target_reaction))
   {
      profTimerStart(t, "match.layout");
      ReactionLayout layout(_target_reaction);

      layout.make();
      _target_reaction.markStereocenterBonds();
   }

   if (preserve_bonds_on_highlighting)
      Reaction::loadBondOrders(_target_reaction, _target_bond_types);

   saver.saveReaction(_target_reaction);
}

const byte * RingoSubstructure::getQueryFingerprint ()
{
   _validateQueryData();
   return _query_fp.ptr();
}
