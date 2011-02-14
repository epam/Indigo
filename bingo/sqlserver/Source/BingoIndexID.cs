using System;
using System.Collections.Generic;
using System.Text;
using System.Data.SqlClient;
using System.Data;
using System.Text.RegularExpressions;

namespace indigo
{
   public class BingoIndexID
   {
      public int object_id;
      public int database_id;

      private string name; // Name for logging

      // Detect database, schema, and table; some from table_name, some from SQL connection
      public BingoIndexID (SqlConnection connection, string table_name)
      {
         object_id = BingoSqlUtils.GetTableObjectID(connection, table_name);

         string database = null, schema = null, table = null;
         parseTableName(table_name, ref database, ref schema, ref table);

         if (database == null)
            database = connection.Database;
         database_id = BingoSqlUtils.GetDatabaseID(connection, database);

         name = table_name;
      }

      public bool Equals (BingoIndexID other)
      {
         return object_id == other.object_id && database_id == other.database_id;
      }

      private void parseTableName (string table_name, ref string database, ref string schema, ref string table)
      {
         Match match = Regex.Match(table_name, @"((\[(.*)\])|(.*))\.((\[(.*)\])|(.*))\.((\[(.*)\])|(.*))");

         if (match != Match.Empty)
         {
            database = (match.Groups[3].Length > 0 ? match.Groups[3].Value : match.Groups[4].Value);
            schema = (match.Groups[7].Length > 0 ? match.Groups[7].Value : match.Groups[8].Value);
            table = (match.Groups[11].Length > 0 ? match.Groups[11].Value : match.Groups[12].Value);
         }
         else
         {
            match = Regex.Match(table_name, @"((\[(.*)\])|(.*))\.((\[(.*)\])|(.*))");

            if (match != Match.Empty)
            {
               schema = (match.Groups[3].Length > 0 ? match.Groups[3].Value : match.Groups[4].Value);
               table = (match.Groups[7].Length > 0 ? match.Groups[7].Value : match.Groups[8].Value);
               match = Match.Empty;
            }
            else
            {
               match = Regex.Match(table_name, @"\[(.*)\]");

               if (match != Match.Empty)
                  table = match.Groups[1].Value;
               else
                  table = table_name;
            }
         }
      }


      public string SchemaName (SqlConnection connection)
      {
         return BingoSqlUtils.ExecStringQuery(connection,
            @"SELECT QUOTENAME(OBJECT_SCHEMA_NAME({0}, {1}))", object_id, database_id);
      }

      public string DatabaseName (SqlConnection connection)
      {
         return BingoSqlUtils.ExecStringQuery(connection,
            @"SELECT QUOTENAME(DB_NAME({0}))", database_id);
      }

      public string FullTableName (SqlConnection connection)
      {
         return BingoSqlUtils.ExecStringQuery(connection,
            @"SELECT QUOTENAME(DB_NAME({0})) + N'.' +
                     QUOTENAME(OBJECT_SCHEMA_NAME({1}, {0})) + N'.' +
                     QUOTENAME(OBJECT_NAME({1}, {0}))", database_id, object_id);
      }

      public string InformationName ()
      {
         return String.Format("{0} (db_id={1}, obj_id={2})", name, database_id, object_id);
      }
   }
}
