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

#ifndef __molecule_json_saver_h__
#define __molecule_json_saver_h__

#include <sstream>

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "base_cpp/locale_guard.h"
#include "base_cpp/output.h"
#include "molecule/base_molecule.h"
#include "molecule/elements.h"

namespace indigo
{

    class Molecule;
    class QueryMolecule;
    class Output;

    class DLLEXPORT MoleculeJsonSaver
    {
    public:
        explicit MoleculeJsonSaver(Output& output);
        void saveMolecule(BaseMolecule& bmol);
        void saveMolecule(BaseMolecule& bmol, rapidjson::Writer<rapidjson::StringBuffer>& writer);
        static void saveSimpleObjects(const PtrArray<MetaObject>& meta_objects, rapidjson::Writer<rapidjson::StringBuffer>& writer);
        bool _add_stereo_desc;

    protected:
        void saveAtoms(BaseMolecule& mol, rapidjson::Writer<rapidjson::StringBuffer>& writer);
        void saveBonds(BaseMolecule& mol, rapidjson::Writer<rapidjson::StringBuffer>& writer);
        void saveRGroup(PtrPool<BaseMolecule>& fragments, int rgnum, rapidjson::Writer<rapidjson::StringBuffer>& writer);
        void saveSGroups(BaseMolecule& mol, rapidjson::Writer<rapidjson::StringBuffer>& writer);
        void saveSGroup(SGroup& sgroup, rapidjson::Writer<rapidjson::StringBuffer>& writer);
        void saveAttachmentPoint(BaseMolecule& mol, int atom_idx, rapidjson::Writer<rapidjson::StringBuffer>& writer);
        void saveStereoCenter(BaseMolecule& mol, int atom_idx, rapidjson::Writer<rapidjson::StringBuffer>& writer);
        void saveHighlights(BaseMolecule& mol, rapidjson::Writer<rapidjson::StringBuffer>& writer);
        void saveSelection(BaseMolecule& mol, rapidjson::Writer<rapidjson::StringBuffer>& writer);

        DECL_ERROR;

    protected:
        void _checkSGroupIndices(BaseMolecule& mol, Array<int>& sgs_list);

        Molecule* _pmol;
        QueryMolecule* _pqmol;
        Output& _output;

    private:
        MoleculeJsonSaver(const MoleculeJsonSaver&); // no implicit copy
    };

} // namespace indigo

#endif
