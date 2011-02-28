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

#ifndef __ringo_matchers__
#define __ringo_matchers__

#include "molecule/molecule.h"
#include "base_cpp/tlscont.h"
#include "reaction/reaction.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction_neighborhood_counters.h"

using namespace indigo;

namespace indigo
{
   class Scanner;
}

class BingoContext;

class RingoSubstructure
{
public:

   explicit RingoSubstructure (BingoContext &context);
   virtual ~RingoSubstructure ();

   bool parse (const char *params);

   bool matchBinary (const Array<char> &buf);
   bool matchBinary (Scanner &scanner);

   void loadTarget (const Array<char> &buf);
   void loadTarget (Scanner &scanner);
   void loadTarget (const char *str);

   bool matchLoadedTarget ();

   void loadQuery (const Array<char> &buf);
   void loadQuery (const char *str);
   void loadQuery (Scanner &scanner);

   bool preserve_bonds_on_highlighting;
   bool treat_x_as_pseudoatom;
   bool ignore_closing_bond_direction_mismatch;
   
   void getHighlightedTarget (Array<char> &buf);

   const byte * getQueryFingerprint ();

   DEF_ERROR("reaction substructure");
protected:

   BingoContext &_context;

   QueryReaction _query_reaction;
   Reaction _target_reaction;

   ReactionAtomNeighbourhoodCounters _nei_target_counters;
   ReactionAtomNeighbourhoodCounters _nei_query_counters;

   ObjArray< Array<int> > _target_bond_types;

   Array<byte> _query_fp;
   bool        _query_data_valid;

   void _validateQueryData ();

   void _initQuery  (QueryReaction &query_in, QueryReaction &query_out);
   void _initTarget (bool from_database);

};

class RingoAAM
{

public:
   RingoAAM ();

   void parse (const char* mode);

   void loadReaction (const Array<char> &buf);
   void loadReaction (const char *str);
   void loadReaction (Scanner &scanner);

   void getResult (Array<char> &buf);

   bool treat_x_as_pseudoatom;
   bool ignore_closing_bond_direction_mismatch;

   DEF_ERROR("ringo AAM");

protected:
   Reaction _reaction;
};

#endif
