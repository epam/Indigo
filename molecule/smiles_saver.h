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

#ifndef __smiles_saver__
#define __smiles_saver__

#include "base_cpp/exception.h"
#include "base_cpp/array.h"
#include "base_cpp/tlscont.h"
#include "molecule/query_molecule.h"
#include "base_cpp/list.h"
#include "base_cpp/obj_array.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class Output;
class BaseMolecule;
class QueryMolecule;
class Molecule;
class GraphHighlighting;

class DLLEXPORT SmilesSaver
{
public:
   DEF_ERROR("SMILES saver");

   SmilesSaver (Output &output);
   ~SmilesSaver ();

   void saveMolecule (Molecule &mol);
   void saveQueryMolecule (QueryMolecule &mol);

   int *vertex_ranks;
   const int *atom_atom_mapping;

   GraphHighlighting *highlighting;

   bool ignore_hydrogens;
   bool canonize_chiralities;
   bool write_extra_info;

   int writtenComponents ();
   const Array<int> & writtenAtoms ();
   const Array<int> & writtenBonds ();

   static void writePseudoAtom (const char *label, Output &out);

   bool smarts_mode;
   bool ignore_invalid_hcount;

protected:

   void _saveMolecule ();

   BaseMolecule *_bmol;
   Molecule *_mol;
   QueryMolecule *_qmol;

   struct _Atom
   {
      _Atom (Pool<List<int>::Elem> &neipool);
      ~_Atom ();

      List<int> neighbors;
      int parent;
      
      bool aromatic;   
      bool lowercase;
      int  chirality;  // 0 means no chirality, 1 means CCW pyramid, 2 means CW pyramid
      int  branch_cnt; // runs from 0 to (branches - 1)
      bool paren_written;
   };
   
   Output &_output;

   void _writeCycleNumber (int n) const;
   void _writeAtom (int idx, bool aromatic, bool lowercase, int chirality) const;
   void _writeSmartsAtom (int idx, QueryMolecule::Atom *atom, int chirality, int depth, bool has_or_parent) const;
   void _writeSmartsBond (int idx, QueryMolecule::Bond *bond) const;
   void _markCisTrans ();
   int  _calcBondDirection (int idx, int vprev);
   bool _updateSideBonds (int bond_idx);
   void _writeStereogroups ();
   void _writeRadicals ();
   void _writePseudoAtoms ();
   void _writeHighlighting ();

   struct _DBond // directed bond (near cis-trans bond)
   {
      int ctbond_beg; // cis-trans bond attached to the begin (-1 if there isn't any)
      int ctbond_end; // cis-trans bond attached to the end (-1 if there isn't any)
      int saved; // 0 -- not saved; 1 -- goes 'up' from begin to end; 2 -- goes 'down'
   };

   TL_CP_DECL(Pool<List<int>::Elem>, _neipool);
   TL_CP_DECL(ObjArray<_Atom>, _atoms);
   TL_CP_DECL(Array<int>, _hcount);
   TL_CP_DECL(Array<_DBond>, _dbonds);
   TL_CP_DECL(Array<int>, _written_atoms);
   TL_CP_DECL(Array<int>, _written_bonds);

   int _written_components;
   int _touched_cistransbonds;
   bool _comma;

private:
   SmilesSaver (const SmilesSaver &); // no implicit copy
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
