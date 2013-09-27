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

#include "indigo_scaffold.h"
#include "indigo_array.h"
#include "indigo_molecule.h"
#include "molecule/molecule_scaffold_detection.h"
#include "molecule/molecule_exact_matcher.h"
#include "base_cpp/scanner.h"

IndigoScaffold::IndigoScaffold () : IndigoObject(SCAFFOLD)
{
}

IndigoScaffold::~IndigoScaffold ()
{
}

CEXPORT int indigoExtractCommonScaffold (int structures, const char* options)
{
   INDIGO_BEGIN
   {
      QS_DEF(ObjArray<Molecule>, mol_set);
      IndigoArray &arr = IndigoArray::cast(self.getObject(structures));
      int i;

      mol_set.clear();
      
      for (i = 0; i < arr.objects.size(); i++)
         mol_set.push().clone(arr.objects[i]->getMolecule(), 0, 0);

      if (self.deconvolution_aromatization)
         for(int i = 0; i < mol_set.size(); ++i)
            MoleculeAromatizer::aromatizeBonds(mol_set[i], self.arom_options);

      AutoPtr<IndigoScaffold> scaf(new IndigoScaffold());

      MoleculeScaffoldDetection msd(&mol_set);

      msd.basketStructures = &scaf->all_scaffolds;

      bool approximate = false;
      int max_iterations = 0;

      if (options != 0) {
         BufferScanner scanner(options);
         QS_DEF(Array<char>, word);

         scanner.skipSpace();
         if (!scanner.isEOF()) {
            scanner.readWord(word, 0);

            if (strcasecmp(word.ptr(), "APPROX") == 0)
               approximate = true;
            else if (strcasecmp(word.ptr(), "EXACT") == 0)
               approximate = false;
            else
               throw IndigoError("indigoExtractCommonScaffold: unknown option %s\n", word.ptr());

            scanner.skipSpace();
            if (!scanner.isEOF()) {
               max_iterations = scanner.readInt();
            }
         }
      }
      if(max_iterations > 0)
         msd.maxIterations = max_iterations;

      if (approximate)
         msd.extractApproximateScaffold(scaf->max_scaffold);
      else
         msd.extractExactScaffold(scaf->max_scaffold);

      return self.addObject(scaf.release());
   }
   INDIGO_END(-1);
}

QueryMolecule & IndigoScaffold::getQueryMolecule ()
{
   return max_scaffold;
}

BaseMolecule & IndigoScaffold::getBaseMolecule ()
{
   return max_scaffold;
}

CEXPORT int indigoAllScaffolds (int extracted)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(extracted);

      if (obj.type != IndigoObject::SCAFFOLD)
         throw IndigoError("indigoAllScaffolds(): can not accept %s",
                           obj.debugInfo());

      IndigoScaffold &scaf = (IndigoScaffold &)obj;

      AutoPtr<IndigoArray> arr(new IndigoArray());
      int i;

      for (i = 0; i < scaf.all_scaffolds.size(); i++)
      {
         AutoPtr<IndigoQueryMolecule> mol(new IndigoQueryMolecule);
         mol->qmol.clone(scaf.all_scaffolds[i], 0, 0);
         arr->objects.add(mol.release());
      }

      return self.addObject(arr.release());
   }
   INDIGO_END(-1);
}
