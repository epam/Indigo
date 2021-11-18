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
   public class MangoShadowFetch
   {
      MangoIndexData _index_data;
      enum SearchType
      {
         UNDEF, EXACT, GROSS, MASS
      };
      SearchType search_type = SearchType.UNDEF;
      string table_copies, where_clause;
      bool need_xyz = false;

      public int? nextAfterStorageId { get; set; }

      public MangoShadowFetch (MangoIndexData index_data)
      {
         _index_data = index_data;
      }

      public void prepareExact (string query, string options)
      {
         int res = BingoCore.lib.mangoSetupMatch("EXACT", query, options);
         if (res < 0)
            throw new Exception(BingoCore.lib.bingoGetError());
         table_copies = "";
         where_clause = "";
         if (options.Contains("TAU"))
         {
            where_clause = String.Format("gross = '{0}' OR gross LIKE '{0} H%%'",
               BingoCore.mangoTauGetQueryGross());
         }
         else
         {
            need_xyz = (BingoCore.lib.mangoNeedCoords() != 0);
            _prepareExactQueryStrings(ref table_copies, ref where_clause);
         }

         search_type = SearchType.EXACT;
      }

      public void prepareGross (string query)
      {
         search_type = SearchType.GROSS;
         int res = BingoCore.lib.mangoSetupMatch("GROSS", query, "");
         if (res < 0)
            throw new Exception(BingoCore.lib.bingoGetError());
         where_clause = BingoCore.mangoGrossGetConditions();
      }

      public void prepareMass (double? min, double? max)
      {
         search_type = SearchType.MASS;

         StringBuilder where_text = new StringBuilder();
         if (min.HasValue)
            where_text.AppendFormat(CultureInfo.InvariantCulture,
                "mass >= {0} ", min.Value);
         if (max.HasValue)
         {
            if (where_text.Length > 0)
               where_text.Append(" AND ");

            where_text.AppendFormat(CultureInfo.InvariantCulture,
                " mass <= {0} ", max.Value);
         }
         where_clause = where_text.ToString();
      }

      public IEnumerable<FetchedData> fetch (SqlConnection conn)
      {                      
         ArrayList res_list = new ArrayList();

         StringBuilder command_text = new StringBuilder();
         command_text.Append("SELECT sh.id");
         if (search_type == SearchType.EXACT)
         {
            command_text.Append(", sh.cmf");
            if (need_xyz)
               command_text.Append(", sh.xyz");
         }
         if (search_type == SearchType.GROSS)
            command_text.Append(", gross");
         if (search_type == SearchType.MASS)
            command_text.Append(", mass");
         command_text.AppendFormat(" FROM {0} sh", _index_data.shadowTable);
         command_text.Append(table_copies);


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
               byte[] xyz = new byte[0];
               while (reader.Read())
               {
                  int id = Convert.ToInt32(reader[0]);

                  int res;
                  if (search_type == SearchType.EXACT)
                  {
                     byte[] cmf = (byte[])reader[1];
                     if (need_xyz)
                        xyz = (byte[])reader[2];
                     res = BingoCore.lib.mangoMatchTargetBinary(cmf, cmf.Length, xyz, xyz.Length);
                  }
                  else if (search_type == SearchType.GROSS)
                  {
                     byte[] gross = encoding.GetBytes((string)reader[1]);
                     res = BingoCore.lib.mangoMatchTarget(gross, gross.Length);
                  }
                  else if (search_type == SearchType.MASS)
                     res = 1;
                  else
                     throw new Exception("Search type is undefined");

                  if (res == -2)
                     throw new Exception(BingoCore.lib.bingoGetError());
                  if (res == -1)
                     throw new Exception(BingoCore.lib.bingoGetWarning());

                  if (res == 1)
                  {
                     FetchedData data = new FetchedData(id);
                     if (search_type == SearchType.MASS)
                        data.value = (double)reader[1];

                     yield return data;
                  }
               }
            }
         }
      }

      void _prepareExactQueryStrings (ref string table_copies_str, ref string where_clause_str)
      {
         int hash_elements_count, hash, count;
         BingoCore.lib.mangoGetHash(false, -1, out hash_elements_count, out hash);

         StringBuilder table_copies = new StringBuilder();
         for (int i = 0; i < hash_elements_count; i++)
            table_copies.AppendFormat(", {0} t{1}", _index_data.componentsTable, i);

         // Create complex WHERE clause
         StringBuilder where_clause = new StringBuilder();
         bool where_was_added = false;
         if (hash_elements_count > 0)
         {
            where_was_added = true;
            // molecule ids must be same
            where_clause.Append("sh.id = t0.id and ");
            for (int i = 1; i < hash_elements_count; i++)
               where_clause.AppendFormat("t{0}.id = t{1}.id and ", i - 1, i);
            // query components must match target components
            for (int i = 0; i < hash_elements_count; i++)
            {
               BingoCore.lib.mangoGetHash(false, i, out count, out hash);
               where_clause.AppendFormat("t{0}.hash = {1} and ", i, hash);
            }

            // components count mast must target components count
            string rel;
            if (BingoCore.lib.mangoExactNeedComponentMatching())
               rel = ">=";
            else
               rel = "=";

            for (int i = 0; i < hash_elements_count; i++)
            {
               if (i != 0)
                  where_clause.Append("and ");
               BingoCore.lib.mangoGetHash(false, i, out count, out hash);
               where_clause.AppendFormat("t{0}.count {1} {2} ", i, rel, count);
            }
         }
         if (!BingoCore.lib.mangoExactNeedComponentMatching())
         {
            if (where_was_added)
               where_clause.Append("and ");

            // There must be no other components in target
            int query_fragments_count = 0;
            for (int i = 0; i < hash_elements_count; i++)
            {
               BingoCore.lib.mangoGetHash(false, i, out count, out hash);
               query_fragments_count += count;
            }
            where_clause.AppendFormat("sh.fragments = {0}", query_fragments_count);
         }

         table_copies_str = table_copies.ToString();
         where_clause_str = where_clause.ToString();
      }


   }
}