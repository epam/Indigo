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

/** Returns Bingo NoSQL library version string. */
CEXPORT const char* bingoVersion();

/** Creates a Bingo NoSQL database file at the given location. */
CEXPORT int bingoCreateDatabaseFile(const char* location, const char* type, const char* options);
/** Loads an existing Bingo NoSQL database file. */
CEXPORT int bingoLoadDatabaseFile(const char* location, const char* options);
/** Closes an opened Bingo NoSQL database handle. */
CEXPORT int bingoCloseDatabase(int db);

//
// Record insertion/deletion
//
/** Inserts an object into the specified database. */
CEXPORT int bingoInsertRecordObj(int db, int obj);
/** Inserts objects produced by the given iterator into the database. */
CEXPORT int bingoInsertIteratorObj(int db, int iterator_obj_id);
/** Inserts an object using the provided identifier. */
CEXPORT int bingoInsertRecordObjWithId(int db, int obj, int id);
/** Inserts an object with an external fingerprint. */
CEXPORT int bingoInsertRecordObjWithExtFP(int db, int obj, int fp);
/** Inserts an object with both identifier and external fingerprint. */
CEXPORT int bingoInsertRecordObjWithIdAndExtFP(int db, int obj, int id, int fp);
/** Deletes a record from the database by identifier. */
CEXPORT int bingoDeleteRecord(int db, int id);
/** Retrieves an object from the database by identifier. */
CEXPORT int bingoGetRecordObj(int db, int id);

/** Rebuilds indexes and optimizes the database. */
CEXPORT int bingoOptimize(int db);

// Search methods that returns search object
// Search object is an iterator
/** Creates a substructure search iterator. */
CEXPORT int bingoSearchSub(int db, int query_obj, const char* options);
/** Creates an exact match search iterator. */
CEXPORT int bingoSearchExact(int db, int query_obj, const char* options);
/** Creates a molecular formula search iterator. */
CEXPORT int bingoSearchMolFormula(int db, const char* query, const char* options);
/** Creates a similarity search iterator for the given object. */
CEXPORT int bingoSearchSim(int db, int query_obj, float min, float max, const char* options);
/** Creates a similarity search iterator using an external fingerprint. */
CEXPORT int bingoSearchSimWithExtFP(int db, int query_obj, float min, float max, int fp, const char* options);

/** Finds top-N most similar records for the given object. */
CEXPORT int bingoSearchSimTopN(int db, int query_obj, int limit, float min, const char* options);
/** Finds top-N most similar records using an external fingerprint. */
CEXPORT int bingoSearchSimTopNWithExtFP(int db, int query_obj, int limit, float min, int fp, const char* options);

/** Enumerates all identifiers in the database. */
CEXPORT int bingoEnumerateId(int db);

//
// Search object methods
//
/** Advances the search iterator to the next result. */
CEXPORT int bingoNext(int search_obj);
/** Returns identifier of the current search result. */
CEXPORT int bingoGetCurrentId(int search_obj);
/** Returns similarity value for the current search result. */
CEXPORT float bingoGetCurrentSimilarityValue(int search_obj);

// Estimation methods
/** Estimates number of remaining results for the search. */
CEXPORT int bingoEstimateRemainingResultsCount(int search_obj);
/** Estimates error for remaining results count. */
CEXPORT int bingoEstimateRemainingResultsCountError(int search_obj);
/** Estimates remaining search time in seconds. */
CEXPORT int bingoEstimateRemainingTime(int search_obj, float* time_sec);
/** Returns number of containers participating in search. */
CEXPORT int bingoContainersCount(int search_obj);
/** Returns number of cells participating in search. */
CEXPORT int bingoCellsCount(int search_obj);
/** Returns current cell number being processed. */
CEXPORT int bingoCurrentCell(int search_obj);
/** Returns minimal cell number participating in search. */
CEXPORT int bingoMinCell(int search_obj);
/** Returns maximal cell number participating in search. */
CEXPORT int bingoMaxCell(int search_obj);

// This method return IndigoObject that represents current object.
// After calling bingoNext this object automatically points to the next found result
/** Retrieves Indigo object for the current search result. */
CEXPORT int bingoGetObject(int search_obj);

/** Releases resources for the given search iterator. */
CEXPORT int bingoEndSearch(int search_obj);

/** Returns profiling statistics; pass non-zero to get session statistics. */
CEXPORT const char* bingoProfilingGetStatistics(int for_session);

#endif // __indigo_bingo__
