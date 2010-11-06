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

#ifndef __smiles_loader__
#define __smiles_loader__

#include "base_cpp/exception.h"
#include "molecule/molecule.h"
#include "base_cpp/tlscont.h"
#include "molecule/query_molecule.h"

namespace indigo {

class Scanner;
class BaseMolecule;
class Molecule;
class QueryMolecule;
class GraphHighlighting;

class SmilesLoader
{
public:
   DEF_ERROR("SMILES loader");

   DLLEXPORT SmilesLoader (Scanner &scanner);

   DLLEXPORT void loadMolecule      (Molecule &mol);
   DLLEXPORT void loadQueryMolecule (QueryMolecule &mol);

   DLLEXPORT void loadSMARTS (QueryMolecule &mol);

   DLLEXPORT void checkQueryAtoms ();

   Array<int> * reaction_atom_mapping;

   GraphHighlighting * highlighting;

   bool inside_rsmiles;

   bool smarts_mode;

   // set to true to accept buggy SMILES like 'N/C=C\1CCCN[C@H]\1S'
   // (see http://groups.google.com/group/indigo-bugs/browse_thread/thread/de7da07a3a5cb3ee
   //  for details)
   bool ignore_closing_bond_direction_mismatch;

protected:

   enum
   {
      _STAR_ATOM = 1,
      _ANY_BOND = -2
   };

   class _AtomDesc
   {
   public:
      _AtomDesc (Pool<List<int>::Elem> &neipool);
      ~_AtomDesc ();

      void pending (int cycle);
      void closure (int cycle, int end);

      List<int> neighbors;
      int parent;

      int label;
      int isotope;
      int charge;
      int hydrogens;
      int chirality;
      int aromatic;
      int aam;
      bool brackets;

      int query_type;
   };

   struct _BondDesc
   {
      int beg;
      int end;
      int type;
      int dir; // 0 -- undirected; 1 -- goes 'up' from beg to end, 2 -- goes 'down'
      int topology;
   };

   struct _CycleDesc
   {
      void clear ()
      {
         beg = -1;
         pending_bond = -1;
      }

      int beg;
      int pending_bond;
   };

   Scanner &_scanner;

   TL_CP_DECL(Pool<List<int>::Elem>, _neipool);
   TL_CP_DECL(ObjArray<_AtomDesc>, _atoms);
   TL_CP_DECL(Array<_BondDesc>, _bonds);

   int  _balance;
   int  _current_compno;
   bool _inside_smarts_component;

   BaseMolecule  *_bmol;
   QueryMolecule *_qmol;
   Molecule      *_mol;

   void _loadMolecule ();

   void _calcStereocenters ();
   void _calcCisTrans ();
   void _readOtherStuff ();

   void _readAtom (Array<char> &atom_str, bool first_in_brackets,
                   _AtomDesc &atom, AutoPtr<QueryMolecule::Atom> &qatom);

   void _readBond (Array<char> &bond_str, _BondDesc &bond,
                   AutoPtr<QueryMolecule::Bond> &qbond, bool smarts_mode);

private:
   SmilesLoader (const SmilesLoader &); // no implicit copy
};

}

#endif
