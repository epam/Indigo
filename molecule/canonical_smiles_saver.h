/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
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

#ifndef __canonical_smiles_saver__
#define __canonical_smiles_saver__

#include "molecule/smiles_saver.h"
#include "base_cpp/exception.h"
#include "base_cpp/output.h"

#include <memory>

namespace indigo {

class DLLEXPORT CanonicalSmilesSaver
{
public:

   enum { MAX_NUMBER_OF_COMPONENTS = 2 };
    
   explicit CanonicalSmilesSaver (Output &output);
   ~CanonicalSmilesSaver ();

   bool find_invalid_stereo;

   void saveMolecule (Molecule &mol);
   
   void setSmartsMode(bool smarts_mode);
   void setInsideRsmiles(bool inside_rsmiles);
   void saveQueryMolecule (QueryMolecule &mol);
   int writtenComponents ();
   const Array<int> & writtenAtoms ();
   const Array<int> & writtenBonds ();

   DECL_ERROR;

protected:
   void _processMolecule (Molecule &mol);
   typedef RedBlackMap<int, int> MapIntInt;

   CP_DECL;   
   TL_CP_DECL(Array<int>, _actual_atom_atom_mapping);
   TL_CP_DECL(MapIntInt, _initial_to_actual);
   
   int _aam_counter;
   Output& _output;
   Array<char> _buffer;
   std::unique_ptr<ArrayOutput> _arrayOutput;
   std::unique_ptr<SmilesSaver> _smilesSaver;
};

}

#endif
