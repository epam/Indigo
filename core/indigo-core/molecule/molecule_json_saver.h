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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include <sstream>

#include "base_cpp/locale_guard.h"
#include "base_cpp/output.h"
#include "molecule/base_molecule.h"
#include "molecule/elements.h"
#include "molecule/json_writer.h"
#include "molecule/meta_commons.h"
#include "molecule/monomers_lib.h"
#include "molecule/query_molecule.h"

namespace indigo
{

    class Molecule;
    class QueryMolecule;
    class Output;
    class MonomerTemplate;

    class DLLEXPORT MoleculeJsonSaver
    {
    public:
        explicit MoleculeJsonSaver(Output& output);
        void saveMolecule(BaseMolecule& bmol);
        void saveMolecule(BaseMolecule& bmol, JsonWriter& writer);

        void saveMetaData(JsonWriter& writer, MetaDataStorage& meta);
        static std::string monomerId(const TGroup& tg);
        static std::string monomerKETClass(const std::string& class_name);
        static std::string monomerHELMClass(const std::string& class_name);

        bool add_stereo_desc;
        bool pretty_json;
        bool use_native_precision; // TODO: Remove option and use_native_precision allways - have to fix a lot of UTs

    protected:
        void saveRoot(BaseMolecule& mol, JsonWriter& writer);
        void saveMoleculeReference(int mol_id, JsonWriter& writer);
        void saveEndpoint(BaseMolecule& mol, const std::string& ep, int beg_idx, int end_idx, JsonWriter& writer, bool hydrogen = false);
        int getMonomerNumber(int mon_idx);

        void writeFloat(JsonWriter& writer, float f_value);
        void writePos(JsonWriter& writer, const Vec3f& pos);

        void saveAtoms(BaseMolecule& mol, JsonWriter& writer);
        void saveBonds(BaseMolecule& mol, JsonWriter& writer);
        void saveRGroup(RGroup& rgroup, int rgnum, JsonWriter& writer);
        void saveFragment(BaseMolecule& fragment, JsonWriter& writer);
        void saveMonomerTemplate(TGroup& tg, JsonWriter& writer);
        void saveAmbiguousMonomerTemplate(TGroup& tg, JsonWriter& writer);
        void saveMonomerAttachmentPoints(TGroup& tg, JsonWriter& writer);
        void saveSuperatomAttachmentPoints(Superatom& sa, JsonWriter& writer);

        void saveSGroups(BaseMolecule& mol, JsonWriter& writer);
        void saveSGroup(SGroup& sgroup, JsonWriter& writer);

        void saveAttachmentPoint(BaseMolecule& mol, int atom_idx, JsonWriter& writer);
        void saveStereoCenter(BaseMolecule& mol, int atom_idx, JsonWriter& writer);
        void saveHighlights(BaseMolecule& mol, JsonWriter& writer);

        DECL_ERROR;

    protected:
        void _checkSGroupIndices(BaseMolecule& mol, Array<int>& sgs_list);
        bool _checkAttPointOrder(BaseMolecule& mol, int rsite);
        bool _needCustomQuery(QueryMolecule::Atom* atom) const;
        void _writeQueryProperties(QueryMolecule::Atom* atom, JsonWriter& writer);

        Molecule* _pmol;
        QueryMolecule* _pqmol;
        Output& _output;
        std::list<std::unordered_set<int>> _s_neighbors;
        std::unordered_map<std::pair<std::string, std::string>, std::reference_wrapper<TGroup>, pair_hash> _templates;
        std::unordered_map<std::pair<int, int>, std::string, pair_int_hash> _monomer_connections;
        std::map<int, int> _monomers_enum;
        std::vector<std::unique_ptr<BaseMolecule>> _no_template_molecules;
        ObjArray<Array<int>> _mappings;
        std::unordered_map<int, int> _atom_to_mol_id;

    private:
        MoleculeJsonSaver(const MoleculeJsonSaver&); // no implicit copy
    };

} // namespace indigo

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
