/****************************************************************************
 * Copyright (C) 2010-2012 GGA Software Services LLC
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

#ifndef __indigo_inchi_core_h__
#define __indigo_inchi_core_h__

#include "base_cpp/array.h"
#include "base_cpp/exception.h"

#include "inchi_api.h"

namespace indigo
{

// Forward declaration
class Molecule; 
struct InchiOutput;

class IndigoInchi
{
public:
   IndigoInchi ();

   void clear();

   // Input parameters
   void setOptions (const char *opt);

   // Output additional results
   Array<char> warning, log, auxInfo;

   void loadMoleculeFromInchi (const char *inchi, Molecule &mol);
   void loadMoleculeFromAux (const char *aux, Molecule &mol);

   void saveMoleculeIntoInchi (Molecule &mol, Array<char> &inchi);

   void parseInchiOutput (const InchiOutput &inchi_output, Molecule &mol);

   void generateInchiInput (Molecule &mol, inchi_Input &input, 
      Array<inchi_Atom> &atoms, Array<inchi_Stereo0D> &stereo);

   void neutralizeV5Nitrogen (Molecule &mol);

   static const char* version ();

   static void InChIKey (const char *inchi, Array<char> &output);

   DECL_EXCEPTION_NO_EXP(Error);

private:
   Array<char> options;
};

}

#endif // __indigo_inchi_core_h__
