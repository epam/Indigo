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

#ifndef __bingo_core_c_internal_h___
#define __bingo_core_c_internal_h___

#include "base_c/defs.h"
#include <memory>
#include "core/bingo_context.h"
#include "core/mango_context.h"
#include "core/ringo_context.h"

#include "core/bingo_error.h"
#include "core/mango_matchers.h"

#include "base_cpp/scanner.h"
#include "gzip/gzip_scanner.h"
#include "molecule/cmf_saver.h"
#include "molecule/elements.h"
#include "molecule/molecule_auto_loader.h"
#include "molecule/molfile_loader.h"
#include "molecule/rdf_loader.h"
#include "molecule/sdf_loader.h"
#include "molecule/smiles_loader.h"
#include "reaction/reaction.h"
#include "reaction/reaction_auto_loader.h"
#include "reaction/rsmiles_loader.h"
#include "reaction/rxnfile_loader.h"

#include "bingo_core_c.h"
#include "bingo_core_c_parallel.h"

namespace indigo
{
    namespace bingo_core
    {

        class BingoCore
        {
        public:
            BingoCore();
            void reset();

            static BingoCore& getInstance();
            int mangoSetupMatch(const char* search_type, const char* query, const char* options);
            int mangoMassD(const char* target_buf, int target_buf_len, const char* type, double* out);
            int mangoMatchTarget(const char* target, int target_buf_len);
            int mangoMatchTargetBinary(const char* target_bin, int target_bin_len, const char* target_xyz, int target_xyz_len);
            int mangoNeedCoords();
            int mangoIndexProcessSingleRecord();
            int mangoIndexReadPreparedMolecule(int* id, const char** cmf_buf, int* cmf_buf_len, const char** xyz_buf, int* xyz_buf_len, const char** gross_str,
                                           const char** counter_elements_str, const char** fingerprint_buf, int* fingerprint_buf_len,
                                           const char** fingerprint_sim_str, float* mass,
                                           int* sim_fp_bits_count);
            int mangoGetHash(bool for_index, int index, int* count, dword* hash);
            void mangoGetQueryFingerprint(const char** query_fp, int* query_fp_len);
            byte mangoExactNeedComponentMatching();
            const char* mangoTauGetQueryGross();
            const char* mangoGrossGetConditions();
            void mangoSimilarityGetBitMinMaxBoundsArray(int count, int* target_ones, int** min_bound_ptr, int** max_bound_ptr);
            void mangoSimilaritySetMinMaxBounds(float min_bound, float max_bound);
            int ringoIndexProcessSingleRecord();
            int ringoIndexReadPreparedReaction(int* id, const char** crf_buf, int* crf_buf_len, const char** fingerprint_buf, int* fingerprint_buf_len);
            int ringoGetHash(bool for_index, dword* hash);
            int bingoSDFImportOpen(const char* file_name);
            int bingoSDFImportClose();
            int bingoSDFImportEOF();
            const char* bingoSDFImportGetNext();
            const char* bingoImportGetPropertyValue(int idx);
            int bingoSetConfigInt(const char* name, int value);
            int bingoSetConfigBin(const char* name, const char* value, int len);
            int bingoGetConfigBin(const char* name, const char** value, int* len);
            int bingoGetConfigInt(const char* name, int* value);
            int bingoAddTautomerRule(int n, const char* beg, const char* end);
            void bingoSetIndexRecordData(int id, const char* data, int data_size);
            int bingoTautomerRulesReady(int n, const char* beg, const char* end);
            int bingoIndexBegin();
            int bingoIndexEnd();
            int bingoImportParseFieldList(const char* fields_str);
            void bingoIndexProcess(bool is_reaction, int (*get_next_record_cb)(void* context), void (*process_result_cb)(void* context),
                              void (*process_error_cb)(int id, void* context), void* context);
            int getTimeout();
            BingoCore& self;

            Array<char> error;
            Array<char> warning;

            BINGO_ERROR_HANDLER error_handler;
            void* error_handler_context;

            BingoContext* bingo_context;
            MangoContext* mango_context;
            RingoContext* ringo_context;

            Obj<StringPool> import_properties;
            Obj<StringPool> import_columns;

            Obj<FileScanner> file_scanner;
            Obj<SdfLoader> sdf_loader;
            Obj<RdfLoader> rdf_loader;

            std::unique_ptr<GZipScanner> gz_scanner;
            Scanner* smiles_scanner;

            Array<char> buffer;

            MangoIndex* mango_index;
            RingoIndex* ringo_index;

            Obj<Array<char>> index_record_data;
            int index_record_data_id;

            Obj<MangoIndex> single_mango_index;
            Obj<RingoIndex> single_ringo_index;

            std::unique_ptr<IndexingDispatcher> parallel_indexing_dispatcher;

            bool skip_calculate_fp;

            enum
            {
                _UNDEF,
                _SUBSTRUCTRE,
                _TAUTOMER,
                _EXACT,
                _SIMILARITY,
                _GROSS
            } mango_search_type,
                ringo_search_type;

            bool mango_search_type_non;

            int sub_screening_max_bits, sim_screening_pass_mark;

            byte* test_ptr;
        };
        

#define BINGO_BEGIN                                                                                                                                            \
    {                                                                                                                                                          \
        BingoCore& self = BingoCore::getInstance();                                                                                                            \
        try                                                                                                                                                    \
        {                                                                                                                                                      \
            self.error.clear();

#define BINGO_END(success, fail)                                                                                                                               \
    }                                                                                                                                                          \
    catch (Exception & ex)                                                                                                                                     \
    {                                                                                                                                                          \
        self.error.readString(ex.message(), true);                                                                                                             \
        if (self.error_handler != 0)                                                                                                                           \
            self.error_handler(ex.message(), self.error_handler_context);                                                                                      \
        return fail;                                                                                                                                           \
    }                                                                                                                                                          \
    return success;                                                                                                                                            \
    }

#define BINGO_BEGIN_TIMEOUT                                                                                                                                    \
    {                                                                                                                                                          \
        BingoCore& self = BingoCore::getInstance();                                                                                                            \
        try                                                                                                                                                    \
        {                                                                                                                                                      \
            self.error.clear();                                                                                                                                \
            int timeout = self.getTimeout();                                                                                                                   \
            CancellationHandler* res = nullptr;                                                                                                                \
            if (timeout > 0)                                                                                                                                   \
            {                                                                                                                                                  \
                res = new TimeoutCancellationHandler(timeout);                                                                                                 \
            }                                                                                                                                                  \
            AutoCancellationHandler handler(res);

#define TRY_READ_TARGET_MOL                                                                                                                                    \
    try                                                                                                                                                        \
    {

#define CATCH_READ_TARGET_MOL(action)                                                                                                                          \
    }                                                                                                                                                          \
    catch (Scanner::Error & e)                                                                                                                                 \
    {                                                                                                                                                          \
        action;                                                                                                                                                \
    }                                                                                                                                                          \
    catch (MolfileLoader::Error & e)                                                                                                                           \
    {                                                                                                                                                          \
        action;                                                                                                                                                \
    }                                                                                                                                                          \
    catch (Element::Error & e)                                                                                                                                 \
    {                                                                                                                                                          \
        action;                                                                                                                                                \
    }                                                                                                                                                          \
    catch (Graph::Error & e)                                                                                                                                   \
    {                                                                                                                                                          \
        action;                                                                                                                                                \
    }                                                                                                                                                          \
    catch (MoleculeStereocenters::Error & e)                                                                                                                   \
    {                                                                                                                                                          \
        action;                                                                                                                                                \
    }                                                                                                                                                          \
    catch (MoleculeCisTrans::Error & e)                                                                                                                        \
    {                                                                                                                                                          \
        action;                                                                                                                                                \
    }                                                                                                                                                          \
    catch (SmilesLoader::Error & e)                                                                                                                            \
    {                                                                                                                                                          \
        action;                                                                                                                                                \
    }                                                                                                                                                          \
    catch (Molecule::Error & e)                                                                                                                                \
    {                                                                                                                                                          \
        action;                                                                                                                                                \
    }                                                                                                                                                          \
    catch (MoleculeAutoLoader::Error & e)                                                                                                                      \
    {                                                                                                                                                          \
        action;                                                                                                                                                \
    }                                                                                                                                                          \
    catch (EmbeddingEnumerator::TimeoutException & e)                                                                                                          \
    {                                                                                                                                                          \
        action;                                                                                                                                                \
    }                                                                                                                                                          \
    catch (DearomatizationsGroups::Error & e)                                                                                                                  \
    {                                                                                                                                                          \
        action;                                                                                                                                                \
    }                                                                                                                                                          \
    catch (MoleculePiSystemsMatcher::Error & e)                                                                                                                \
    {                                                                                                                                                          \
        action;                                                                                                                                                \
    }                                                                                                                                                          \
    catch (SkewSymmetricNetwork::Error & e)                                                                                                                    \
    {                                                                                                                                                          \
        action;                                                                                                                                                \
    }

//catch (IcmLoader::Error &e) { action;} \

#define TRY_READ_TARGET_RXN                                                                                                                                    \
    try                                                                                                                                                        \
    {

#define CATCH_READ_TARGET_RXN(action)                                                                                                                          \
    }                                                                                                                                                          \
    catch (Scanner::Error & e)                                                                                                                                 \
    {                                                                                                                                                          \
        action;                                                                                                                                                \
    }                                                                                                                                                          \
    catch (MolfileLoader::Error & e)                                                                                                                           \
    {                                                                                                                                                          \
        action;                                                                                                                                                \
    }                                                                                                                                                          \
    catch (RxnfileLoader::Error & e)                                                                                                                           \
    {                                                                                                                                                          \
        action;                                                                                                                                                \
    }                                                                                                                                                          \
    catch (Element::Error & e)                                                                                                                                 \
    {                                                                                                                                                          \
        action;                                                                                                                                                \
    }                                                                                                                                                          \
    catch (Graph::Error & e)                                                                                                                                   \
    {                                                                                                                                                          \
        action;                                                                                                                                                \
    }                                                                                                                                                          \
    catch (MoleculeStereocenters::Error & e)                                                                                                                   \
    {                                                                                                                                                          \
        action;                                                                                                                                                \
    }                                                                                                                                                          \
    catch (MoleculeCisTrans::Error & e)                                                                                                                        \
    {                                                                                                                                                          \
        action;                                                                                                                                                \
    }                                                                                                                                                          \
    catch (SmilesLoader::Error & e)                                                                                                                            \
    {                                                                                                                                                          \
        action;                                                                                                                                                \
    }                                                                                                                                                          \
    catch (RSmilesLoader::Error & e)                                                                                                                           \
    {                                                                                                                                                          \
        action;                                                                                                                                                \
    }                                                                                                                                                          \
    catch (Molecule::Error & e)                                                                                                                                \
    {                                                                                                                                                          \
        action;                                                                                                                                                \
    }                                                                                                                                                          \
    catch (Reaction::Error & e)                                                                                                                                \
    {                                                                                                                                                          \
        action;                                                                                                                                                \
    }                                                                                                                                                          \
    catch (ReactionAutoLoader::Error & e)                                                                                                                      \
    {                                                                                                                                                          \
        action;                                                                                                                                                \
    }                                                                                                                                                          \
    catch (EmbeddingEnumerator::TimeoutException & e)                                                                                                          \
    {                                                                                                                                                          \
        action;                                                                                                                                                \
    }                                                                                                                                                          \
    catch (DearomatizationsGroups::Error & e)                                                                                                                  \
    {                                                                                                                                                          \
        action;                                                                                                                                                \
    }                                                                                                                                                          \
    catch (MoleculePiSystemsMatcher::Error & e)                                                                                                                \
    {                                                                                                                                                          \
        action;                                                                                                                                                \
    }                                                                                                                                                          \
    catch (SkewSymmetricNetwork::Error & e)                                                                                                                    \
    {                                                                                                                                                          \
        action;                                                                                                                                                \
    }

    } // namespace bingo_core
} // namespace indigo

#endif // __bingo_core_c_h___
