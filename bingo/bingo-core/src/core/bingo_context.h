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

#ifndef __bingo_context__
#define __bingo_context__

#include "base_cpp/nullable.h"
#include "bingo_version.h"
#include "lzw/lzw_dictionary.h"
#include "molecule/molecule_fingerprint.h"
#include "molecule/molecule_tautomer.h"

// extern const char *bingo_version_string;

namespace indigo
{
    class MoleculeAutoLoader;
    class ReactionAutoLoader;
    class SmilesLoader;
    class RSmilesLoader;

    class BingoContext
    {
    public:
        enum
        {
            /*
             * Default operation timeout = 60000 ms
             */
            DEFAULT_TIMEOUT = 60000
        };

        explicit BingoContext(int id_);
        virtual ~BingoContext();

        bool tautomer_rules_ready;
        bool fp_parameters_ready;
        bool atomic_mass_map_ready;

        int id;
        LzwDict cmf_dict;
        LzwDict rid_dict;
        int fp_chunk_qwords;

        int nthreads;
        int timeout;

        Nullable<bool> treat_x_as_pseudoatom;
        Nullable<bool> ignore_closing_bond_direction_mismatch;
        Nullable<bool> ignore_stereocenter_errors;
        Nullable<bool> stereochemistry_bidirectional_mode;
        Nullable<bool> stereochemistry_detect_haworth_projection;
        Nullable<bool> ignore_cistrans_errors;
        Nullable<bool> allow_non_unique_dearomatization;
        Nullable<bool> zero_unknown_aromatic_hydrogens;
        Nullable<bool> ignore_bad_valence;

        // Throw exception when invalid structure is being added to the index
        Nullable<bool> reject_invalid_structures;

        MoleculeFingerprintParameters fp_parameters;

        PtrArray<TautomerRule> tautomer_rules;

        std::map<int, double> relative_atomic_mass_map;

        void setLoaderSettings(MoleculeAutoLoader& loader);
        void setLoaderSettings(ReactionAutoLoader& loader);
        void setLoaderSettings(SmilesLoader& loader);
        void setLoaderSettings(RSmilesLoader& loader);
        StereocentersOptions getStereocentersOptions();

        static void remove(int id);

        void reset();

        DECL_ERROR;

        static BingoContext* get(int id);
        static BingoContext* existing(int id);

    protected:
        TL_DECL(PtrArray<BingoContext>, _instances);

        static BingoContext* _get(int id);
        static std::mutex _instances_lock;

        Array<char> _relative_atomic_mass;

    private:
        BingoContext(const BingoContext&); // no implicit copy
    };
} // namespace indigo
void bingoGetTauCondition(const char* list_ptr, int& aromaticity, indigo::Array<int>& label_list);

void bingoGetName(indigo::Scanner& scanner, indigo::Array<char>& result);

#endif
