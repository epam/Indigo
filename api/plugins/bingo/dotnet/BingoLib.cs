using System;
using System.Collections.Generic;
using System.Text;

#pragma warning disable 1591

namespace com.ggasoftware.indigo
{
	public unsafe interface BingoLib
    {
		int bingoCreateDatabaseFile(string location, string type, string options);
		int bingoLoadDatabaseFile(string location, string type, string options);
		int bingoCloseDatabase(int db);
		int bingoInsertRecordObj (int db, int obj);
        int bingoInsertRecordObjWithId(int db, int obj, int id);
		int bingoDeleteRecord (int db, int index);
		int bingoSearchSub (int db, int query_obj, string options);
		int bingoSearchSim (int db, int query_obj, float min, float max, string options);

		int bingoNext (int search_obj);
		int bingoGetCurrentId (int search_obj);
		int bingoGetObject (int search_obj);
		int bingoEndSearch (int search_obj);
    }
}
