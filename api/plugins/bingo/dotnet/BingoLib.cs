using System;
using System.Collections.Generic;
using System.Text;

namespace com.ggasoftware.indigo
{
	public unsafe interface BingoLib
    {
		int bingoCreateDatabaseFile(sbyte* location, sbyte* type, sbyte* options);
		int bingoLoadDatabaseFile(sbyte* location, sbyte* type);
		int bingoDeleteDatabase(int db);
		int bingoInsertRecordObj (int db, int obj);
		int bingoDeleteRecord (int db, int index);
		int bingoSearchSub (int db, int query_obj, sbyte* options);
		int bingoSearchSim (int db, int query_obj, float min, float max, sbyte* options);
		
		int bingoNext (int search_obj);
		int bingoGetCurrentIndex (int search_obj);
		int bingoGetObject (int search_obj);
		int bingoEndSearch (int search_obj);
    }
}
