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

#ifndef __indigo_bingo__
#define __indigo_bingo__

#include "indigo.h"

/** Returns the Bingo NoSQL library version string. */
CEXPORT const char* bingoVersion();

/**
 * Creates a new Bingo NoSQL database file.
 * \param location Path to the database storage.
 * \param type Database type (for example, "molecule" or "reaction").
 * \param options Additional options, e.g. "id:<property-name>".
 * \return Database handle on success or -1 on error.
 */
CEXPORT int bingoCreateDatabaseFile(const char* location, const char* type, const char* options);

/**
 * Loads an existing Bingo NoSQL database.
 * \param location Path to the database storage.
 * \param options Additional options string.
 * \return Database handle on success or -1 on error.
 */
CEXPORT int bingoLoadDatabaseFile(const char* location, const char* options);

/**
 * Closes a previously opened Bingo database handle.
 * \param db Database handle returned by open/create functions.
 * \return 1 on success or -1 on error.
 */
CEXPORT int bingoCloseDatabase(int db);

//
// Record insertion/deletion
//

/**
 * Inserts an Indigo object using its id property if present.
 * \param db Database handle.
 * \param obj Indigo object handle (molecule or reaction).
 * \return Assigned record id on success or -1 on error.
 */
CEXPORT int bingoInsertRecordObj(int db, int obj);

/**
 * Inserts all items from an Indigo iterator into the database.
 * \param db Database handle.
 * \param iterator_obj_id Indigo iterator handle.
 * \return Last inserted id or -1 on error.
 */
CEXPORT int bingoInsertIteratorObj(int db, int iterator_obj_id);

/**
 * Inserts an Indigo object with an explicit record id.
 * \param db Database handle.
 * \param obj Indigo object handle.
 * \param id Record identifier to store with the object.
 * \return Record id on success or -1 on error.
 */
CEXPORT int bingoInsertRecordObjWithId(int db, int obj, int id);

/**
 * Inserts an Indigo object with an external fingerprint.
 * \param db Database handle.
 * \param obj Indigo object handle.
 * \param fp Indigo fingerprint object handle.
 * \return Record id on success or -1 on error.
 */
CEXPORT int bingoInsertRecordObjWithExtFP(int db, int obj, int fp);

/**
 * Inserts an Indigo object with both explicit id and external fingerprint.
 * \param db Database handle.
 * \param obj Indigo object handle.
 * \param id Record identifier to assign.
 * \param fp Indigo fingerprint object handle.
 * \return Record id on success or -1 on error.
 */
CEXPORT int bingoInsertRecordObjWithIdAndExtFP(int db, int obj, int id, int fp);

/**
 * Removes a record from the database.
 * \param db Database handle.
 * \param id Record identifier.
 * \return Removed record id on success or -1 on error.
 */
CEXPORT int bingoDeleteRecord(int db, int id);

/**
 * Retrieves a record as an Indigo object.
 * \param db Database handle.
 * \param id Record identifier.
 * \return Indigo object handle on success or -1 on error.
 */
CEXPORT int bingoGetRecordObj(int db, int id);

/**
 * Optimizes internal database structures.
 * \param db Database handle.
 * \return 0 when optimization completes successfully or -1 on error.
 */
CEXPORT int bingoOptimize(int db);

// Search methods that returns search object
// Search object is an iterator

/**
 * Performs substructure search using the given query object.
 * \param db Database handle.
 * \param query_obj Indigo query molecule or reaction handle.
 * \param options Search options string.
 * \return Search iterator handle on success or -1 on error.
 */
CEXPORT int bingoSearchSub(int db, int query_obj, const char* options);

/**
 * Performs exact match search.
 * \param db Database handle.
 * \param query_obj Indigo molecule or reaction handle.
 * \param options Search options string.
 * \return Search iterator handle on success or -1 on error.
 */
CEXPORT int bingoSearchExact(int db, int query_obj, const char* options);

/**
 * Performs molecular formula search.
 * \param db Database handle.
 * \param query Molecular formula string.
 * \param options Search options string.
 * \return Search iterator handle on success or -1 on error.
 */
CEXPORT int bingoSearchMolFormula(int db, const char* query, const char* options);

/**
 * Performs similarity search with score bounds.
 * \param db Database handle.
 * \param query_obj Indigo query molecule or reaction handle.
 * \param min Minimal similarity threshold.
 * \param max Maximal similarity threshold.
 * \param options Search options string.
 * \return Search iterator handle on success or -1 on error.
 */
CEXPORT int bingoSearchSim(int db, int query_obj, float min, float max, const char* options);

/**
 * Performs similarity search using an external fingerprint.
 * \param db Database handle.
 * \param query_obj Indigo query molecule or reaction handle.
 * \param min Minimal similarity threshold.
 * \param max Maximal similarity threshold.
 * \param fp External fingerprint object handle.
 * \param options Search options string.
 * \return Search iterator handle on success or -1 on error.
 */
CEXPORT int bingoSearchSimWithExtFP(int db, int query_obj, float min, float max, int fp, const char* options);

/**
 * Returns the top-N most similar records.
 * \param db Database handle.
 * \param query_obj Indigo query molecule or reaction handle.
 * \param limit Maximal number of results to return.
 * \param min Minimal similarity threshold.
 * \param options Search options string.
 * \return Search iterator handle on success or -1 on error.
 */
CEXPORT int bingoSearchSimTopN(int db, int query_obj, int limit, float min, const char* options);

/**
 * Returns the top-N most similar records using an external fingerprint.
 * \param db Database handle.
 * \param query_obj Indigo query molecule or reaction handle.
 * \param limit Maximal number of results to return.
 * \param min Minimal similarity threshold.
 * \param fp External fingerprint object handle.
 * \param options Search options string.
 * \return Search iterator handle on success or -1 on error.
 */
CEXPORT int bingoSearchSimTopNWithExtFP(int db, int query_obj, int limit, float min, int fp, const char* options);

/**
 * Creates an iterator that enumerates all record identifiers.
 * \param db Database handle.
 * \return Search iterator handle on success or -1 on error.
 */
CEXPORT int bingoEnumerateId(int db);

//
// Search object methods
//

/**
 * Advances the search iterator to the next result.
 * \param search_obj Search iterator handle.
 * \return Non-zero when positioned on a result, 0 when finished, or -1 on error.
 */
CEXPORT int bingoNext(int search_obj);

/**
 * Returns the record id for the current search result.
 * \param search_obj Search iterator handle.
 * \return Record identifier or -1 on error.
 */
CEXPORT int bingoGetCurrentId(int search_obj);

/**
 * Returns the similarity value for the current result.
 * \param search_obj Search iterator handle.
 * \return Similarity score or -1 on error.
 */
CEXPORT float bingoGetCurrentSimilarityValue(int search_obj);

// Estimation methods

/**
 * Estimates the number of remaining results in the current search.
 * \param search_obj Search iterator handle.
 * \return Estimated number of remaining results or -1 on error.
 */
CEXPORT int bingoEstimateRemainingResultsCount(int search_obj);

/**
 * Returns an error estimate for the remaining results count.
 * \param search_obj Search iterator handle.
 * \return Estimated error value or -1 on error.
 */
CEXPORT int bingoEstimateRemainingResultsCountError(int search_obj);

/**
 * Estimates remaining search time.
 * \param search_obj Search iterator handle.
 * \param time_sec Pointer to a float that will be updated with the estimated seconds (must not be NULL).
 * \return 1 on success or -1 on error.
 */
CEXPORT int bingoEstimateRemainingTime(int search_obj, float* time_sec);

/**
 * Returns the number of containers involved in the current search.
 * \param search_obj Search iterator handle.
 * \return Container count or -1 on error.
 */
CEXPORT int bingoContainersCount(int search_obj);

/**
 * Returns the number of cells in the current search.
 * \param search_obj Search iterator handle.
 * \return Cells count or -1 on error.
 */
CEXPORT int bingoCellsCount(int search_obj);

/**
 * Returns the index of the current cell being processed.
 * \param search_obj Search iterator handle.
 * \return Cell index or -1 on error.
 */
CEXPORT int bingoCurrentCell(int search_obj);

/**
 * Returns the minimal cell index for the search.
 * \param search_obj Search iterator handle.
 * \return Minimal cell index or -1 on error.
 */
CEXPORT int bingoMinCell(int search_obj);

/**
 * Returns the maximal cell index for the search.
 * \param search_obj Search iterator handle.
 * \return Maximal cell index or -1 on error.
 */
CEXPORT int bingoMaxCell(int search_obj);

// This method returns IndigoObject that represents current object.
// After calling bingoNext this object automatically points to the next found result

/**
 * Retrieves the current search result as an Indigo object.
 * \param search_obj Search iterator handle.
 * \return Indigo object handle or -1 on error.
 */
CEXPORT int bingoGetObject(int search_obj);

/**
 * Releases a search iterator and its resources.
 * \param search_obj Search iterator handle.
 * \return 1 on success or -1 on error.
 */
CEXPORT int bingoEndSearch(int search_obj);

/**
 * Returns profiling statistics as a string.
 * \param for_session Flag indicating whether to collect per-session statistics.
 * \return Pointer to a null-terminated statistics string managed by the library (do not free, copy if persistence is needed) or NULL on error.
 */
CEXPORT const char* bingoProfilingGetStatistics(int for_session);

#endif // __indigo_bingo__
