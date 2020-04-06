using System;
using System.Runtime.InteropServices;
using System.Security;

#pragma warning disable 1591

namespace com.epam.indigo
{
    public unsafe class BingoLib
    {
        static BingoLib()
        {
            if (System.Environment.OSVersion.Platform == System.PlatformID.Win32NT)
            {
                if (System.Environment.Is64BitProcess)
                {
                    IndigoNativeLibraryLoader.LoadLibrary("lib/Win/x64/bingo.dll", true);
                }
                else
                {
                    IndigoNativeLibraryLoader.LoadLibrary("lib/Win/x86/bingo.dll", true);
                }
            }
            else if (System.Environment.OSVersion.Platform == System.PlatformID.Win32NT)
            {
                if (IndigoNativeLibraryLoader.isMac())
                {
                    IndigoNativeLibraryLoader.LoadLibrary("lib/Mac/10.7/bingo.dylib", true);
                }
                else
                {
                    if (Environment.Is64BitProcess)
                    {
                        IndigoNativeLibraryLoader.LoadLibrary("lib/Linux/x64/bingo.dylib", true);
                    }
                    else
                    {
                        IndigoNativeLibraryLoader.LoadLibrary("lib/Linux/x86/bingo.dylibo", true);
                    }
                }
            }
            else
            {
                throw new PlatformNotSupportedException();
            }
        }

        [DllImport("bingo"), SuppressUnmanagedCodeSecurity]
        public static extern int bingoCreateDatabaseFile(string location, string type, string options);

        [DllImport("bingo"), SuppressUnmanagedCodeSecurity]
        public static extern int bingoLoadDatabaseFile(string location, string options);

        [DllImport("bingo"), SuppressUnmanagedCodeSecurity]
        public static extern int bingoCloseDatabase(int db);


        [DllImport("bingo"), SuppressUnmanagedCodeSecurity]
        public static extern int bingoInsertRecordObj(int db, int obj);

        [DllImport("bingo"), SuppressUnmanagedCodeSecurity]
        public static extern int bingoInsertRecordObjWithId(int db, int obj, int id);

        [DllImport("bingo"), SuppressUnmanagedCodeSecurity]
        public static extern int bingoInsertRecordObjWithExtFP(int db, int obj, int ext_fp);

        [DllImport("bingo"), SuppressUnmanagedCodeSecurity]
        public static extern int bingoInsertRecordObjWithIdAndExtFP(int db, int obj, int ext_fp, int id);

        [DllImport("bingo"), SuppressUnmanagedCodeSecurity]
        public static extern int bingoDeleteRecord(int db, int index);

        [DllImport("bingo"), SuppressUnmanagedCodeSecurity]
        public static extern int bingoOptimize(int db);

        [DllImport("bingo"), SuppressUnmanagedCodeSecurity]
        public static extern int bingoSearchSub(int db, int query_obj, string options);

        [DllImport("bingo"), SuppressUnmanagedCodeSecurity]
        public static extern int bingoSearchSim(int db, int query_obj, float min, float max, string options);

        [DllImport("bingo"), SuppressUnmanagedCodeSecurity]
        public static extern int bingoSearchSimWithExtFP(int db, int query_obj, float min, float max, int ext_fp, string options);

        [DllImport("bingo"), SuppressUnmanagedCodeSecurity]
        public static extern int bingoSearchSimTopN(int db, int query_obj, int limit, float minSim, string options);

        [DllImport("bingo"), SuppressUnmanagedCodeSecurity]
        public static extern int bingoSearchSimTopNWithExtFP(int db, int query_obj, int limit, float minSim, int ext_fp, string options);

        [DllImport("bingo"), SuppressUnmanagedCodeSecurity]
        public static extern int bingoSearchExact(int db, int query_obj, string options);

        [DllImport("bingo"), SuppressUnmanagedCodeSecurity]
        public static extern int bingoSearchMolFormula(int db, string query, string options);

        [DllImport("bingo"), SuppressUnmanagedCodeSecurity]
        public static extern int bingoEnumerateId(int db);

        [DllImport("bingo"), SuppressUnmanagedCodeSecurity]
        public static extern int bingoNext(int search_obj);

        [DllImport("bingo"), SuppressUnmanagedCodeSecurity]
        public static extern int bingoGetCurrentId(int search_obj);

        [DllImport("bingo"), SuppressUnmanagedCodeSecurity]
        public static extern float bingoGetCurrentSimilarityValue(int search_obj);

        [DllImport("bingo"), SuppressUnmanagedCodeSecurity]
        public static extern int bingoEstimateRemainingResultsCount(int search_obj);

        [DllImport("bingo"), SuppressUnmanagedCodeSecurity]
        public static extern int bingoEstimateRemainingResultsCountError(int search_obj);

        [DllImport("bingo"), SuppressUnmanagedCodeSecurity]
        public static extern int bingoEstimateRemainingTime(int search_obj, float* time_sec);

        [DllImport("bingo"), SuppressUnmanagedCodeSecurity]
        public static extern int bingoGetObject(int search_obj);

        [DllImport("bingo"), SuppressUnmanagedCodeSecurity]
        public static extern int bingoEndSearch(int search_obj);

        [DllImport("bingo"), SuppressUnmanagedCodeSecurity]
        public static extern int bingoGetRecordObj(int db, int obj_id);

        [DllImport("bingo"), SuppressUnmanagedCodeSecurity]
        public static extern sbyte* bingoVersion();
    }
}
