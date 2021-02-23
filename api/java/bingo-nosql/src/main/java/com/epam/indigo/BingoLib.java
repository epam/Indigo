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

package com.epam.indigo;

import com.sun.jna.Library;
import com.sun.jna.ptr.FloatByReference;

public interface BingoLib extends Library {
    int bingoCreateDatabaseFile(String location, String type, String options);

    int bingoLoadDatabaseFile(String location, String options);

    int bingoCloseDatabase(int db);

    int bingoInsertRecordObj(int db, int obj);

    int bingoInsertRecordObjWithId(int db, int obj, int id);

    int bingoInsertRecordObjWithExtFP(int db, int obj, int ext_fp);

    int bingoInsertRecordObjWithIdAndExtFP(int db, int obj, int ext_fp, int id);

    int bingoDeleteRecord(int db, int index);

    int bingoOptimize(int db);

    int bingoSearchSub(int db, int query_obj, String options);

    int bingoSearchSim(int db, int query_obj, float min, float max, String options);

    int bingoSearchSimWithExtFP(int db, int query_obj, float min, float max, int ext_fp, String options);

    int bingoSearchSimTopN(int db, int query_obj, int limit, float minSim, String options);

    int bingoSearchSimTopNWithExtFP(int db, int query_obj, int limit, float minSim, int ext_fp, String options);

    int bingoSearchExact(int db, int query_obj, String options);

    int bingoSearchMolFormula(int db, String query, String options);

    int bingoEnumerateId(int db);

    int bingoNext(int search_obj);

    int bingoGetCurrentId(int search_obj);

    float bingoGetCurrentSimilarityValue(int search_obj);

    int bingoEstimateRemainingResultsCount(int search_obj);

    int bingoEstimateRemainingResultsCountError(int search_obj);

    int bingoEstimateRemainingTime(int search_obj, FloatByReference time_sec);

    int bingoGetObject(int search_obj);

    int bingoEndSearch(int search_obj);

    int bingoGetRecordObj(int db, int obj_id);

    String bingoVersion();
}
