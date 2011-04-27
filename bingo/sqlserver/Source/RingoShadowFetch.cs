using System.IO;
using System;
using System.Collections.Generic;
using System.Collections;
using System.Text;
using System.Data;
using System.Data.SqlClient;
using System.Data.SqlTypes;
using System.Globalization;

namespace indigo
{
   public class RingoShadowFetch
   {
      RingoIndexData _index_data;
      enum SearchType
      {
         UNDEF, EXACT
      };
      SearchType search_type = SearchType.UNDEF;
      string where_clause;

      public int? nextAfterStorageId { get; set; }

      public RingoShadowFetch (RingoIndexData index_data)
      {
         _index_data = index_data;
      }

      public void prepareExact (string query, string options)
      {
         int res = BingoCore.lib.ringoSetupMatch("REXACT", query, options);
         if (res < 0)
            throw new Exception(BingoCore.lib.bingoGetError());

         int query_hash;
         BingoCore.lib.ringoGetHash(false, out query_hash);
         where_clause = String.Format("hash = {0}", query_hash);
         search_type = SearchType.EXACT;
      }

      public IEnumerable<FetchedData> fetch (SqlConnection conn)
      {                      
         ArrayList res_list = new ArrayList();

         StringBuilder command_text = new StringBuilder();
         command_text.Append("SELECT sh.id");
         if (search_type == SearchType.EXACT)
         {
            command_text.Append(", sh.crf");
         }
         command_text.AppendFormat(" FROM {0} sh", _index_data.shadowTable);

         StringBuilder final_where_clause = new StringBuilder();
         if (where_clause.Length > 0)
         {
            final_where_clause.Append(" ( ");
            final_where_clause.Append(where_clause);
            final_where_clause.Append(" ) ");
         }
         if (nextAfterStorageId != null)
         {
            if (final_where_clause.Length > 0)
               final_where_clause.Append(" and ");
            final_where_clause.Append(" (");
            final_where_clause.AppendFormat(" storage_id > {0} ", nextAfterStorageId.Value);
            final_where_clause.Append(" )");
         }
         if (final_where_clause.Length > 0)
         {
            command_text.Append(" WHERE ");
            command_text.Append(final_where_clause.ToString());
         }

         if (nextAfterStorageId.HasValue)
            command_text.Append(" ORDER BY storage_id");

         UTF8Encoding encoding = new UTF8Encoding();

         using (SqlCommand cmd = new SqlCommand(command_text.ToString(), conn))
         {
            cmd.CommandTimeout = 3600;
            using (SqlDataReader reader = cmd.ExecuteReader())
            {
               while (reader.Read())
               {
                  int id = Convert.ToInt32(reader[0]);

                  int res;
                  if (search_type == SearchType.EXACT)
                  {
                     byte[] crf = (byte[])reader[1];
                     res = BingoCore.lib.ringoMatchTargetBinary(crf, crf.Length);
                  }
                  else
                     throw new Exception("Search type is undefined");

                  if (res == -2)
                     throw new Exception(BingoCore.lib.bingoGetError());
                  if (res == -1)
                     throw new Exception(BingoCore.lib.bingoGetWarning());

                  if (res == 1)
                  {
                     FetchedData data = new FetchedData(id);
                     yield return data;
                  }
               }
            }
         }
      }
   }
}