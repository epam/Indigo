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

#include "reaction/reaction_neighborhood_counters.h"
#include "reaction/reaction.h"
#include "reaction/query_reaction.h"
#include "molecule/molecule_neighbourhood_counters.h"

using namespace indigo;

void ReactionAtomNeighbourhoodCounters::calculate(Reaction &reac) {
   int i;

   _counters.resize(reac.count());

   for (i = reac.begin(); i < reac.end(); i = reac.next(i))
      _counters[i].calculate(reac.getMolecule(i));
}

void ReactionAtomNeighbourhoodCounters::calculate(QueryReaction &reac) {
   int i;

   _counters.resize(reac.count());

   for (i = reac.begin(); i < reac.end(); i = reac.next(i))
      _counters[i].calculate(reac.getQueryMolecule(i));
}

const MoleculeAtomNeighbourhoodCounters & ReactionAtomNeighbourhoodCounters::getCounters (int idx) const
{
   return _counters[idx];
}
