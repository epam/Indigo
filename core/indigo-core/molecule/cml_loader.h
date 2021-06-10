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

#ifndef __cml_loader__
#define __cml_loader__

#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "base_cpp/array.h"
#include "base_cpp/exception.h"
#include "molecule/base_molecule.h"
#include "molecule/molecule_stereocenter_options.h"
#include "molecule/query_molecule.h"

class TiXmlHandle;
class TiXmlElement;
class TiXmlNode;

namespace indigo
{
    class Scanner;
    class Molecule;
    class QueryMolecule;

    class CmlLoader
    {
    public:
        DECL_ERROR;

        CmlLoader(Scanner& scanner);
        CmlLoader(TiXmlHandle& handle);

        void loadMolecule(Molecule& mol);
        void loadQueryMolecule(QueryMolecule& mol);

        StereocentersOptions stereochemistry_options;
        bool ignore_bad_valence;

    protected:
        Scanner* _scanner;
        TiXmlHandle* _handle;
        TiXmlNode* _molecule;

        void _loadMolecule();
        void _loadMoleculeElement(TiXmlHandle& handle);
        void _loadSGroupElement(TiXmlElement* elem, std::unordered_map<std::string, int>& atoms_id, int parent);
        void _loadRgroupElement(TiXmlHandle& handle);
        bool _findMolecule(TiXmlNode* node);
        void _parseRlogicRange(const char* str, ArrayInt& ranges);
        void _appendQueryAtom(const char* atom_label, AutoPtr<QueryMolecule::Atom>& atom);

        Molecule* _mol;
        BaseMolecule* _bmol;
        QueryMolecule* _qmol;

    private:
        CmlLoader(const CmlLoader&); // no implicit copy
    };

} // namespace indigo

#endif
