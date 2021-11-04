using System;
using System.Data;
using System.Collections.Generic;
using System.Text;
using System.Collections;
using System.Data.SqlClient;
using System.Data.SqlTypes;

namespace indigo
{
   class BingoConfig
   {
      public class TautomerRule
      {
         public int n;
         public string beg;
         public string end;
      }

      private static object get (SqlConnection connection, string bingo_schema,
         string name, int id, string table)
      {
         using (SqlCommand cmd = new SqlCommand())
         {
            cmd.Connection = connection;

            cmd.CommandText = String.Format(
               @"select value from {0}.{1} where name='{2}' and n in (0, {3}) order by n desc",
               bingo_schema, table, name, id);
            cmd.CommandType = CommandType.Text;
            cmd.CommandTimeout = 3600;

            object result = cmd.ExecuteScalar();
            if (result == null)
               throw new Exception("Key " + name + " isn't present in the configuration table");
            return result;
         }
      }

      private static object _set (SqlConnection connection, string bingo_schema,
         string name, int id, string table, object value)
      {
         _remove(connection, bingo_schema, name, id, table);
         using (SqlCommand cmd = new SqlCommand())
         {
            cmd.Connection = connection;
            cmd.CommandTimeout = 3600;

            cmd.CommandText = String.Format(
               @"insert into {0}.{1} values({2}, '{3}', @data)",
               bingo_schema, table, id, name);
            cmd.CommandType = CommandType.Text;
            cmd.Parameters.AddWithValue("@data", value);

            return cmd.ExecuteScalar();
         }
      }

      public static int getInt (SqlConnection connection, string bingo_schema, string name, int id)
      {
         object res = get(connection, bingo_schema, name, id, "CONFIG");
         return Convert.ToInt32(res);
      }

      public static double getDouble (SqlConnection connection, string bingo_schema, string name, int id)
      {
         object res = get(connection, bingo_schema, name, id, "CONFIG");
         return Convert.ToDouble(res);
      }

      public static string getString (SqlConnection connection, string bingo_schema, string name, int id)
      {
         object res = get(connection, bingo_schema, name, id, "CONFIG");
         return Convert.ToString(res);
      }

      public static byte[] getBinary (SqlConnection connection, string bingo_schema, string name, int id)
      {
         object res = get(connection, bingo_schema, name, id, "CONFIG_BIN");
         return (byte[])res;
      }

      public static void setBinary (SqlConnection connection, string bingo_schema,
         string name, int id, byte[] data)
      {
         _set(connection, bingo_schema, name, id, "CONFIG_BIN", data);
      }

      public static void setInt (SqlConnection connection, string bingo_schema,
         string name, int id, int data)
      {
         remove(connection, bingo_schema, name, id);
         _set(connection, bingo_schema, name, id, "CONFIG", data);
      }

      public static void _remove (SqlConnection connection, string bingo_schema, string name, int id, string table)
      {
         BingoSqlUtils.ExecNonQueryNoThrow(connection, "DELETE FROM {0}.{1} where name='{2}' and n={3}",
            bingo_schema, table, name, id);
      }

      public static void remove (SqlConnection connection, string bingo_schema, string name, int id)
      {
         _remove(connection, bingo_schema, name, id, "CONFIG");
      }

      public static void removeBinary (SqlConnection connection, string bingo_schema, string name, int id)
      {
         _remove(connection, bingo_schema, name, id, "CONFIG_BIN");
      }

      public static ArrayList getTautomerRules (SqlConnection connection, string bingo_schema)
      {
         ArrayList result = new ArrayList();

         SqlCommand cmd = new SqlCommand();
         cmd.Connection = connection;

         cmd.CommandText = String.Format(@"select id, begg, endd from
                 {0}.TAUTOMER_RULES", bingo_schema);
         cmd.CommandType = CommandType.Text;
         cmd.CommandTimeout = 3600;

         using (SqlDataReader reader = cmd.ExecuteReader())
         {
            while (reader.Read())
            {
               TautomerRule rule = new TautomerRule();

               rule.n = Convert.ToInt32(reader[0]);
               rule.beg = Convert.ToString(reader[1]);
               rule.end = Convert.ToString(reader[2]);

               result.Add(rule);
            }
         }

         return result;
      }
   }
}
