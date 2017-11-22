/****************************************************************************
 * Copyright (C) 2013 EPAM Systems
 *
 * This file is part of Indigo toolkit.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

package com.epam.indigo;

import com.sun.jna.Library;
import com.sun.jna.ptr.FloatByReference;

public interface BingoLib extends Library
{
        int bingoCreateDatabaseFile(String location, String type, String options);
        int bingoLoadDatabaseFile(String location, String options);
        int bingoCloseDatabase(int db);

        int bingoInsertRecordObj (int db, int obj);
        int bingoInsertRecordObjWithId(int db, int obj, int id);
        int bingoInsertRecordObjWithExtFP (int db, int obj, int ext_fp);
        int bingoInsertRecordObjWithIdAndExtFP(int db, int obj, int ext_fp, int id);
        int bingoDeleteRecord (int db, int index);

        int bingoOptimize (int db);

        int bingoSearchSub (int db, int query_obj, String options);
        int bingoSearchSim (int db, int query_obj, float min, float max, String options);
        int bingoSearchSimWithExtFP (int db, int query_obj, float min, float max, int ext_fp, String options);
        int bingoSearchExact (int db, int query_obj, String options);
        int bingoSearchMolFormula (int db, String query, String options);

        int bingoEnumerateId (int db);

        int bingoNext (int search_obj);
        int bingoGetCurrentId (int search_obj);
        float bingoGetCurrentSimilarityValue(int search_obj);

        int bingoEstimateRemainingResultsCount (int search_obj);
        int bingoEstimateRemainingResultsCountError (int search_obj);
        int bingoEstimateRemainingTime (int search_obj, FloatByReference time_sec);

        int bingoGetObject (int search_obj);
        int bingoEndSearch (int search_obj);

        int bingoGetRecordObj (int db, int obj_id);

        String bingoVersion();
}