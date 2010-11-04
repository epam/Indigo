using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;
using System.Data.SqlClient;
using System.Resources;
using System.Reflection;

namespace indigo
{
   public abstract class BingoIndexData
   {
      public BingoIndexID id;
      public string id_column;
      public string data_column;
      public string bingo_schema;

      public BingoFingerprints fingerprints;
      public BingoStorage storage;

      public enum IndexType { Molecule, Reaction }

      public bool keep_cache;

      public BingoIndexData (BingoIndexID id, string id_column, string data_column, string bingo_schema)
      {
         this.id = id;
         this.id_column = id_column;
         this.data_column = data_column;
         this.bingo_schema = bingo_schema;

         keep_cache = false;

         fingerprints = new BingoFingerprints(this);
         storage = new BingoStorage(this);
      }

      private static BingoIndexData _extractIndexData (SqlConnection conn, string bingo_schema, BingoIndexID id)
      {
         BingoIndexData data = null;

         using (SqlCommand cmd = new SqlCommand(String.Format(
            "SELECT id_column, data_column, type FROM {0} WHERE obj_id = '{1}'",
            _contextTable(bingo_schema), id.object_id), conn))
         {
            using (SqlDataReader reader = cmd.ExecuteReader())
            {
               reader.Read();

               string id_column = Convert.ToString(reader[0]);
               string data_column = Convert.ToString(reader[1]);
               string type = Convert.ToString(reader[2]);

               if (type.ToLower().Equals("molecule"))
                  data = new MangoIndexData(id, id_column, data_column, bingo_schema);
               if (type.ToLower().Equals("reaction"))
                  data = new RingoIndexData(id, id_column, data_column, bingo_schema);

               if (data == null)
                  throw new Exception("unknown type: " + type);

            }
         }

         data.keep_cache =
            (BingoConfig.getInt(conn, bingo_schema, "KEEP_CACHE", id.object_id) != 0);

         return data;
      }

      public abstract IndexType getIndexType ();
   
      private static string _contextTable (string bingo_schema)
      {
         return "[" + bingo_schema + "].context";
      }

      public class BingoIndexDataRefs
      {
         public BingoIndexData index_data;
         public List<int> session_ids;
      }

      private static object index_data_list_lock = new Object();
      private static ArrayList index_data_list = new ArrayList();

      public static BingoIndexData CreateIndexData (int spid, SqlConnection conn, string bingo_schema,
                      string table, string id_column, string data_column, bool reaction)
      {
         lock (index_data_list_lock)
         {
            BingoIndexID id = new BingoIndexID(conn, table);

            string command_text =
               String.Format("SELECT obj_id FROM {0} WHERE obj_id = '{1}'",
               _contextTable(bingo_schema), id.object_id);
            using (SqlCommand cmd = new SqlCommand(command_text, conn))
            {
               object res = cmd.ExecuteScalar();

               if (res != null)
                  _DropIndexData(conn, bingo_schema, table);
            }

            BingoIndexData data;

            if (reaction)
               data = new RingoIndexData(id, id_column, data_column, bingo_schema);
            else
               data = new MangoIndexData(id, id_column, data_column, bingo_schema);

            BingoSqlUtils.ExecNonQuery(conn,
               "INSERT INTO {0} VALUES({1}, '{2}', '{3}', '{4}', '{5}')",
               _contextTable(bingo_schema), id.object_id, id.FullTableName(), id_column, data_column,
               reaction ? "reaction" : "molecule");

            data.CreateTables(conn);
            data.CreateTriggers(conn);
            _AddIndexDataToList(spid, data);
            return data;
         }
      }

      public static void DropIndexData (SqlConnection conn, string bingo_schema, string table)
      {
         lock (index_data_list_lock)
         {
            _DropIndexData(conn, bingo_schema, table);
         }
      }

      private static void _DropIndexData (SqlConnection conn, string bingo_schema, string table)
      {
         BingoIndexID id = new BingoIndexID(conn, table);

         BingoIndexData data = _extractIndexData(conn, bingo_schema, id);
         data.DropTables(conn);
         data.DropTriggers(conn);

         BingoSqlUtils.ExecNonQueryNoThrow(conn, "DELETE FROM {0} WHERE obj_id = '{1}'",
            _contextTable(bingo_schema), id.object_id);

         for (int i = index_data_list.Count - 1; i >= 0; i--)
         {
            BingoIndexDataRefs refs = (BingoIndexDataRefs)index_data_list[i];
            if (refs.index_data.id.object_id == id.object_id &&
                refs.index_data.bingo_schema.Equals(bingo_schema))
            {
               index_data_list.RemoveAt(i);
               BingoLog.logMessage("Session for table {0} released",
                  refs.index_data.id.FullTableName());
            }
         }
      }

      public static BingoIndexData GetIndexData (SqlConnection conn, string bingo_schema, string table, int spid)
      {
         lock (index_data_list_lock)
         {
            BingoIndexID id = new BingoIndexID(conn, table);

            foreach (BingoIndexDataRefs index_data_refs in index_data_list)
            {
               if (index_data_refs.index_data.id.object_id == id.object_id &&
                   index_data_refs.index_data.bingo_schema.Equals(bingo_schema))
               {
                  if (!index_data_refs.session_ids.Contains(spid))
                     index_data_refs.session_ids.Add(spid);
                  return index_data_refs.index_data;
               }
            }

            BingoLog.logMessage("Extracting new BingoIndexData for spid={0} table={1}",
               spid, table);

            BingoIndexData data = _extractIndexData(conn, bingo_schema, id);

            _AddIndexDataToList(spid, data);

            return data;
         }
      }

      private static void _AddIndexDataToList (int spid, BingoIndexData data)
      {
         BingoIndexDataRefs new_refs = new BingoIndexDataRefs();

         new_refs.index_data = data;
         new_refs.session_ids = new List<int>();
         new_refs.session_ids.Add(spid);

         index_data_list.Add(new_refs);
      }

      class TableWithId
      {
         public string table_name { get; set; }
         public int id { get; set; }
      };

      public static void DropAllIndices (SqlConnection conn, string bingo_schema, bool only_invalid)
      {
         List<TableWithId> tables = new List<TableWithId>();
         using (SqlCommand cmd = new SqlCommand(String.Format(
            "SELECT full_table_name, obj_id FROM {0}",
            _contextTable(bingo_schema)), conn))
         {
            cmd.CommandTimeout = 3600;
            using (SqlDataReader reader = cmd.ExecuteReader())
            {
               while (reader.Read())
               {
                  string full_table_name = Convert.ToString(reader[0]);
                  int table_id = Convert.ToInt32(reader[1]);
                  tables.Add(new TableWithId() { table_name = full_table_name, id = table_id } );
               }
            }
         }

         foreach (TableWithId table in tables)
         {
            if (only_invalid)
            {
               object ret = BingoSqlUtils.ExecObjQuery(conn, "SELECT OBJECT_NAME({0})", table.id);
               if (ret != null)
                  continue;
            }
            DropIndexData(conn, bingo_schema, table.table_name);
         }
      }

      public static void FlushInAllSessions (int spid, bool check_spid)
      {
         lock (index_data_list_lock)
         {
            SqlConnection ctx_conn = null;
            try
            {
               for (int i = index_data_list.Count - 1; i >= 0; i--)
               {
                  BingoIndexDataRefs refs = (BingoIndexDataRefs)index_data_list[i];

                  if (check_spid && !refs.session_ids.Contains(spid))
                     continue;

                  if (!refs.index_data.needFlush())
                     continue;

                  if (ctx_conn == null)
                  {
                     ctx_conn = new SqlConnection("context connection=true");
                     ctx_conn.Open();
                  }
                  refs.index_data.flush(ctx_conn);
               }
            }
            finally
            {
               if (ctx_conn != null)
                  ctx_conn.Close();
            }


         }
      }

      public static void OnSessionClose (int spid)
      {
         FlushInAllSessions(spid, true);
         lock (index_data_list_lock)  
         {
            for (int i = index_data_list.Count - 1; i >= 0; i--)
            {
               BingoIndexDataRefs refs = (BingoIndexDataRefs)index_data_list[i];

               refs.session_ids.Remove(spid);
               if (refs.session_ids.Count < 1 && !refs.index_data.keep_cache)
               {
                  index_data_list.RemoveAt(i);
                  BingoLog.logMessage("Session for table {0} released",
                     refs.index_data.id.FullTableName());
               }
            }
         }
      }

      public virtual void CreateTables (SqlConnection conn)
      {
         storage.createTables(conn);
         fingerprints.createTables(conn);
      }

      public virtual void DropTables (SqlConnection conn)
      {
         storage.dropTables(conn);
         fingerprints.dropTables(conn);
      }

      public virtual bool needFlush()
      {
         if (storage.needFlush() || fingerprints.needFlush())
            return true;
         return false;
      }

      public virtual void flush (SqlConnection conn)
      {
         storage.flush(conn);
         fingerprints.flush(conn);
      }

      public virtual void prepareForDeleteRecord (SqlConnection conn)
      {
      }

      public void CreateTriggers (SqlConnection conn)
      {
         string insert_trigger = String.Format(resource.OnInsertTrigger,
            getTriggerName("Insert"), id.FullTableName(),
            id_column, data_column, bingo_schema, id.FullTableName());
         BingoSqlUtils.ExecNonQuery(conn, "{0}", insert_trigger);

         string delete_trigger = String.Format(resource.OnDeleteTrigger,
            getTriggerName("Delete"), id.FullTableName(),
            id_column, bingo_schema, id.FullTableName());
         BingoSqlUtils.ExecNonQuery(conn, "{0}", delete_trigger);

         string update_trigger = String.Format(resource.OnUpdateTrigger,
            getTriggerName("Update"), id.FullTableName(),
            id_column, data_column, bingo_schema, id.FullTableName());
         BingoSqlUtils.ExecNonQuery(conn, "{0}", update_trigger);
      }

      public void DropTriggers (SqlConnection conn)
      {
         BingoSqlUtils.ExecNonQueryNoThrow(conn, "DROP TRIGGER {0}", getTriggerName("Insert"));
         BingoSqlUtils.ExecNonQueryNoThrow(conn, "DROP TRIGGER {0}", getTriggerName("Delete"));
         BingoSqlUtils.ExecNonQueryNoThrow(conn, "DROP TRIGGER {0}", getTriggerName("Update"));
      }

      public virtual void deleteRecordById (int id, SqlConnection conn)
      {
         int? storage_id =
            BingoSqlUtils.ExecIntQuery(conn, "SELECT storage_id from {0} where id={1}",
               shadowTable, id);
         if (!storage_id.HasValue)
            // Such molecule wasn't added to the molecule 
            // index because it might be invalid
            return;
         storage.deleteRecord(storage_id.Value, conn);
         BingoSqlUtils.ExecNonQuery(conn, "DELETE from {0} where id={1}",
            shadowTable, id);
      }

      public virtual void createIndices (SqlConnection conn)
      {
      }

      private string getTriggerName (string operation)
      {
         return String.Format("[{0}].[{1}_{2}]",
            id.schema, id.table, operation);
      }

      public string fingerprintsTable
      {
         get { return "[" + bingo_schema + "].fingerprints_" + id.object_id; }
      }

      public string fingerprintBitsTable
      {
         get { return "[" + bingo_schema + "].fingerprint_bits_" + id.object_id; }
      }

      public string storageTable
      {
         get { return "[" + bingo_schema + "].storage_" + id.object_id; }
      }

      public string shadowTable
      {
         get { return "[" + bingo_schema + "].shadow_" + id.object_id; }
      }

      public void syncContextParameters (SqlConnection conn, string bingo_schema)
      {
         fingerprints.syncContextParameters(getIndexType() == IndexType.Reaction);

         keep_cache =
            (BingoConfig.getInt(conn, bingo_schema, "KEEP_CACHE", id.object_id) != 0);
      }

      public void setKeepCache (SqlConnection conn, string bingo_schema, bool keep)
      {
         keep_cache = keep;
         BingoConfig.setInt(conn, bingo_schema, "KEEP_CACHE", id.object_id, keep ? 1 : 0);
      }

   }
}
