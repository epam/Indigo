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

#ifndef __bingo_core_c_h___
#define __bingo_core_c_h___

#include "base_c/defs.h"
/*
 * Bingo common core functions
 */
CEXPORT const char* bingoGetVersion();
CEXPORT const char* bingoGetError();
CEXPORT const char* bingoGetWarning();
CEXPORT qword bingoAllocateSessionID();
CEXPORT void bingoReleaseSessionID(qword session_id);
CEXPORT void bingoSetSessionID(qword session_id);
CEXPORT qword bingoGetSessionID();
typedef void (*BINGO_ERROR_HANDLER)(const char* message, void* context);
CEXPORT void bingoSetErrorHandler(BINGO_ERROR_HANDLER handler, void* context);
CEXPORT int bingoSetContext(int id);
CEXPORT int bingoSetConfigInt(const char* name, int value);
CEXPORT int bingoGetConfigInt(const char* name, int* value);
CEXPORT int bingoGetConfigBin(const char* name, const char** value, int* len);
CEXPORT int bingoSetConfigBin(const char* name, const char* value, int len);
CEXPORT int bingoClearTautomerRules();
CEXPORT int bingoAddTautomerRule(int n, const char* beg, const char* end);
CEXPORT int bingoTautomerRulesReady(int n, const char* beg, const char* end);
/*
 * Returns number of parsed field mappings
 */
CEXPORT int bingoImportParseFieldList(const char* fields_str);
CEXPORT const char* bingoImportGetColumnName(int idx);
CEXPORT const char* bingoImportGetPropertyName(int idx);
/*
 * Get value by parsed field list
 */
CEXPORT const char* bingoImportGetPropertyValue(int idx);
/*
 * SDF import
 */
CEXPORT int bingoSDFImportOpen(const char* file_name);
CEXPORT int bingoSDFImportClose();
CEXPORT int bingoSDFImportEOF();
CEXPORT const char* bingoSDFImportGetNext();
CEXPORT const char* bingoSDFImportGetProperty(const char* param_name);
/*
 * RDF import
 */
CEXPORT int bingoRDFImportOpen(const char* file_name);
CEXPORT int bingoRDFImportClose();
CEXPORT int bingoRDFImportEOF();
CEXPORT const char* bingoRDFImportGetNext();
CEXPORT const char* bingoRDFImportGetProperty(const char* param_name);
/*
 * SMILES import
 */
CEXPORT int bingoSMILESImportOpen(const char* file_name);
CEXPORT int bingoSMILESImportClose();
CEXPORT int bingoSMILESImportEOF();
CEXPORT const char* bingoSMILESImportGetNext();
CEXPORT const char* bingoSMILESImportGetId();

CEXPORT void bingoProfilingReset(byte reset_whole_session);
CEXPORT const char* bingoProfilingGetStatistics(bool for_session);
CEXPORT float bingoProfilingGetTime(const char* counter_name, byte for_session);
CEXPORT qword bingoProfilingGetValue(const char* counter_name, byte for_session);
CEXPORT qword bingoProfilingGetCount(const char* counter_name, byte for_session);
CEXPORT int bingoCheckMemoryAllocate(int size);
CEXPORT int bingoCheckMemoryFree();
CEXPORT qword bingoProfNanoClock();
CEXPORT void bingoProfIncTimer(const char* name, qword dt);
CEXPORT void bingoProfIncCounter(const char* name, int dv);
CEXPORT const char* bingoGetNameCore(const char* target_buf, int target_buf_len);
CEXPORT int bingoSetIndexRecordData(int id, const char* data, int data_size);

CEXPORT int bingoIndexEnd();
CEXPORT int bingoIndexBegin();
CEXPORT int bingoIndexMarkTermintate();
CEXPORT int bingoIndexProcess(bool is_reaction, int (*get_next_record_cb)(void* context), void (*process_result_cb)(void* context),
                              void (*process_error_cb)(int id, void* context), void* context);

/*
 * Mango core interface
 */
CEXPORT int mangoIndexProcessSingleRecord();

CEXPORT int mangoIndexReadPreparedMolecule(int* id, const char** cmf_buf, int* cmf_buf_len, const char** xyz_buf, int* xyz_buf_len, const char** gross_str,
                                           const char** counter_elements_str, const char** fingerprint_buf, int* fingerprint_buf_len,
                                           const char** fingerprint_sim_str, float* mass, int* sim_fp_bits_count);

CEXPORT int mangoGetHash(bool for_index, int index, int* count, dword* hash);
CEXPORT int mangoGetAtomCount(const char* target_buf, int target_buf_len);
CEXPORT int mangoGetBondCount(const char* target_buf, int target_buf_len);
CEXPORT int mangoSetupMatch(const char* search_type, const char* query, const char* options);
CEXPORT int mangoSimilarityGetBitMinMaxBoundsArray(int count, int* target_ones, int** min_bound_ptr, int** max_bound_ptr);

CEXPORT int mangoSimilarityGetScore(float* score);
CEXPORT int mangoSimilaritySetMinMaxBounds(float min_bound, float max_bound);
// Return value:
//   1 if the query is a substructure of the taret
//   0 if it is not
//  -1 if something is bad with the target ("quiet" error)
//  -2 if some other thing is bad ("sound" error)
CEXPORT int mangoMatchTarget(const char* target, int target_buf_len);

// Return value:
//   1 if the query is a substructure of the taret
//   0 if it is not
//  -1 if something is bad with the target ("quiet" error)
//  -2 if some other thing is bad ("sound" error)
CEXPORT int mangoMatchTargetBinary(const char* target_bin, int target_bin_len, const char* target_xyz, int target_xyz_len);

CEXPORT int mangoLoadTargetBinaryXyz(const char* target_xyz, int target_xyz_len);
CEXPORT int mangoSetHightlightingMode(int enable);
CEXPORT const char* mangoGetHightlightedMolecule();
CEXPORT const char* mangoSMILES(const char* target_buf, int target_buf_len, int canonical);
CEXPORT const char* mangoMolfile(const char* molecule, int molecule_len);
CEXPORT const char* mangoCML(const char* molecule, int molecule_len);
CEXPORT int mangoGetQueryFingerprint(const char** query_fp, int* query_fp_len);
CEXPORT const char* mangoGetCountedElementName(int index);
CEXPORT int mangoNeedCoords();
CEXPORT byte mangoExactNeedComponentMatching();
CEXPORT const char* mangoTauGetQueryGross();
CEXPORT int mangoMass(const char* target_buf, int target_buf_len, const char* type, float* out);
CEXPORT int mangoMassD(const char* target_buf, int target_buf_len, const char* type, double* out);
CEXPORT const char* mangoGross(const char* target_buf, int target_buf_len);
CEXPORT const char* mangoGrossGetConditions();
CEXPORT const char* mangoCheckMolecule(const char* molecule, int molecule_len);
CEXPORT const char* mangoICM(const char* molecule, int molecule_len, bool save_xyz, int* out_len);
CEXPORT const char* mangoFingerprint(const char* molecule, int molecule_len, const char* options, int* out_len);
CEXPORT const char* mangoInChI(const char* molecule, int molecule_len, const char* options, int* out_len);
CEXPORT const char* mangoInChIKey(const char* inchi);
CEXPORT int mangoIndexProcess(int (*get_next_record_cb)(void* context), void (*process_result_cb)(void* context),
                              void (*process_error_cb)(int id, void* context), void* context);
CEXPORT const char* mangoStandardize(const char* molecule, int molecule_len, const char* options);

/*
 * Ringo core interface
 */

CEXPORT int ringoIndexProcessSingleRecord();

CEXPORT int ringoIndexReadPreparedReaction(int* id, const char** crf_buf, int* crf_buf_len, const char** fingerprint_buf, int* fingerprint_buf_len);

CEXPORT int ringoSetupMatch(const char* search_type, const char* query, const char* options);
// Return value:
//   1 if the query is a substructure of the taret
//   0 if it is not
//  -1 if something is bad with the target ("quiet" error)
//  -2 if some other thing is bad ("sound" error)
CEXPORT int ringoMatchTarget(const char* target, int target_buf_len);
// Return value:
//   1 if the query is a substructure of the taret
//   0 if it is not
//  -1 if something is bad with the target ("quiet" error)
//  -2 if some other thing is bad ("sound" error)
CEXPORT int ringoMatchTargetBinary(const char* target_bin, int target_bin_len);
CEXPORT const char* ringoRSMILES(const char* target_buf, int target_buf_len);
CEXPORT const char* ringoRxnfile(const char* reaction, int reaction_len);
CEXPORT const char* ringoRCML(const char* reaction, int reaction_len);
CEXPORT const char* ringoAAM(const char* reaction, int reaction_len, const char* mode);
CEXPORT const char* ringoCheckReaction(const char* reaction, int reaction_len);
CEXPORT int ringoGetQueryFingerprint(const char** query_fp, int* query_fp_len);
CEXPORT int ringoSetHightlightingMode(int enable);
CEXPORT const char* ringoGetHightlightedReaction();
CEXPORT const char* ringoICR(const char* reaction, int reaction_len, bool save_xyz, int* out_len);
CEXPORT int ringoGetHash(bool for_index, dword* hash);
CEXPORT const char* ringoFingerprint(const char* reaction, int reaction_len, const char* options, int* out_len);

#endif // __bingo_core_c_h___
