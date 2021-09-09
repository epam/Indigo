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

#include "indigo_internal.h"
#include "indigo_molecule.h"

#include "molecule/molecule_rgroups_composition.h"

using MoleculeIter = MoleculeRGroupsComposition::MoleculeIter;

class DLLEXPORT IndigoCompositionElem : public IndigoObject
{
public:
    IndigoCompositionElem() : IndigoObject(COMPOSITION_ELEM)
    {
    }
    ~IndigoCompositionElem() override
    {
    }

    Molecule molecule;
    MoleculeRGroups variants[RGCOMP_OPT_COUNT];
};

CEXPORT int indigoGetFragmentedMolecule(int elem, const char* options)
{
    INDIGO_BEGIN
    {
        if (!strcmp(options, ""))
        {
            options = MoleculeIter::OPTION(RGCOMP_OPT::ERASE);
        }

        IndigoObject& obj = self.getObject(elem);
        IndigoCompositionElem& elem = dynamic_cast<IndigoCompositionElem&>(obj);

        MoleculeRGroups* rgroups = nullptr;
        RGCOMP_OPT OPTS[RGCOMP_OPT_COUNT] = RGCOMP_OPT_ENUM;
        for (auto i = 0; i < RGCOMP_OPT_COUNT; i++)
        {
            if (!strcmp(options, MoleculeIter::OPTION(OPTS[i])))
            {
                rgroups = &elem.variants[i];
                break;
            }
        }
        if (rgroups == nullptr)
        {
            throw IndigoError("indigoGetFragmentedMolecule(): weird options \"%s\"", options);
        }

        std::unique_ptr<IndigoMolecule> result = std::make_unique<IndigoMolecule>();
        result->mol.clone(elem.molecule, nullptr, nullptr);
        result->mol.rgroups.copyRGroupsFromMolecule(*rgroups);
        return self.addObject(result.release());
    }
    INDIGO_END(-1);
}

class DLLEXPORT IndigoCompositionIter : public IndigoObject
{
public:
    IndigoCompositionIter(BaseMolecule& mol) : IndigoObject(COMPOSITION_ITER), _composition(mol), _it(_composition.begin()), _end(_composition.end())
    {
    }
    ~IndigoCompositionIter() override
    {
    }

    IndigoObject* next() override
    {
        if (!_hasNext)
        {
            return nullptr;
        }
        std::unique_ptr<IndigoCompositionElem> result = std::make_unique<IndigoCompositionElem>();
        _it.dump(result->molecule);
        RGCOMP_OPT OPTS[RGCOMP_OPT_COUNT] = RGCOMP_OPT_ENUM;
        for (auto i = 0; i < RGCOMP_OPT_COUNT; i++)
        {
            result->variants[i].copyRGroupsFromMolecule(*_it.modifyRGroups(MoleculeIter::OPTION(OPTS[i])));
        }

        _hasNext = _it.next();
        return result.release();
    }
    bool hasNext() override
    {
        return _hasNext;
    }

protected:
    MoleculeRGroupsComposition _composition;
    MoleculeRGroupsComposition::MoleculeIter _it;
    MoleculeRGroupsComposition::MoleculeIter _end;

    bool _hasNext = true;
};

CEXPORT int indigoRGroupComposition(int molecule, const char* options)
{
    INDIGO_BEGIN
    {
        BaseMolecule& target = self.getObject(molecule).getBaseMolecule();
        return self.addObject(new IndigoCompositionIter(target));
    }
    INDIGO_END(-1);
}