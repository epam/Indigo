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

#include "indigo_internal.h"
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
      IndigoArray &arr = self.getObject(structures).asArray();
      int i;

      mol_set.clear();
      
      for (i = 0; i < arr.objects.size(); i++)
         mol_set.push().clone(arr.objects[i]->getMolecule(), 0, 0);

      if (self.deconvolution_aromatization)
         for(int i = 0; i < mol_set.size(); ++i)
           MoleculeAromatizer::aromatizeBonds(mol_set[i]);

      AutoPtr<IndigoScaffold> scaf(new IndigoScaffold());

      MoleculeScaffoldDetection msd(&mol_set);

      msd.flags = MoleculeExactMatcher::CONDITION_ALL & ~MoleculeExactMatcher::CONDITION_STEREO;
      msd.basketStructures = &scaf->all_scaffolds;

      bool approximate = false;

      if (options != 0)
      {
         BufferScanner scanner(options);
         QS_DEF(Array<char>, word);

         scanner.skipSpace();
         if (!scanner.isEOF())
            scanner.readWord(word, 0);

         if (strcasecmp(word.ptr(), "APPROX") == 0)
            approximate = true;
         else if (strcasecmp(word.ptr(), "EXACT") == 0)
            approximate = false;
         else
            throw IndigoError("indigoExtractCommonScaffold: unknown option %s\n", word.ptr());
      }

      if (approximate)
         msd.extractApproximateScaffold(scaf->max_scaffold);
      else
         msd.extractExactScaffold(scaf->max_scaffold);

      return self.addObject(scaf.release());
   }
   INDIGO_END(0, -1);
}

Molecule & IndigoScaffold::getMolecule ()
{
   return max_scaffold;
}

BaseMolecule & IndigoScaffold::getBaseMolecule ()
{
   return max_scaffold;
}

GraphHighlighting * IndigoScaffold::getMoleculeHighlighting ()
{
   return 0;
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
         AutoPtr<IndigoMolecule> mol(new IndigoMolecule);
         mol->mol.clone(scaf.all_scaffolds[i], 0, 0);
         arr->objects.add(mol.release());
      }

      return self.addObject(arr.release());
   }
   INDIGO_END(0, -1);
}
