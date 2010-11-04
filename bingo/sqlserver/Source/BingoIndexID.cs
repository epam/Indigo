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
      public string database;
      public string schema;
      public string table;
      public int    object_id;

      public BingoIndexID (string full_table_name, int object_id)
      {
         parseTableName(full_table_name);
         this.object_id = object_id;
      }

      // Detect database, schema, and table; some from table_name, some from SQL connection
      public BingoIndexID (SqlConnection connection, string table_name)
      {
         using (SqlCommand cmd = new SqlCommand("SELECT OBJECT_ID('"  + table_name + "')", connection))
         {
            cmd.CommandTimeout = 3600;
            object res = cmd.ExecuteScalar();

            if (res == null || res == System.DBNull.Value)
               throw new Exception("can not get OBJECT_ID('" + table_name + "')");

            object_id = Convert.ToInt32(res);
         }

         parseTableName(table_name);

         if (database == null)
            database = connection.Database;

         if (schema == null)
            schema = GetConnectionSchema(connection);

         schema = schema.ToUpper();
         database = database.ToUpper();
         table = table.ToUpper();
      }

      private void parseTableName (string table_name)
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

      public bool Equals (BingoIndexID other)
      {
        if (!this.database.Equals(other.database))
            return false;
        if (!this.schema.Equals(other.schema))
            return false;
        if (!this.table.Equals(other.table))
            return false;

        return true;
      }

      private string GetConnectionSchema (SqlConnection connection)
      {
         using (SqlCommand cmd = new SqlCommand("SELECT SCHEMA_NAME()", connection))
         {
            object result = cmd.ExecuteScalar();
            return Convert.ToString(result);
         }
      }

      public string FullTableName ()
      {
         return "[" + database + "].[" + schema + "].[" + table + "]";
      }
   }
}
