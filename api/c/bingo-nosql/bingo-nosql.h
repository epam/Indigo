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

CEXPORT const char* bingoVersion();

// options = "id: <property-name>"
CEXPORT int bingoCreateDatabaseFile(const char* location, const char* type, const char* options);
CEXPORT int bingoLoadDatabaseFile(const char* location, const char* options);
CEXPORT int bingoCloseDatabase(int db);

//
// Record insertion/deletion
//
CEXPORT int bingoInsertRecordObj(int db, int obj);
CEXPORT int bingoInsertIteratorObj(int db, int iterator_obj_id);
CEXPORT int bingoInsertRecordObjWithId(int db, int obj, int id);
CEXPORT int bingoInsertRecordObjWithExtFP(int db, int obj, int fp);
CEXPORT int bingoInsertRecordObjWithIdAndExtFP(int db, int obj, int id, int fp);
CEXPORT int bingoDeleteRecord(int db, int id);
CEXPORT int bingoGetRecordObj(int db, int id);

CEXPORT int bingoOptimize(int db);

// Search methods that returns search object
// Search object is an iterator
CEXPORT int bingoSearchSub(int db, int query_obj, const char* options);
CEXPORT int bingoSearchExact(int db, int query_obj, const char* options);
CEXPORT int bingoSearchMolFormula(int db, const char* query, const char* options);
CEXPORT int bingoSearchSim(int db, int query_obj, float min, float max, const char* options);
CEXPORT int bingoSearchSimWithExtFP(int db, int query_obj, float min, float max, int fp, const char* options);

CEXPORT int bingoSearchSimTopN(int db, int query_obj, int limit, float min, const char* options);
CEXPORT int bingoSearchSimTopNWithExtFP(int db, int query_obj, int limit, float min, int fp, const char* options);

CEXPORT int bingoEnumerateId(int db);

//
// Search object methods
//
CEXPORT int bingoNext(int search_obj);
CEXPORT int bingoGetCurrentId(int search_obj);
CEXPORT float bingoGetCurrentSimilarityValue(int search_obj);

// Estimation methods
CEXPORT int bingoEstimateRemainingResultsCount(int search_obj);
CEXPORT int bingoEstimateRemainingResultsCountError(int search_obj);
CEXPORT int bingoEstimateRemainingTime(int search_obj, float* time_sec);
CEXPORT int bingoContainersCount(int search_obj);
CEXPORT int bingoCellsCount(int search_obj);
CEXPORT int bingoCurrentCell(int search_obj);
CEXPORT int bingoMinCell(int search_obj);
CEXPORT int bingoMaxCell(int search_obj);

// This method return IndigoObject that represents current object.
// After calling bingoNext this object automatically points to the next found result
CEXPORT int bingoGetObject(int search_obj);

CEXPORT int bingoEndSearch(int search_obj);

CEXPORT const char* bingoProfilingGetStatistics(int for_session);

#endif // __indigo_bingo__
