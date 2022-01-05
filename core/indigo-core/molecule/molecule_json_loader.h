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
    class Molecule;
    class QueryMolecule;
    class SGroup;

    /*
     * Loader for JSON format
     */

    class DLLEXPORT MoleculeJsonLoader : public NonCopyable
    {
    public:
        DECL_ERROR;
        explicit MoleculeJsonLoader(rapidjson::Value& molecule, rapidjson::Value& rgroups, rapidjson::Value& simple_objects);
        explicit MoleculeJsonLoader(rapidjson::Value& molecule, rapidjson::Value& rgroups);
        void loadMolecule(BaseMolecule& mol);
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
        int addBondToMoleculeQuery(int beg, int end, int order, int topology = 0);
        void validateMoleculeBond(int order);
        void parseAtoms(const rapidjson::Value& atoms, BaseMolecule& mol, std::vector<EnhancedStereoCenter>& stereo_centers);
        void parseBonds(const rapidjson::Value& bonds, BaseMolecule& mol);
        void parseHighlight(const rapidjson::Value& highlight, BaseMolecule& mol);
        void parseSelection(const rapidjson::Value& selection, BaseMolecule& mol);
        void parseSGroups(const rapidjson::Value& sgroups, BaseMolecule& mol);
        void setStereoFlagPosition(const rapidjson::Value& pos, int fragment_index, BaseMolecule& mol);
        void handleSGroup(SGroup& sgroup, const std::unordered_set<int>& atoms, BaseMolecule& bmol);

    private:
        rapidjson::Value _empty_array;
        rapidjson::Value& _mol_nodes;
        rapidjson::Value& _rgroups;
        rapidjson::Value& _simple_objects;
        Molecule* _pmol;
        QueryMolecule* _pqmol;
        std::vector<EnhancedStereoCenter> _stereo_centers;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
