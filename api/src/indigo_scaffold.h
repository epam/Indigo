/****************************************************************************
 * Copyright (C) 2010-2011 GGA Software Services LLC
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
#include "molecule/query_molecule.h"

class IndigoScaffold : public IndigoObject
{
public:
   IndigoScaffold ();
   virtual ~IndigoScaffold ();

   void extractScaffold ();

   virtual QueryMolecule & getQueryMolecule ();
   virtual BaseMolecule & getBaseMolecule ();

   QueryMolecule           max_scaffold;
   ObjArray<QueryMolecule> all_scaffolds;
};

#endif