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

#ifndef __molecule_arom_match_h__
#define __molecule_arom_match_h__

#include "base_cpp/tlscont.h"
#include "base_cpp/array.h"
#include "base_cpp/exception.h"
#include "molecule/molecule.h"

namespace indigo {

class Molecule;
class QueryMolecule;

/*
 * Aromaticity matcher handles cases of the query possible aromatic cycles.
 * Some cycles in the query molecule can be aromatic or not depending on the 
 * embedding to the target molecule. For example [#7]C1=C~[#6]~[#6]~C=C1O 
 * query molecule should match both NC1=CCCC=C1O and Nc1ccccc1O target 
 * molecules. Only aromatic bonds can match aromatic bonds, so to match 
 * aromatic bond query must have aromatic realization (assignment exact values 
 * to the any bond and any atoms that leads cycle to be aromatic). 
 */
class AromaticityMatcher
{
public:
   AromaticityMatcher (QueryMolecule &query, BaseMolecule &base, const AromaticityOptions &arom_options);

   // Check if aromaticity matcher is necessary for specified query
   static bool isNecessary (QueryMolecule &query);

   // Update internal structures when query molecule changes (grow)
   void validateQuery ();

   // Check if query bond can be aromatic if 'aromatic' is true and 
   // nonaromatic otherwise.
   bool canFixQueryBond (int query_edge_idx, bool aromatic);

   // Fix query bond to aromatic or nonaromatic state
   void fixQueryBond (int query_edge_idx, bool aromatic);

   // Unfix query bond (opposite to fixQueryBond)
   void unfixQueryBond (int query_edge_idx);

   // Unfix all neighbour bonds
   void unfixNeighbourQueryBond (int query_arom_idx);

   // Check if embedding is possible. 'core_sub' corresponds 
   // to the mapping from the query to the target. Vertices 
   // with negative values are ignored. 'core_super' is 
   // an inverse mapping for 'core_sub'.
   bool match (int *core_sub, int *core_super);

   DECL_ERROR;
protected:
   QueryMolecule &_query;
   BaseMolecule &_base;

   AromaticityOptions _arom_options;

   enum {
      ANY = 0, AROMATIC, NONAROMATIC
   };
   TL_CP_DECL(Array<int>, _matching_edges_state);
   AutoPtr<BaseMolecule> _submolecule;
};

}

#endif
