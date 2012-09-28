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

#ifndef __molecule_inchi_h__
#define __molecule_inchi_h__

#include "base_cpp/tlscont.h"
#include "base_cpp/array.h"
#include "base_cpp/reusable_obj_array.h"
#include "base_cpp/exception.h"
#include "molecule/molecule.h"
#include "molecule/molecule_inchi_layers.h"
#include "molecule/molecule_inchi_component.h"

namespace indigo {

class Output;
class Graph;

// Molecule InChI code constructor class 
class MoleculeInChI
{
public:

   explicit MoleculeInChI (Output &output);

   // InChI version. By default it is "Indigo=1.1"
   const char *prefix;

   // Save InChI code to the output
   void outputInChI (Molecule &mol);

   DECL_ERROR;

private:
   //
   // Components compare methods
   //

   // Compare components. Returns DIFFERENCE_**** for the first found difference
   static int _cmpComponents (int &index1, int &index2, void *context);

   // 
   // Printing
   //
   void _printInChI ();

   class _PrintLayerFuncBase
   {
   public:
      virtual ~_PrintLayerFuncBase() {}

      virtual void operator() (MoleculeInChICompoment &comp, Array<char> &result) = 0;
   };
   template <typename Layer>
   class _ComponentLayerPrintFunction;

   bool _printInChILayer (_PrintLayerFuncBase &func, const char *delim, const char *multiplier,
      const char *layer_prefix);

   void _printInChIComponentCisTrans (MoleculeInChICompoment &comp, Array<char> &result);

   void _normalizeMolecule (Molecule &mol);

   Output &_output;                                                                    

   // Array with molecule components and InChI information and sorted indices
   TL_CP_DECL(ReusableObjArray<MoleculeInChICompoment>, _components);
   TL_CP_DECL(Array<int>, _component_indices);
};

}

#endif // __molecule_inchi_h__
