/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#ifndef __reaction_neighborhood_counters_h__
#define __reaction_neighborhood_counters_h__

#include "base_cpp/obj_array.h"
#include "molecule/molecule_neighbourhood_counters.h"

namespace indigo
{

    class BaseReaction;
    class Reaction;
    class QueryReaction;
    class MoleculeAtomNeighbourhoodCounters;

    class ReactionAtomNeighbourhoodCounters
    {
    public:
        void calculate(Reaction& reac);
        void calculate(QueryReaction& reac);

        const MoleculeAtomNeighbourhoodCounters& getCounters(int idx) const;

    private:
        ObjArray<MoleculeAtomNeighbourhoodCounters> _counters;
    };

} // namespace indigo

#endif
