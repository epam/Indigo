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

#ifndef __cml_saver_h__
#define __cml_saver_h__

#include "molecule/base_molecule.h"

namespace tinyxml2
{
    class XMLDocument;
    class XMLElement;
}

namespace indigo
{

    class Molecule;
    class QueryMolecule;
    class Output;
    class SGroup;

    class CmlSaver
    {
    public:
        explicit CmlSaver(Output& output);

        void saveMolecule(Molecule& mol);
        void saveQueryMolecule(QueryMolecule& mol);
        bool skip_cml_tag; // skips <?xml> and <cml> tags

        DECL_ERROR;

    protected:
        void _saveMolecule(BaseMolecule& bmol, bool query);
        void _validate(BaseMolecule& bmol);

        void _addMoleculeElement(tinyxml2::XMLElement* elem, BaseMolecule& mol, bool query);
        void _addSgroupElement(tinyxml2::XMLElement* elem, BaseMolecule& mol, SGroup& sgroup);
        void _addRgroups(tinyxml2::XMLElement* elem, BaseMolecule& mol, bool query);
        void _addRgroupElement(tinyxml2::XMLElement* elem, RGroup& rgroup, bool query);

        bool _getRingBondCountFlagValue(QueryMolecule& qmol, int idx, int& value);
        bool _getSubstitutionCountFlagValue(QueryMolecule& qmol, int idx, int& value);
        void _writeOccurrenceRanges(Output& out, const Array<int>& occurrences);

        Output& _output;

        tinyxml2::XMLDocument* _doc;
        tinyxml2::XMLElement* _root;

    private:
        CmlSaver(const CmlSaver&); // no implicit copy
    };

} // namespace indigo

#endif
