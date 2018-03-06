using System;
using System.Collections.Generic;
using System.Text;

#pragma warning disable 1591

namespace com.epam.indigo
{
    public unsafe interface BingoLib
    {
        int bingoCreateDatabaseFile(string location, string type, string options);
        int bingoLoadDatabaseFile(string location, string options);
        int bingoCloseDatabase(int db);

        int bingoInsertRecordObj (int db, int obj);
        int bingoInsertRecordObjWithId(int db, int obj, int id);
        int bingoInsertRecordObjWithExtFP (int db, int obj, int ext_fp);
        int bingoInsertRecordObjWithIdAndExtFP(int db, int obj, int ext_fp, int id);
        int bingoDeleteRecord (int db, int index);

        int bingoOptimize (int db);

        int bingoSearchSub (int db, int query_obj, string options);
        int bingoSearchSim (int db, int query_obj, float min, float max, string options);
        int bingoSearchSimWithExtFP (int db, int query_obj, float min, float max, int ext_fp, string options);
        int bingoSearchSimTopN(int db, int query_obj, int limit, float minSim, string options);
        int bingoSearchSimTopNWithExtFP(int db, int query_obj, int limit, float minSim, int ext_fp, string options);
        int bingoSearchExact (int db, int query_obj, string options);
        int bingoSearchMolFormula (int db, string query, string options);

        int bingoEnumerateId (int db);

        int bingoNext (int search_obj);
        int bingoGetCurrentId (int search_obj);
        float bingoGetCurrentSimilarityValue(int search_obj);

        int bingoEstimateRemainingResultsCount (int search_obj);
        int bingoEstimateRemainingResultsCountError (int search_obj);
        int bingoEstimateRemainingTime (int search_obj, float *time_sec);

        int bingoGetObject (int search_obj);
        int bingoEndSearch (int search_obj);

        int bingoGetRecordObj (int db, int obj_id);

        string bingoVersion();
    }
}
