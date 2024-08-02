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

#ifndef __molecule_json_loader__
#define __molecule_json_loader__

#include <list>
#include <rapidjson/document.h>
#include <unordered_set>
#include <vector>

#include "base_c/defs.h"
#include "base_cpp/exception.h"
#include "base_cpp/non_copyable.h"
#include "molecule/molecule_stereocenter_options.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{
    class Scanner;
    class BaseMolecule;
    class MetaDataStorage;
    class Molecule;
    class QueryMolecule;
    class SGroup;
    class Superatom;
    class MetaObjectsInterface;
    class MonomerTemplateLibrary;
    class KetDocument;
    class KetMolecule;

    /*
     * Loader for JSON format
     */

    class DLLEXPORT MoleculeJsonLoader : public NonCopyable
    {
        using RGroupDescriptionList = std::list<std::pair<int, std::reference_wrapper<rapidjson::Value>>>;

    public:
        DECL_ERROR;
        explicit MoleculeJsonLoader(rapidjson::Document& ket);
        explicit MoleculeJsonLoader(rapidjson::Value& mol_nodes);
        explicit MoleculeJsonLoader(Scanner& scanner);

        void loadMolecule(BaseMolecule& mol, bool load_arrows = false);
        void loadMonomerLibrary(MonomerTemplateLibrary& library);
        StereocentersOptions stereochemistry_options;
        bool treat_x_as_pseudoatom; // normally 'X' means 'any halogen'
        bool skip_3d_chirality;     // do not compute chirality from 3D coordinates
        bool ignore_no_chiral_flag; // ignore chiral flag absence (treat stereo "as drawn")
                                    // (depricated, use treat_stereo-as instead of this option)

        // When true, the "bond topology", "stereo care", "ring bond count", and "unsaturation"
        // specifications are ignored when a non-query molecule is being loaded.
        // Otherwise, an error is thrown (this is the default).
        bool ignore_noncritical_query_features;

        int treat_stereo_as; // treat stereo as 'ucf', 'abs', 'rel', 'rac', 'any'
                             //  = 0 ('ucf') - use chiral flag (default value)
                             //  = ATOM_ABS ('abs')
                             //  = ATOM_OR  ('rel')
                             //  = ATOM_AND ('rac')
                             //  = ATOM_ANY ('any')

        static void loadMetaObjects(rapidjson::Value& meta_objects, MetaDataStorage& meta);
        static int parseMonomerTemplate(const rapidjson::Value& monomer_template, BaseMolecule& mol, StereocentersOptions stereochemistry_options);

    protected:
        struct EnhancedStereoCenter
        {
            EnhancedStereoCenter(int atom_idx, int type, int group) : _atom_idx(atom_idx), _type(type), _group(group)
            {
            }
            int _atom_idx;
            int _type;
            int _group;
        };

        int addAtomToMoleculeQuery(const char* label, int element, int charge, int valence, int radical, int isotope);
        int addBondToMoleculeQuery(int beg, int end, int order, int topology = 0, int direction = 0);
        void validateMoleculeBond(int order);
        void parseAtoms(const rapidjson::Value& atoms, BaseMolecule& mol, std::vector<EnhancedStereoCenter>& stereo_centers);
        void parseBonds(const rapidjson::Value& bonds, BaseMolecule& mol);
        void parseHighlight(const rapidjson::Value& highlight, BaseMolecule& mol);
        void parseSGroups(const rapidjson::Value& sgroups, BaseMolecule& mol);
        void parseProperties(const rapidjson::Value& props, BaseMolecule& mol);
        void setStereoFlagPosition(const rapidjson::Value& pos, int fragment_index, BaseMolecule& mol);
        void handleSGroup(SGroup& sgroup, const std::unordered_set<int>& atoms, BaseMolecule& bmol);
        static void addMonomerTemplate(const rapidjson::Value& mt_json, MonomerTemplateLibrary* library, KetDocument* document);
        void addToLibMonomerGroupTemplate(MonomerTemplateLibrary& library, const rapidjson::Value& monomer_group_template);
        static std::string monomerTemplateClass(const rapidjson::Value& monomer_template);
        std::string monomerMolClass(const std::string& class_name);

    private:
        void parse_ket(rapidjson::Document& ket);
        static void fillXBondsAndBrackets(Superatom& sa, BaseMolecule& mol);
        rapidjson::Value& _mol_nodes;

        RGroupDescriptionList _rgroups;

        rapidjson::Value _meta_objects;
        rapidjson::Value _mol_array;
        rapidjson::Value _monomer_array;
        rapidjson::Value _connection_array;
        rapidjson::Value _templates;
        std::unordered_map<std::string, int> _id_to_template;
        std::map<std::string, std::string> _template_ref_to_id;
        Molecule* _pmol;
        QueryMolecule* _pqmol;
        std::vector<EnhancedStereoCenter> _stereo_centers;
        unsigned int components_count;
        rapidjson::Document _document;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
