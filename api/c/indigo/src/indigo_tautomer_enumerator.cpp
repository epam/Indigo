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

#include "indigo_tautomer_enumerator.h"
#include "indigo_molecule.h"

CEXPORT int indigoIterateTautomers(int molecule, const char* options)
{
    INDIGO_BEGIN
    {
        Molecule& mol = self.getObject(molecule).getMolecule();

        TautomerMethod method;
        if (strncasecmp(options, "INCHI", 5) == 0)
            method = INCHI;
        else if (strncasecmp(options, "RSMARTS", 7) == 0)
            method = RSMARTS;
        else
            method = RSMARTS;
        return self.addObject(new IndigoTautomerIter(mol, method));
    }
    INDIGO_END(-1);
}

IndigoTautomerIter::IndigoTautomerIter(Molecule& molecule, TautomerMethod method) : IndigoObject(TAUTOMER_ITER), _enumerator(molecule, method), _complete(false)
{
    bool needAromatize = molecule.isAromatized();
    if (needAromatize)
        _currentPosition = _enumerator.beginAromatized();
    else
        _currentPosition = _enumerator.beginNotAromatized();
}

const char* IndigoTautomerIter::debugInfo() const
{
    return "<tautomer iterator>";
}

IndigoTautomerIter::~IndigoTautomerIter()
{
}

int IndigoTautomerIter::getIndex()
{
    return _currentPosition > 0 ? _currentPosition : -_currentPosition;
}

IndigoObject* IndigoTautomerIter::next()
{
    if (hasNext())
    {
        std::unique_ptr<IndigoMoleculeTautomer> result = std::make_unique<IndigoMoleculeTautomer>(_enumerator, _currentPosition);
        _currentPosition = _enumerator.next(_currentPosition);
        return result.release();
    }
    return NULL;
}

bool IndigoTautomerIter::hasNext()
{
    return _enumerator.isValid(_currentPosition);
}

IndigoMoleculeTautomer::IndigoMoleculeTautomer(TautomerEnumerator& enumerator, int index) : IndigoObject(TAUTOMER_MOLECULE), _index(index)
{
    enumerator.constructMolecule(_molInstance, index);
}

const char* IndigoMoleculeTautomer::debugInfo() const
{
    return "<molecule tautomer>";
}

IndigoMoleculeTautomer::~IndigoMoleculeTautomer()
{
}

IndigoObject* IndigoMoleculeTautomer::clone()
{
    return IndigoMolecule::cloneFrom(*this);
}

int IndigoMoleculeTautomer::getIndex()
{
    return (_index > 0 ? _index : -_index) - 1;
}

Molecule& IndigoMoleculeTautomer::getMolecule()
{
    return _molInstance;
}
