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

#ifndef __molecule_ionize_h__
#define __molecule_ionize_h__

#include "base_cpp/obj_array.h"
#include "base_cpp/red_black.h"
#include "base_cpp/tlscont.h"
#include "base_molecule.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class Molecule;
    class QueryMolecule;

    struct IonizeOptions
    {
        enum PkaModel
        {
            PKA_MODEL_SIMPLE,
            PKA_MODEL_ADVANCED
        };

        PkaModel model;
        int level = 0;
        int min_level = 0;

        IonizeOptions(PkaModel model = PKA_MODEL_SIMPLE) : model(model)
        {
        }
    };

    class MoleculePkaModel
    {
    public:
        DECL_ERROR;
        static void estimate_pKa(Molecule& mol, const IonizeOptions& options, Array<int>& acid_sites, Array<int>& basic_sites, Array<float>& acid_pkas,
                                 Array<float>& basic_pkas);
        static void getAtomLocalFingerprint(Molecule& mol, int idx, Array<char>& fp, int level);
        static void getAtomLocalKey(Molecule& mol, int idx, Array<char>& fp);
        static bool getAtomLocalFeatureSet(BaseMolecule& mol, int idx, Array<int>& fp);
        static int buildPkaModel(int level, float threshold, const char* filename);

        static float getAcidPkaValue(Molecule& mol, int idx, int level, int min_level);
        static float getBasicPkaValue(Molecule& mol, int idx, int level, int min_level);

    private:
        MoleculePkaModel();
        static MoleculePkaModel _model;

        static void _loadSimplePkaModel();
        static void _loadAdvancedPkaModel();
        static void _estimate_pKa_Simple(Molecule& mol, const IonizeOptions& options, Array<int>& acid_sites, Array<int>& basic_sites, Array<float>& acid_pkas,
                                         Array<float>& basic_pkas);

        static void _estimate_pKa_Advanced(Molecule& mol, const IonizeOptions& options, Array<int>& acid_sites, Array<int>& basic_sites,
                                           Array<float>& acid_pkas, Array<float>& basic_pkas);

        static int _asc_cmp_cb(int& v1, int& v2, void* context);
        static void _checkCanonicalOrder(Molecule& mol, Molecule& can_mol, Array<int>& order);
        static void _removeExtraHydrogens(Molecule& mol);

        ObjArray<QueryMolecule> acids;
        ObjArray<QueryMolecule> basics;
        Array<float> a_pkas;
        Array<float> b_pkas;
        bool simple_model_ready = false;

        RedBlackStringObjMap<Array<float>> adv_a_pkas;
        RedBlackStringObjMap<Array<float>> adv_b_pkas;
        int level;
        Array<float> max_deviations;
        bool advanced_model_ready = false;
    };

    class DLLEXPORT MoleculeIonizer
    {
    public:
        MoleculeIonizer();

        static bool ionize(Molecule& molecule, float ph, float ph_toll, const IonizeOptions& options);

        DECL_ERROR;
        CP_DECL;

    protected:
        static void _setCharges(Molecule& mol, float ph, float ph_toll, const IonizeOptions& options, Array<int>& acid_sites, Array<int>& basic_sites,
                                Array<float>& acid_pkas, Array<float>& basic_pkas);
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
