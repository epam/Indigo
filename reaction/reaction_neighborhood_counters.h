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

#ifndef __reaction_neighborhood_counters_h__
#define __reaction_neighborhood_counters_h__

#include "base_cpp/obj_array.h"
#include "molecule/molecule_neighbourhood_counters.h"

namespace indigo {

class BaseReaction;
class Reaction;
class QueryReaction;
class MoleculeAtomNeighbourhoodCounters;

class ReactionAtomNeighbourhoodCounters
{
public:
   void calculate (Reaction &reac);
   void calculate (QueryReaction &reac);

   const MoleculeAtomNeighbourhoodCounters & getCounters (int idx) const;

private:
   ObjArray<MoleculeAtomNeighbourhoodCounters> _counters;
};

}

#endif
