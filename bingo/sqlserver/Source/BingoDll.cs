using System;
using System.Runtime.InteropServices;
using System.Data;
using Microsoft.SqlServer.Server;
using System.Data.SqlTypes;
using System.Data.SqlClient;
using System.Collections;
using System.IO;
using System.Text;
using Microsoft.Win32;
using indigo.SqlAttributes;

namespace indigo
{
   public class BingoDll
   {
      [DllImport("kernel32.dll", CharSet = CharSet.Auto)]
      static extern IntPtr LoadLibrary(string lpFileName);

      [DllImport("kernel32.dll", CharSet = CharSet.Auto)]
      static extern int FreeLibrary(IntPtr module);

      IntPtr _bingo_core_handle = IntPtr.Zero;

      ~BingoDll()
      {
         // Unload library on destructor
         BingoLog.logMessage("Unloading bingo core dll library...");
         _unload();
         BingoLog.logMessage("  Done.");
      }

      private void _load(SqlString bingo_schema)
      {
         if (_bingo_core_handle != IntPtr.Zero)
            return;

         BingoLog.logMessage("Loading bingo core dll library...");
         // Double lock for library loading
         lock (this)
         {
            if (_bingo_core_handle != IntPtr.Zero)
               return;

            String dll_name = @"\lib32\bingo-core-c.dll";

            if (IntPtr.Size == 8)
               dll_name = @"\lib64\bingo-core-c.dll";

            using (SqlConnection connection = new SqlConnection("context connection=true"))
            {
               connection.Open();

               SqlCommand cmd = new SqlCommand();
               cmd.Connection = connection;
               cmd.CommandTimeout = 3600;

               cmd.CommandText =
                  String.Format(@"select value from {0}.CONFIG 
                  where name='DLL_DIRECTORY' and n=0", bingo_schema);
               cmd.CommandType = CommandType.Text;
               object result = cmd.ExecuteScalar();
               _bingo_core_handle = LoadLibrary(Convert.ToString(result) + dll_name);
            }
         }
         BingoLog.logMessage("  Done.");
      }

      private void _unload ()
      {
         if (_bingo_core_handle != IntPtr.Zero)
         {
            // After FreeLibrary is called only once 
            // bingo-core-c.dll is still been used by SQL due to DllImport
            FreeLibrary(_bingo_core_handle);
            FreeLibrary(_bingo_core_handle);
            _bingo_core_handle = IntPtr.Zero;
            return;
         }
      }

      // Automatic bingo dll loading and unloading on assembly unloading
      private static BingoDll bingo_dll = new BingoDll();
      public static void load(SqlString bingo_schema)
      {
         bingo_dll._load(bingo_schema);
      }
      public static void unload()
      {
         bingo_dll._unload();
      }

      [SqlFunction]
      [BingoSqlFunction(access_level=AccessLevelKind.None)]
      public static void _UnloadLibrary()
      {
         BingoDll.unload();
      }
   }
}
