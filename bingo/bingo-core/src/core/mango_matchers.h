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

#ifndef __mango_matchers__
#define __mango_matchers__

#include "base_cpp/reusable_obj_array.h"
#include "base_cpp/tlscont.h"
#include "molecule/cmf_loader.h"
#include "molecule/molecule.h"
#include "molecule/molecule_neighbourhood_counters.h"
#include "molecule/molecule_substructure_matcher.h"
#include "molecule/molecule_tautomer.h"
#include "molecule/query_molecule.h"
#include <memory>

namespace indigo
{

    class BingoContext;

    class MangoSubstructure
    {
    public:
        MangoSubstructure(BingoContext& context);

        void loadQuery(const Array<char>& buf);
        void loadQuery(const char* str);
        void loadQuery(Scanner& scanner);

        void loadSMARTS(const Array<char>& buf);
        void loadSMARTS(const char* str);
        void loadSMARTS(Scanner& scanner);

        void loadTarget(const Array<char>& molfile_buf);
        void loadTarget(const char* target);
        void loadTarget(Scanner& scanner);
        bool matchLoadedTarget();

        bool matchBinary(const Array<char>& target_buf, const Array<char>* xyz_buf);
        bool matchBinary(Scanner& scanner, Scanner* xyz_scanner);

        void loadBinaryTargetXyz(Scanner& xyz_scanner);

        const byte* getQueryFingerprint();

        void getHighlightedTarget(Array<char>& molfile_buf);
        void getHighlightedTarget_Smiles(Array<char>& smiles_buf);

        bool needCoords();

        // parameters to pass to MoleculeSubstructureMatcher
        int match_3d;
        float rms_threshold;

        bool preserve_bonds_on_highlighting;

        bool parse(const char* params);

        DECL_ERROR;

    protected:
        BingoContext& _context;

        Molecule _target;
        QueryMolecule _query;
        Array<int> _target_bond_types;
        Array<byte> _query_fp;
        bool _use_pi_systems_matcher;
        MoleculeAtomNeighbourhoodCounters _nei_target_counters;
        MoleculeAtomNeighbourhoodCounters _nei_query_counters;

        ObjArray<RedBlackStringMap<int>> _fmcache;

        // cmf loader for delayed xyz loading
        Obj<CmfLoader> cmf_loader;

        bool _query_has_stereocare_bonds;
        bool _query_has_stereocenters;
        bool _query_fp_valid;
        bool _query_extra_valid;

        void _validateQueryFP();
        void _validateQueryExtraData();

        void _initQuery(QueryMolecule& query_in, QueryMolecule& query_out);
        void _initSmartsQuery(QueryMolecule& query_in, QueryMolecule& query_out);
        void _initTarget(bool from_database);

        void _correctQueryStereo(QueryMolecule& query);
    };

    class MangoExact
    {
    public:
        MangoExact(BingoContext& context);

        void loadQuery(const Array<char>& buf);
        void loadQuery(Scanner& scanner);
        void loadQuery(const char* buf);

        // Molecule hash consists of connected components hash with counter
        struct HashElement
        {
            HashElement()
            {
            }
            int count;
            dword hash;

            void clear()
            {
                hash = 0;
                count = 0;
            }
        };
        typedef ReusableObjArray<HashElement> Hash;

        const Hash& getQueryHash() const;

        void loadTarget(const Array<char>& molfile_buf);
        void loadTarget(Scanner& scanner);
        void loadTarget(const char* target);

        bool matchLoadedTarget();

        bool matchBinary(Scanner& scanner, Scanner* xyz_scanner);
        bool matchBinary(const Array<char>& target_buf, const Array<char>* xyz_buf);

        void setParameters(const char* conditions);
        bool parse(const char* params);
        bool needCoords() const;
        bool needComponentMatching() const;

        static void calculateHash(Molecule& mol, Hash& hash);

    protected:
        BingoContext& _context;

        Molecule _query;
        Molecule _target;
        Hash _query_hash;
        int _flags;
        float _rms_threshold;

        void _initQuery(Molecule& query);
        static void _initTarget(Molecule& target, bool from_database);
    };

    class MangoSimilarity
    {
    public:
        MangoSimilarity(BingoContext& context);

        enum
        {
            BIT_METRICS_TANIMOTO,
            BIT_METRICS_TVERSKY,
            BIT_METRICS_EUCLID_SUB
        };

        struct Metrics
        {
            Metrics(int type = -1) : type(type)
            {
            }

            // Metric type
            int type;
            // Additional parameters for tversky metric
            float tversky_alpha, tversky_beta;
        };

        Metrics metrics;
        float bottom, top;

        bool include_bottom;
        bool include_top;

        static Metrics whichMetrics(const char* metrics_str);
        void setMetrics(const char* metrics_str);

        void loadQuery(const Array<char>& buf);
        void loadQuery(Scanner& scanner);
        void loadQuery(const char* str);

        float calc(const Array<char>& target_buf);
        float calc(Scanner& scanner);

        // upper and lower bounds for common ones
        int getLowerBound(int target_ones);
        int getUpperBound(int target_ones);

        bool match(int ones_target, int ones_common);
        bool matchBinary(Scanner& scanner);

        const byte* getQueryFingerprint();

        // Returns stored similarity score after calc, match or matchBinary
        float getSimilarityScore();

        DECL_ERROR;

    protected:
        BingoContext& _context;
        Array<byte> _query_fp;
        int _query_ones;

        float _numerator_value, _denominator_value;

        static void _initQuery(Molecule& query);
        static float _similarity(int ones1, int ones2, int ones_common, Metrics metrics, float& num, float& den);
        static float _numerator(int ones1, int ones2, int ones_common, Metrics metrics);
        static float _denominator(int ones1, int ones2, int ones_common, Metrics metrics);
    };

    class MangoTautomer
    {
    public:
        struct Params
        {
            int conditions;
            bool force_hydrogens;
            bool ring_chain;
            bool substructure;
        };

        MangoTautomer(BingoContext& context);

        void loadQuery(const Array<char>& buf);
        void loadQuery(Scanner& scanner);
        void loadQuery(const char* str);

        void setParams(int conditions, bool force_hydrogens, bool ring_chain, bool substructure);
        void setParameters(const char* conditions);

        const char* getQueryGross();
        const byte* getQueryFingerprint();

        void loadTarget(Scanner& scanner);
        void loadTarget(const Array<char>& molfile_buf);
        void loadTarget(const char* target);

        bool matchLoadedTarget();

        bool matchBinary(const Array<char>& target_buf);
        bool matchBinary(Scanner& scanner);

        void getHighlightedTarget(Array<char>& molfile_buf);

        bool preserve_bonds_on_highlighting;

        DECL_ERROR;

        Params _params;

        bool parseSub(const char* params);
        bool parseExact(const char* params);

    protected:
        BingoContext& _context;

        std::unique_ptr<BaseMolecule> _query;
        Molecule _target;
        Array<char> _query_gross_str;
        Array<byte> _query_fp;
        bool _query_data_valid;
        Array<int> _target_bond_types;

        void _validateQueryData();

        void _initTarget(bool from_database);
    };

    class MangoGross
    {
    public:
        MangoGross(BingoContext& context);

        // query like '> C6 H6'
        void parseQuery(const char* query);
        void parseQuery(const Array<char>& query);
        void parseQuery(Scanner& scanner);

        // SLQ conditions like 'CNT_C > 6 AND CNT_H > 6'
        const char* getConditions();

        bool checkGross(const Array<int>& target_gross);
        bool checkGross(const char* target_gross_str);

        bool checkMolecule(const Array<char>& target_buf);
        bool checkMolecule(Scanner& scanner);

        DECL_ERROR;

    protected:
        BingoContext& _context;

        Array<int> _query_gross;
        int _sign;
        Array<char> _conditions;
    };

    class MangoMass
    {
    public:
        // Mass bounds
        float top, bottom;
    };

} // namespace indigo

#endif
