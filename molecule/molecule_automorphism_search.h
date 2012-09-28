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

#ifndef __molecule_automorphism_search__
#define __molecule_automorphism_search__

#include "graph/automorphism_search.h"

namespace indigo {

class Molecule;
class MoleculeStereocenters;
class CancellationHandler;

class MoleculeAutomorphismSearch : public AutomorphismSearch
{
public:

   MoleculeAutomorphismSearch ();
   virtual ~MoleculeAutomorphismSearch () {}
   
   void process (Molecule &mol);

   // By default all stereocenters and cis-trans bonds are treated as valid.
   bool detect_invalid_stereocenters;
   bool detect_invalid_cistrans_bonds;
   bool find_canonical_ordering;

   // By default all the atoms with undefinded number of hydrogens throws an exception.
   // If allow_undefined is true then such atoms are simply uncomparable and some 
   // symmetries might be lost.
   bool allow_undefined;

   bool invalidCisTransBond (int idx);
   bool invalidStereocenter (int idx);

   // Bonds indices that will be checked for possible cis-trans property.
   // After calling "process" method all bonds, that cannot be cis-trans
   // will be removed from this array.
   Array<int> possible_cis_trans_to_check;

   DECL_ERROR;
   DECL_TIMEOUT_EXCEPTION;

protected:
   static int  _vertex_cmp  (Graph &graph, int v1, int v2, const void *context);
   static int  _edge_rank    (Graph &graph, int edge_idx, const void *context);
   static bool _check_automorphism (Graph &graph, const Array<int> &mapping, const void *context);

   static bool _isCisTransBondMappedRigid (Molecule &mol, int i, const int *mapping);
   static bool _isStereocenterMappedRigid (const MoleculeStereocenters &stereocenters, int i, const int *mapping);

   static int  _compare_mapped (Graph &graph, const Array<int> &mapping1, const Array<int> &mapping2, const void *context);

   static void _automorphismCallback (const int *automorphism, void *context);

   static int _compareRSites (Molecule &mol, int v1, int v2, const void *context);
   static int _compareStereo (Molecule &mol, int v1, int v2, const void *context);

   int _compareMappedStereocenters (Molecule &mol, const Array<int> &mapping1, 
      const Array<int> &mapping2, const Array<int> &inv_mapping1, const Array<int> &inv_mapping2) const;

   bool _checkStereocentersAutomorphism (Molecule &mol, const Array<int> &mapping) const;

   void _initialize                  (Molecule &mol);
   void _calculateHydrogensAndDegree (Molecule &mol);
   void _getFirstApproximation       (Molecule &mol);

   int _validCisTransBond (int idx, const Array<int> &orbits);
   int  _validStereocenter (int idx, Array<int> &orbits, int *parity = 0);
   int  _validStereocenterByAtom (int atom_idx, Array<int> &orbits, int *parity = 0);

   int _treat_undef_as;
   int _getStereo (int state) const;

   bool _findInvalidStereo (Molecule &mol);
   bool _findInvalidStereoCisTrans (Molecule &mol);
   void _markValidOrInvalidStereo (bool find_valid, Array<int> &approximation_orbits, bool *found);
   void _findCisTransStereoBondParirties (Molecule &mol);

   bool _hasStereo (Molecule &mol);

   void _markComplicatedStereocentersAsValid (Molecule &mol);

   bool _checkCisTransInvalid (Molecule &mol, int bond_idx);
   void _findAllPossibleCisTrans (Molecule &mol);
   void _findAllPossibleCisTransOneStep (Molecule &mol);

   struct EdgeInfo
   {
      int mapped_vertex;
      int edge;
   };

   static void _getSortedNei (Graph &g, int v, Array<EdgeInfo> &sorted_nei, Array<int>& inv_mapping);
   int _getMappedBondOrderAndParity (Molecule &m, int e, Array<int>& inv_mapping) const;

   TL_CP_DECL(Array<int>, _approximation_orbits);
   TL_CP_DECL(Array<int>, _approximation_orbits_saved);
   TL_CP_DECL(Array<int>, _hcount);
   TL_CP_DECL(Array<int>, _cistrans_stereo_bond_parity);
   TL_CP_DECL(Array<int>, _degree);
   TL_CP_DECL(Array<int>, _independent_component_index);

   enum { _NO_STEREO = -1, _INVALID, _VALID, _UNDEF };
   TL_CP_DECL(Array<int>, _stereocenter_state);
   TL_CP_DECL(Array<int>, _cistrans_bond_state);

   // Target stereocenters and cis-trans bond for checking permutation parity
   int _target_stereocenter, _target_bond;
   bool _target_stereocenter_parity_inv, _target_bond_parity_inv;
   int _fixed_atom;

   CancellationHandler *_cancellation_handler;
};

}

#endif
