using System;
using System.Collections.Generic;
using System.Text;
using System.Data;
using System.Data.SqlClient;
using System.Data.SqlTypes;

namespace indigo
{
   public class RingoIndexData : BingoIndexData
   {
      object _sync_object = new Object();

      public RingoIndexData (BingoIndexID id, string id_column, string data_column, string bingo_schema) :
         base(id, id_column, data_column, bingo_schema)
      {
      }

      public override IndexType getIndexType ()
      {
         return IndexType.Reaction;
      }

      public override void CreateTables (SqlConnection conn)
      {
         base.CreateTables(conn);

         StringBuilder cmd = new StringBuilder();

         // Create shadow table
         cmd.AppendFormat(@"CREATE TABLE {0}
            (id int not null, storage_id int not null, crf varbinary(max), hash int not null)", shadowTable);

         BingoSqlUtils.ExecNonQuery(conn, cmd.ToString());
      }

      public override void createIndices (SqlConnection conn)
      {
         BingoSqlUtils.ExecNonQuery(conn,
            "ALTER TABLE {0} ADD PRIMARY KEY (storage_id)", shadowTable);
         BingoSqlUtils.ExecNonQuery(conn,
            "CREATE UNIQUE INDEX id ON {0}(id)", shadowTable);
         BingoSqlUtils.ExecNonQuery(conn,
            "CREATE INDEX hash ON {0}(hash)", shadowTable);
      }

      public override void DropTables (SqlConnection conn)
      {
         base.DropTables(conn);
         BingoSqlUtils.ExecNonQueryNoThrow(conn, "DROP TABLE " + shadowTable);
      }

      DataTable shadow_datatable = null;

      public void addToShadowTable (SqlConnection conn, RingoIndex index, int id, int storage_id)
      {
         lock (_sync_object)
         {
            if (shadow_datatable == null)
               _createDataTable();

            if (shadow_datatable.Rows.Count >= 10000)
               _flushShadowTable(conn);

            DataRow shadow_row = shadow_datatable.NewRow();
            shadow_row["id"] = id;
            shadow_row["storage_id"] = storage_id;
            shadow_row["crf"] = index.crf;
            shadow_row["hash"] = index.hash;

            shadow_datatable.Rows.Add(shadow_row);
         }
      }

      private void _createDataTable ()
      {
         shadow_datatable = new DataTable();
         DataColumnCollection sc = shadow_datatable.Columns;

         sc.Add(new DataColumn("id", Type.GetType("System.Int32")));
         sc.Add(new DataColumn("storage_id", Type.GetType("System.Int32")));
         sc.Add(new DataColumn("crf", Type.GetType("System.Array")));
         sc.Add(new DataColumn("hash", Type.GetType("System.Int32")));
      }

      public override bool needFlush()
      {
         lock (_sync_object)
         {
            if (base.needFlush())
               return true;
            return shadow_datatable != null && shadow_datatable.Rows.Count > 0;
         }
      }

      public override void flush(SqlConnection conn)
      {
         lock (_sync_object)
         {
            base.flush(conn);
            _flushShadowTable(conn);
         }
      }

      private void _flushShadowTable (SqlConnection conn)
      {
         if (shadow_datatable == null || shadow_datatable.Rows.Count == 0)
            return;

         if (conn.ConnectionString == "context connection=true")
         {
            // SqlBulkInsert cannot be used in the context connection
            _flushShadowTableInContext(conn);
            return;
         }

         BingoTimer timer = new BingoTimer("shadow_table.flush");

         using (SqlTransaction transaction =
                  conn.BeginTransaction())
         {
            // Copy shadow table
            using (SqlBulkCopy bulkCopy = new SqlBulkCopy(conn,
                                                SqlBulkCopyOptions.TableLock, transaction))
            {
               bulkCopy.DestinationTableName = shadowTable;

               foreach (DataColumn dc in shadow_datatable.Columns)
                  bulkCopy.ColumnMappings.Add(dc.ColumnName, dc.ColumnName);

               bulkCopy.BatchSize = shadow_datatable.Rows.Count;
               bulkCopy.BulkCopyTimeout = 3600;
               bulkCopy.WriteToServer(shadow_datatable);
            }
            shadow_datatable.Rows.Clear();

            transaction.Commit();
         }

         timer.end();
      }

      private void _flushShadowTableInContext (SqlConnection conn)
      {
         foreach (DataRow row in shadow_datatable.Rows)
         {
            using (SqlCommand cmd = new SqlCommand())
            {
               cmd.CommandTimeout = 3600;

               StringBuilder cmd_text = new StringBuilder();

               cmd_text.AppendFormat("INSERT INTO {0} VALUES ", shadowTable);

               cmd_text.AppendFormat("({0}, {1}, @crf, {2})",
                  row["id"], row["storage_id"], row["hash"]);

               cmd.Parameters.AddWithValue("@crf", new SqlBinary((byte[])row["crf"]));

               cmd.Connection = conn;
               cmd.CommandText = cmd_text.ToString();
               cmd.ExecuteNonQuery();
            }
         }
         shadow_datatable.Rows.Clear();
      }

      public override void prepareForDeleteRecord (SqlConnection conn)
      {
         lock (_sync_object)
         {
            _flushShadowTable(conn);
         }
      }
   }
}
