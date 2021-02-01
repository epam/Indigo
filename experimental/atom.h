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

#ifndef __atom_h__
#define __atom_h__

#include "item.h"
#include "math/algebra.h"
#include <map>
#include <memory>
#include <stdint.h>
#include <vadefs.h>

namespace indigo2
{

    class ChemElem
    {
        static const ChemElem periodicTable[104];
        string symbol;
        string name;
        int isotopes[50];
        /* See Indigo\molecule\elements.h*/
    };

    typedef Atom shared_ptr<_Atom>;
    class _Atom : public _Vertex
    {
        const ChemElem& element;
        bool explicit_valence;
        bool explicit_impl_h;
        int isotope;
        int charge;
        int pseudoatom_value_idx; // if number == ELEM_PSEUDO, this is the corresponding
                                  // index from _pseudo_atom_values
        int rgroup_bits;          // if number == ELEM_RSITE, these are 32 bits, each allowing
                                  // an r-group with corresponding number to go for this atom.
                                  // Simple 'R' atoms have this field equal to zero.
        int template_occur_idx;   // if number == ELEM_TEMPLATE, this is the corresponding
                                  // index from _template_occurrences
    }


    typedef QAtomList shared_ptr<_QAtomList>;
    class QAtomList : public _Vertex
    {
        std::vector<Atom> list;
        bool not ;
    }

} // namespace indigo2
#endif
