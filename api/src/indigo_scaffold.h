/****************************************************************************
 * Copyright (C) 2010 GGA Software Services LLC
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

#ifndef __indigo_scaffold__
#define __indigo_scaffold__

#include "indigo_internal.h"
#include "molecule/molecule.h"

class IndigoScaffold : public IndigoObject
{
public:
   IndigoScaffold ();
   virtual ~IndigoScaffold ();

   void extractScaffold ();

   virtual Molecule & getMolecule();
   virtual BaseMolecule & getBaseMolecule ();
   virtual GraphHighlighting * getMoleculeHighlighting ();

   Molecule           max_scaffold;
   ObjArray<Molecule> all_scaffolds;
};

#endif
