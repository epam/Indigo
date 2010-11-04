using System;
using System.IO;
using System.Data;
using System.Collections.Generic;
using Microsoft.SqlServer.Server;
using System.Data.SqlTypes;
using System.Data.SqlClient;
using System.Text;
using indigo.SqlAttributes;

namespace indigo
{
   public class BingoSqlUtils
   {
      public static void ExecNonQuery (SqlConnection conn, string command, params object[] args)
      {
         string query = String.Format(command, args);
         using (SqlCommand cmd = new SqlCommand(query, conn))
         {
            cmd.CommandTimeout = 3600 * 10;
            cmd.ExecuteNonQuery();
         }
      }

      public static void ExecNonQueryNoThrow (SqlConnection conn, string command, params object[] args)
      {
         try
         {
            ExecNonQuery(conn, command, args);
         }
         catch (SqlException)
         {
         }
         catch (Exception ex)
         {
            BingoLog.logMessage("Excetion in : ");
            BingoLog.logMessage("Exception in ExecNonQueryNoThrow: {0} in {1}:\n{2}", 
               ex.Message, ex.Source, ex.StackTrace);
         }
      }

      public static object ExecObjQuery (SqlConnection conn, string command, params object[] args)
      {
         using (SqlCommand cmd = new SqlCommand(String.Format(command, args), conn))
         {
            cmd.CommandTimeout = 3600 * 10;
            object obj = cmd.ExecuteScalar();
            if (obj == DBNull.Value)
               return null;
            return obj;
         }
      }

      public static int? ExecIntQuery (SqlConnection conn, string command, params object[] args)
      {
         object obj = ExecObjQuery(conn, command, args);
         if (obj == null)
            return null;
         return Convert.ToInt32(obj);
      }

      public static string ExecStringQuery (SqlConnection conn, string command, params object[] args)
      {
         object obj = ExecObjQuery(conn, command, args);
         if (obj == null)
            return null;
         return Convert.ToString(obj);
      }

      public static List<object> ExecQuery (SqlConnection conn, string command, params object[] args)
      {
         using (SqlCommand cmd = new SqlCommand(String.Format(command, args), conn))
         {
            cmd.CommandTimeout = 3600;
            using (SqlDataReader reader = cmd.ExecuteReader())
            {
               if (!reader.Read())
                  return null;

               List<object> ret = new List<object>();
               for (int i = 0; i < reader.FieldCount; i++)
                  ret.Add(reader[i]);
               return ret;
            }
         }
      }

      [SqlFunction]
      [BingoSqlFunction]
      public static SqlString ReadFileAsText (SqlString filename)
      {
         return File.ReadAllText(filename.Value);
      }

      [SqlFunction]
      [BingoSqlFunction]
      public static SqlBinary ReadFileAsBinary (SqlString filename)
      {
         return File.ReadAllBytes(filename.Value);
      }

      [SqlFunction]
      [BingoSqlFunction(access_level = AccessLevelKind.None)]
      public static void _ForceGC ()
      {
         GC.Collect();
         GC.WaitForPendingFinalizers();
      }
   }

   public class BingoSqlCursor : IDisposable
   {
      string select_command;
      SqlConnection connection;
      string cursor_name = string.Format("[{0}]", Guid.NewGuid().ToString());
      bool closed = false;
      List<object> row;

      public BingoSqlCursor (SqlConnection conn, string selectCommand, params object[] args)
      {
         this.select_command = String.Format(selectCommand, args);
         this.connection = conn;

         BingoSqlUtils.ExecNonQuery(conn, 
            "DECLARE {0} CURSOR FORWARD_ONLY READ_ONLY FOR {1}; OPEN {0};",
            cursor_name, this.select_command);
      }

      public bool read ()
      {
         row = BingoSqlUtils.ExecQuery(connection, "FETCH NEXT FROM {0};", cursor_name);
         return row != null;
      }

      public object this[int index]
      {
         get { return row[index]; }
      }

      public void close ()
      {
         if (!closed)
         {
            BingoSqlUtils.ExecNonQuery(connection, "CLOSE {0}; DEALLOCATE {0};", cursor_name);
            closed = true;
         }
      }

      public void Dispose ()
      {
         close();
         GC.SuppressFinalize(this);
      }

      ~BingoSqlCursor ()
      {
         close();
      }
   }

   public class BingoLog   
   {
      static object sync_obj = new object();

      [SqlFunction]
      [BingoSqlFunction(access_level = AccessLevelKind.None)]
      public static void _WriteLog (SqlString message)
      {
         try 
         {
            logMessage(message.Value);
            SqlContext.Pipe.Send(String.Format(
               "Message '{0}' should be saved to '{1}'", message.Value, getLogFileName()));
         }
         catch (Exception ex)
         {
            SqlContext.Pipe.Send(String.Format(@"Error: Cannot save
               message '{0}' to '{1}'", message.Value, getLogFileName()));
            SqlContext.Pipe.Send(String.Format("Exception '{0}' in '{1}':\n{2}",
               ex.Message, ex.Source, ex.StackTrace));
         }
      }

      public static string getLogFileName ()
      {                  
         return System.IO.Path.GetTempPath() + "bingo_sql_server.log";
      }

      public static void logMessageWithThrow (string message, params object[] args)
      {
         lock (sync_obj)
         {
            try
            {
               using (StreamWriter sw = System.IO.File.AppendText(getLogFileName()))
               {
                  BingoSqlUtils._ForceGC(); 
                  
                  string logLine = String.Format(
                      "{0:G}: {1}", DateTime.Now, String.Format(message, args));

                  sw.WriteLine(logLine);
                  double size_mb = GC.GetTotalMemory(false) / 1000 / 1000.0;
                  sw.WriteLine(String.Format("GC Memory: {0:0.00} Mb", size_mb));
               }
            }
            catch
            {
            }
         }
      }

      public static void logMessage (string message, params object[] args)
      {
         try
         {
            logMessageWithThrow(message, args);
         }
         catch
         {
         }
      }
   }

   // Profiling timer
   public class BingoTimer
   {
      UInt64 _start;
      string _name;

      public BingoTimer (string name)
      {
         _start = BingoCore.bingoProfNanoClock();
         _name = name;
      }

      public void end ()
      {
         UInt64 end = BingoCore.bingoProfNanoClock();
         BingoCore.bingoProfIncTimer(_name, end - _start);
      }
   }
}
