using System;
using System.Collections.Generic;
using System.Text;
using System.Data;
using System.Data.SqlClient;
using System.Data.SqlTypes;
using System.Globalization;

namespace indigo
{
   public class MangoIndexData : BingoIndexData
   {
      object _sync_object = new Object();

      public MangoIndexData (BingoIndexID id, string id_column, string data_column, string bingo_schema) :
         base(id, id_column, data_column, bingo_schema)
      {
      }

      public override IndexType getIndexType ()
      {
         return IndexType.Molecule;
      }

      public override void CreateTables (SqlConnection conn)
      {
         base.CreateTables(conn);

         StringBuilder cmd = new StringBuilder();

         // Create shadow table
         cmd.AppendFormat(@"CREATE TABLE {0}
            (id int not null, storage_id int not null, gross VARCHAR(500), cmf varbinary(max), 
            xyz varbinary(max), mass real not null, fragments int not null", shadowTable);

         for (int i = 0; i < MangoIndex.COUNTED_ELEMENTS_COUNT; i++)
            cmd.AppendFormat(", {0} int not null", BingoCore.mangoGetCountedElementName(i));
         cmd.Append(")");

         BingoSqlUtils.ExecNonQuery(conn, cmd.ToString());

         // Create shadow table for molecule components
         BingoSqlUtils.ExecNonQuery(conn,
            "CREATE TABLE {0} (id int not null, hash int not null, count int not null)",
            componentsTable);
      }

      public override void createIndices (SqlConnection conn)
      {
         BingoSqlUtils.ExecNonQuery(conn,
            "ALTER TABLE {0} ADD PRIMARY KEY (storage_id)", shadowTable);
         BingoSqlUtils.ExecNonQuery(conn, 
            "CREATE UNIQUE INDEX id ON {0}(id)", shadowTable);
         BingoSqlUtils.ExecNonQuery(conn,
            "CREATE INDEX gross ON {0}(gross)", shadowTable);
         BingoSqlUtils.ExecNonQuery(conn,
            "CREATE INDEX mass ON {0}(mass)", shadowTable);
         for (int i = 0; i < MangoIndex.COUNTED_ELEMENTS_COUNT; i++)
         {
            BingoSqlUtils.ExecNonQuery(conn,
               "CREATE INDEX {1} ON {0}({1})", shadowTable,
               BingoCore.mangoGetCountedElementName(i));
         }

         // Create indices for components shadow table
         BingoSqlUtils.ExecNonQuery(conn,
            "CREATE INDEX id ON {0}(id)", componentsTable);
         BingoSqlUtils.ExecNonQuery(conn,
            "CREATE INDEX hash ON {0}(hash)", componentsTable);
         BingoSqlUtils.ExecNonQuery(conn,
            "CREATE INDEX count ON {0}(hash, count)", componentsTable);
      }

      public override void DropTables (SqlConnection conn)
      {
         base.DropTables(conn);

         BingoSqlUtils.ExecNonQueryNoThrow(conn, "DROP TABLE " + shadowTable);
         BingoSqlUtils.ExecNonQueryNoThrow(conn, "DROP TABLE " + componentsTable);
      }

      DataTable shadow_datatable = null;
      DataTable components_datatable = null;

      public void addToShadowTable (SqlConnection conn, MangoIndex index, int id, int storage_id)
      {
         lock (_sync_object)
         {
            if (shadow_datatable == null)
               _createDataTables();

            if (shadow_datatable.Rows.Count >= 10000)
               _flushShadowTable(conn);

            DataRow shadow_row = shadow_datatable.NewRow();
            shadow_row["id"] = id;
            shadow_row["storage_id"] = storage_id;
            shadow_row["gross"] = index.gross;
            shadow_row["cmf"] = index.cmf;
            shadow_row["xyz"] = index.xyz;
            shadow_row["mass"] = index.mass;

            int fragments_count = 0;
            for (int i = 0; i < index.hash.elements.Count; i++)
               fragments_count += index.hash.elements[i].count;
            shadow_row["fragments"] = fragments_count;

            string[] counted = index.counted_elements_str.Split(',');
            for (int i = 0; i < MangoIndex.COUNTED_ELEMENTS_COUNT; i++)
               shadow_row[BingoCore.mangoGetCountedElementName(i)] = Convert.ToInt32(counted[i + 1]);

            shadow_datatable.Rows.Add(shadow_row);

            foreach (MoleculeHashElement elem in index.hash.elements)
            {
               DataRow comp_row = components_datatable.NewRow();
               comp_row["id"] = id;
               comp_row["hash"] = elem.hash;
               comp_row["count"] = elem.count;

               components_datatable.Rows.Add(comp_row);
            }
         }
      }

      private void _createDataTables ()
      {
         shadow_datatable = new DataTable();
         DataColumnCollection sc = shadow_datatable.Columns;

         sc.Add(new DataColumn("id", Type.GetType("System.Int32")));
         sc.Add(new DataColumn("storage_id", Type.GetType("System.Int32")));
         sc.Add(new DataColumn("gross", Type.GetType("System.String")));
         sc.Add(new DataColumn("cmf", Type.GetType("System.Array")));
         sc.Add(new DataColumn("xyz", Type.GetType("System.Array")));
         sc.Add(new DataColumn("mass", Type.GetType("System.Single")));
         sc.Add(new DataColumn("fragments", Type.GetType("System.Int32")));
         for (int i = 0; i < MangoIndex.COUNTED_ELEMENTS_COUNT; i++)
            sc.Add(new DataColumn(BingoCore.mangoGetCountedElementName(i), Type.GetType("System.Int32")));

         components_datatable = new DataTable();
         DataColumnCollection cc = components_datatable.Columns;

         cc.Add(new DataColumn("id", Type.GetType("System.Int32")));
         cc.Add(new DataColumn("hash", Type.GetType("System.Int32")));
         cc.Add(new DataColumn("count", Type.GetType("System.Int32")));
      }

      public override bool needFlush()
      {
         lock (_sync_object)
         {
            if (base.needFlush())
               return true;
            return shadow_datatable != null && (shadow_datatable.Rows.Count > 0 || components_datatable.Rows.Count > 0);
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
         if (shadow_datatable == null || (shadow_datatable.Rows.Count == 0 && components_datatable.Rows.Count == 0))
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

            // Copy components table
            using (SqlBulkCopy bulkCopy = new SqlBulkCopy(conn,
                                                SqlBulkCopyOptions.TableLock, transaction))
            {
               bulkCopy.DestinationTableName = componentsTable;

               foreach (DataColumn dc in components_datatable.Columns)
                  bulkCopy.ColumnMappings.Add(dc.ColumnName, dc.ColumnName);

               bulkCopy.BatchSize = components_datatable.Rows.Count;
               bulkCopy.BulkCopyTimeout = 3600;
               bulkCopy.WriteToServer(components_datatable);
            }
            components_datatable.Rows.Clear();

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

               cmd_text.AppendFormat(CultureInfo.InvariantCulture,
                   "({0}, {1}, '{2}', @cmf, @xyz, {3}, {4} ",
                  row["id"], row["storage_id"], row["gross"],
                  row["mass"], row["fragments"]);

               for (int i = 0; i < MangoIndex.COUNTED_ELEMENTS_COUNT; i++)
                  cmd_text.AppendFormat(", {0}", row[BingoCore.mangoGetCountedElementName(i)]);
               cmd_text.Append(")");

               cmd.Parameters.AddWithValue("@cmf", new SqlBinary((byte[])row["cmf"]));
               cmd.Parameters.AddWithValue("@xyz", new SqlBinary((byte[])row["xyz"]));

               cmd.Connection = conn;
               cmd.CommandText = cmd_text.ToString();
               cmd.ExecuteNonQuery();
            }
         }
         shadow_datatable.Rows.Clear();

         foreach (DataRow row in components_datatable.Rows)
         {
            using (SqlCommand cmd = new SqlCommand())
            {
               cmd.CommandTimeout = 3600;
               StringBuilder cmd_text = new StringBuilder();

               cmd_text.AppendFormat("INSERT INTO {0} VALUES ", componentsTable);
               cmd_text.AppendFormat("({0}, {1}, {2}) ",
                  row["id"], row["hash"], row["count"]);

               cmd.Connection = conn;
               cmd.CommandText = cmd_text.ToString();
               cmd.ExecuteNonQuery();
            }
         }

         components_datatable.Rows.Clear();
      }

      public byte[] getXyz (int storage_id, SqlConnection conn)
      {
         object ret = BingoSqlUtils.ExecObjQuery(conn, "SELECT xyz from {0} where storage_id={1}",
            shadowTable, storage_id);
         return (byte[])ret;
      }

      public override void prepareForDeleteRecord (SqlConnection conn)
      {
         lock (_sync_object)
         {
            _flushShadowTable(conn);
         }
      }

      public override void deleteRecordById (int id, SqlConnection conn)
      {
         lock (_sync_object)
         {
            base.deleteRecordById(id, conn);
            BingoSqlUtils.ExecNonQuery(conn, "DELETE from {0} where id={1}",
               componentsTable, id);
         }
      }

      public string componentsTable
      {
         get { return "[" + bingo_schema + "].components_" + id.table_id; }
      }
   }
}
