/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#include "indigo_scaffold.h"
#include "base_cpp/scanner.h"
#include "indigo_array.h"
#include "indigo_molecule.h"
#include "molecule/molecule_exact_matcher.h"
#include "molecule/molecule_scaffold_detection.h"

IndigoScaffold::IndigoScaffold() : IndigoObject(SCAFFOLD)
{
}

IndigoScaffold::~IndigoScaffold()
{
}

CEXPORT int indigoExtractCommonScaffold(int structures, const char* options)
{
    INDIGO_BEGIN
    {
        QS_DEF(ObjArray<Molecule>, mol_set);
        IndigoArray& arr = IndigoArray::cast(self.getObject(structures));
        int i;

        mol_set.clear();

        for (i = 0; i < arr.objects.size(); i++)
            mol_set.push().clone(arr.objects[i]->getMolecule(), 0, 0);

        if (self.deconvolution_aromatization)
            for (int i = 0; i < mol_set.size(); ++i)
                MoleculeAromatizer::aromatizeBonds(mol_set[i], self.arom_options);

        AutoPtr<IndigoScaffold> scaf(new IndigoScaffold());

        MoleculeScaffoldDetection msd(&mol_set);

        msd.basketStructures = &scaf->all_scaffolds;

        bool approximate = false;
        int max_iterations = 0;

        if (options != 0)
        {
            BufferScanner scanner(options);
            QS_DEF(ArrayChar, word);

            scanner.skipSpace();
            if (!scanner.isEOF())
            {
                scanner.readWord(word, 0);

                if (strcasecmp(word.ptr(), "APPROX") == 0)
                    approximate = true;
                else if (strcasecmp(word.ptr(), "EXACT") == 0)
                    approximate = false;
                else
                    throw IndigoError("indigoExtractCommonScaffold: unknown option %s\n", word.ptr());

                scanner.skipSpace();
                if (!scanner.isEOF())
                {
                    max_iterations = scanner.readInt();
                }
            }
        }
        if (max_iterations > 0)
            msd.maxIterations = max_iterations;

        if (approximate)
            msd.extractApproximateScaffold(scaf->max_scaffold);
        else
            msd.extractExactScaffold(scaf->max_scaffold);

        return self.addObject(scaf.release());
    }
    INDIGO_END(-1);
}

QueryMolecule& IndigoScaffold::getQueryMolecule()
{
    return max_scaffold;
}

BaseMolecule& IndigoScaffold::getBaseMolecule()
{
    return max_scaffold;
}

CEXPORT int indigoAllScaffolds(int extracted)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(extracted);

        if (obj.type != IndigoObject::SCAFFOLD)
            throw IndigoError("indigoAllScaffolds(): can not accept %s", obj.debugInfo());

        IndigoScaffold& scaf = (IndigoScaffold&)obj;

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
