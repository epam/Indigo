using System;
using System.Reflection;
using System.Reflection.Emit;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Data;
using Microsoft.SqlServer.Server;
using System.Data.SqlTypes;
using System.Data.SqlClient;
using System.Collections;
using System.IO;
using System.Text;
using Microsoft.Win32;
using System.Threading;
using System.Text.RegularExpressions;
using indigo.SqlAttributes;

namespace indigo
{
   public class Bingo
   {
      [SqlFunction(DataAccess = DataAccessKind.Read,
        SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader]
      public static SqlString GetVersion (SqlString bingo_schema)
      {
         return BingoCore.lib.bingoGetVersion();
      }

      private delegate void bingoCallback ();

      private static SqlInt32 _Match (SqlBinary target, SqlString query, SqlString options,
         SqlString bingo_schema, string search_type,
         bingoCallback prepare_match, bingoCallback process_matched)
      {
         using (BingoSession sessions = new BingoSession())
         {
            ContextFlags flags = 0;

            if (options.Value.Contains("TAU"))
               flags |= ContextFlags.TAU_RULES;
            if (search_type == "SIM")
               flags |= ContextFlags.FINGERPRINTS;

            using (SqlConnection conn = new SqlConnection("context connection=true"))
            {
               conn.Open();
               prepareContext(conn, bingo_schema.Value, 0, flags);
            }

            int res = BingoCore.lib.mangoSetupMatch(search_type, query.Value, options.Value);
            if (res < 0)
               throw new Exception(BingoCore.lib.bingoGetError());

            if (prepare_match != null)
               prepare_match();

            res = BingoCore.lib.mangoMatchTarget(target.Value, target.Value.Length);
            if (res == -2)
               throw new Exception(BingoCore.lib.bingoGetError());

            if (res == -1)
            {
               // can not use SqlContext.Pipe from inside the function, 
               // so just returning NULL without printing the error message
               return SqlInt32.Null;
            }

            if (res == 1 && process_matched != null)
               process_matched();

            return new SqlInt32(res);
         }
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
        SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "target")]
      public static SqlInt32 Sub (SqlBinary target, SqlString query, SqlString options, SqlString bingo_schema)
      {
         return _Match(target, query, options, bingo_schema, "SUB", null, null);
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
        SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "target")]
      public static SqlInt32 SMARTS (SqlBinary target, SqlString query, SqlString options, SqlString bingo_schema)
      {
         return _Match(target, query, options, bingo_schema, "SMARTS", null, null);
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
        SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "target")]
      public static SqlInt32 Exact (SqlBinary target, SqlString query, SqlString options, SqlString bingo_schema)
      {
         return _Match(target, query, options, bingo_schema, "EXACT", null, null);
      }

      private static string MatchWithHighlighting (SqlBinary target, SqlString query,
         SqlString parameters, SqlString bingo_schema, string search_type)
      {
         string highlighting = null;

         bingoCallback prepare =
            () =>
            {
               BingoCore.lib.mangoSetHightlightingMode(1);
            };

         bingoCallback handle =
            () =>
            {
               highlighting = BingoCore.mangoGetHightlightedMolecule();
            };

         _Match(target, query, parameters, bingo_schema, search_type, prepare, handle);
         return highlighting;
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "target")]
      public static SqlString SubHi (SqlBinary target, SqlString query, SqlString parameters, SqlString bingo_schema)
      {
         return MatchWithHighlighting(target, query, parameters, bingo_schema, "SUB");
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "target")]
      public static SqlString SMARTSHi (SqlBinary target, SqlString query, SqlString parameters, SqlString bingo_schema)
      {
         return MatchWithHighlighting(target, query, parameters, bingo_schema, "SMARTS");
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
        SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "target")]
      public static SqlDouble Sim (SqlBinary target, SqlString query, SqlString metrics, SqlString bingo_schema)
      {
         SqlDouble score = SqlDouble.Null;
         bingoCallback handler =
            () =>
            {
               Double score_value;
               BingoCore.lib.mangoSimilarityGetScore(out score_value);
               score = score_value;
            };

         _Match(target, query, metrics, bingo_schema, "SIM", null, handler);
         return score;
      }

      private static SqlInt32 _RMatch (SqlBinary target, SqlString query, SqlString options,
         SqlString bingo_schema, string search_type, bool heed_highlighting, ref string highlighting)
      {
         using (BingoSession sessions = new BingoSession())
         {
            ContextFlags flags = 0;

            if (options.Value.Contains("TAU"))
               flags |= ContextFlags.TAU_RULES;

            using (SqlConnection conn = new SqlConnection("context connection=true"))
            {
               conn.Open();
               prepareContext(conn, bingo_schema.Value, 0, flags);
            }

            int res = BingoCore.lib.ringoSetupMatch(search_type, query.Value, options.Value);
            if (res < 0)
               throw new Exception(BingoCore.lib.bingoGetError());

            if (heed_highlighting)
               BingoCore.lib.ringoSetHightlightingMode(1);

            res = BingoCore.lib.ringoMatchTarget(target.Value, target.Value.Length);
            if (res == -2)
               throw new Exception(BingoCore.lib.bingoGetError());

            if (res == -1)
            {
               // can not use SqlContext.Pipe from inside the function, 
               // so just returning NULL without printing the error message
               return SqlInt32.Null;
            }

            if (res == 1 && heed_highlighting)
               highlighting = BingoCore.ringoGetHightlightedReaction();

            return new SqlInt32(res);
         }
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "target")]
      public static SqlInt32 RSub (SqlBinary target, SqlString query, SqlString bingo_schema)
      {
         string highlighting = null;
         return _RMatch(target, query, "", bingo_schema, "RSUB", false, ref highlighting);
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "target")]
      public static SqlString RSubHi (SqlBinary target, SqlString query, SqlString bingo_schema)
      {
         string highlighting = null;
         _RMatch(target, query, "", bingo_schema, "RSUB", true, ref highlighting);
         return highlighting;
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "target")]
      public static SqlInt32 RSMARTS (SqlBinary target, SqlString query, SqlString bingo_schema)
      {
         string highlighting = null;
         return _RMatch(target, query, "", bingo_schema, "RSMARTS", false, ref highlighting);
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "target")]
      public static SqlString RSMARTSHi (SqlBinary target, SqlString query, SqlString bingo_schema)
      {
         string highlighting = null;
         _RMatch(target, query, "", bingo_schema, "RSMARTS", true, ref highlighting);
         return highlighting;
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
        SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "target")]
      public static SqlInt32 RExact (SqlBinary target, SqlString query, SqlString options, SqlString bingo_schema)
      {
         string highlighting = null;
         return _RMatch(target, query, options, bingo_schema, "REXACT", false, ref highlighting);
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "reaction")]
      public static SqlString AAM (SqlBinary reaction, SqlString options, SqlString bingo_schema)
      {
         using (SqlConnection conn = new SqlConnection("context connection=true"))
         {
            conn.Open();
            prepareContext(conn, bingo_schema.Value, 0, 0);
         }
         return new SqlString(BingoCore.ringoAAM(reaction.Value, options.Value));
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "reaction")]
      public static SqlString CheckReaction (SqlBinary reaction, SqlString bingo_schema)
      {
         using (SqlConnection conn = new SqlConnection("context connection=true"))
         {
            conn.Open();
            prepareContext(conn, bingo_schema.Value, 0, 0);
         }
         string res = BingoCore.checkReaction(reaction.Value);
         if (res == null)
            return SqlString.Null;
         return new SqlString(res);
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "molecule")]
      public static SqlString CheckMolecule (SqlBinary molecule, SqlString bingo_schema)
      {
         using (SqlConnection conn = new SqlConnection("context connection=true"))
         {
            conn.Open();
            prepareContext(conn, bingo_schema.Value, 0, 0);
         }
         string res = BingoCore.checkMolecule(molecule.Value);
         if (res == null)
            return SqlString.Null;
         return new SqlString(res);
      }

      private delegate void bingoOperationDelegate
         (SqlConnection ctx_conn, SqlConnection conn, BingoIndexData index_data);
      private delegate BingoIndexData bingoGetIndexDataDelegate
         (SqlConnection ctx_conn, SqlConnection conn, SqlString bingo_schema);

      [Flags]
      enum BingoOp
      {
         NEED_STAT = 0x01,
         DROP_ON_EXCEPTION = 0x02,
         LOAD_TAU_RULES = 0x04,
         LOAD_CMF = 0x08,
         NON_CONTEXT_CONN = 0x10,
         LOCK_INDEX = 0x20
      }

      // Method for executing abstract operation with BingoIndexData
      private static void _ExecuteBingoOperation(SqlString bingo_schema, 
         bingoOperationDelegate operationDelegate,
         bingoGetIndexDataDelegate getBingoDataDelegate, BingoOp op_flags)
      {
         string table_name = "<Undef>";
         BingoIndexID id = null;
         SqlConnection ext_conn = null;
         try
         {
            using (SqlConnection ctx_conn = new SqlConnection("context connection=true"))
            {
               ctx_conn.Open();

               SqlConnection conn = ctx_conn;
               if ((op_flags & BingoOp.NON_CONTEXT_CONN) != 0)
               {
                  string conn_string = String.Format(
                     "server={0};integrated security=true;database={1};enlist=false", 
                     getServername(ctx_conn), ctx_conn.Database);
                  ext_conn = new SqlConnection(conn_string);
                  ext_conn.Open();
                  conn = ext_conn;
               }

               using (BingoSession session = new BingoSession())
               {
                  BingoCore.lib.bingoProfilingReset(false);

                  BingoTimer timer = new BingoTimer("total");

                  BingoIndexData index_data = getBingoDataDelegate(ctx_conn, conn, bingo_schema);
                  table_name = index_data.id.FullTableName(ctx_conn);
                  id = index_data.id;
                  if (index_data.locked)
                  {
                     BingoLog.logMessage("Attempt to get locked index for the table {0}", table_name);
                     throw new Exception("Chemical index for the table '" + table_name + "' is locked");
                  }
                  if ((op_flags & BingoOp.LOCK_INDEX) != 0)
                     index_data.locked = true;

                  ContextFlags flags = ContextFlags.NTHREADS | ContextFlags.FINGERPRINTS;
                  if ((op_flags & BingoOp.LOAD_TAU_RULES) != 0)
                     flags |= ContextFlags.TAU_RULES;
                  if ((op_flags & BingoOp.LOAD_CMF) != 0)
                     flags |= ContextFlags.CMF;

                  prepareContext(ctx_conn, bingo_schema.Value, index_data.id.table_id, flags);
                  index_data.syncContextParameters(ctx_conn, bingo_schema.Value);

                  try
                  {
                     operationDelegate(ctx_conn, conn, index_data);
                  }
                  catch (Exception ex)
                  {
                     if ((Thread.CurrentThread.ThreadState & ThreadState.AbortRequested) != 0)
                        Thread.ResetAbort();

                     BingoLog.logMessage("Exception {0} in {1}:\n{2}", ex.Message, ex.Source, ex.StackTrace);

                     if ((op_flags & BingoOp.LOCK_INDEX) != 0)
                        index_data.locked = false;

                     if ((op_flags & BingoOp.DROP_ON_EXCEPTION) != 0)
                        BingoIndexData.DropIndexData(conn, bingo_schema.Value, id, false);

                     throw ex;
                  }

                  if ((op_flags & BingoOp.LOCK_INDEX) != 0)
                     index_data.locked = false;

                  timer.end();

                  if ((op_flags & BingoOp.NEED_STAT) != 0)
                     BingoLog.logMessage("Statistics for table {0}:\n{1}\n",
                        table_name, BingoCore.bingoProfilingGetStatistics(false));
               }
            }
         }
         finally
         {
            if ((Thread.CurrentThread.ThreadState & ThreadState.AbortRequested) != 0)
               Thread.ResetAbort();
            if (ext_conn != null)
               ext_conn.Close();
         }
      }

      // Method for executing abstract operation with BingoIndexData
      private static void _ExecuteBingoOperationChangeIndex (SqlString bingo_schema, 
         bingoOperationDelegate operationDelegate,
         bingoGetIndexDataDelegate getBingoDataDelegate, BingoOp flags)
      {
         bingoOperationDelegate opWithIndex =
            (ctx_conn, conn, index_data) =>
            {
               if (BingoCore.lib.bingoIndexBegin() != 1)
                  throw new Exception(BingoCore.lib.bingoGetError());

               try
               {
                  index_data.fingerprints.init(ctx_conn);
                  index_data.storage.validate(ctx_conn);

                  operationDelegate(ctx_conn, conn, index_data);

                  ContextFlags save_flags = ContextFlags.CMF;
                  saveContext(ctx_conn, bingo_schema.Value, index_data.id.table_id, save_flags);
               }
               finally
               {
                  BingoCore.lib.bingoIndexEnd();
               }
            };

         _ExecuteBingoOperation(bingo_schema,
            opWithIndex, getBingoDataDelegate, flags);
      }


      private static void CreateIndex (SqlString table, SqlString id_column, 
         SqlString data_column, SqlString bingo_schema, bool reaction)
      {
         bingoOperationDelegate opWithIndex = 
            getInsertRecordsDelegate(table.Value, true, true, true, null);

         bingoGetIndexDataDelegate createDataDelegate =
            (ctx_conn, conn, schema) => BingoIndexData.CreateIndexData(getSPID(ctx_conn),
                     conn, schema.Value, table.Value, id_column.Value, data_column.Value, reaction);

         _ExecuteBingoOperationChangeIndex(bingo_schema, opWithIndex,
            createDataDelegate, BingoOp.DROP_ON_EXCEPTION | BingoOp.NEED_STAT |
            BingoOp.NON_CONTEXT_CONN | BingoOp.LOCK_INDEX);
      }

      // Parameters:
      //    table:
      //       Table name for selecting records
      //    existing_cursor_name
      //       Existing cursor name for selecting records
      // Remarks:
      //    Only table or existing_cursor_name must be non null
      //
      private static bingoOperationDelegate getInsertRecordsDelegate (string table, 
         bool insert_records, bool flush_and_create_index, bool create_cursor, ArrayList error_list)
      {
         UTF8Encoding encoding = new UTF8Encoding();

         bingoOperationDelegate opWithIndex =
            (ctx_conn, conn, data) =>
            {
               // Process each molecule
               // If ThreadAbortException occurs then Thread.ResetAbort() is 
               // called and indexing is terminated 
               String select_command = 
                  String.Format("SELECT {0}, {1} FROM {2}", data.id_column, data.data_column, table);
               using (SqlCommand command = new SqlCommand(select_command, ctx_conn))
               {
                  command.CommandTimeout = 3600 * 10;
                  using (SqlDataReader cursor = command.ExecuteReader())
                  {
                     Exception exception = null;
                     int counter = 0;

                     BingoCore.GetNextRecordHandler get_next_record =
                        (IntPtr context) =>
                        {
                           int? id = null;
                           try
                           {
                              if (exception != null)
                                 return 0;
                              while (cursor.Read())
                              {
                                 if (cursor[0] == DBNull.Value)
                                 {
                                    string message =
                                       String.Format("Record with id=null was skipped.");
                                    if (SqlContext.Pipe != null)
                                       SqlContext.Pipe.Send(message);
                                    BingoLog.logMessage(message);
                                    continue;
                                 }
                                 id = Convert.ToInt32(cursor[0]);
                                 if (cursor[1] == DBNull.Value)
                                 {
                                    string message =
                                       String.Format("Record with id={0} has null data. Skipped.", id);
                                    if (SqlContext.Pipe != null)
                                       SqlContext.Pipe.Send(message);
                                    BingoLog.logMessage(message);
                                    if (error_list != null)
                                       error_list.Add(new FetchedData(id.Value) { str = "null data" });
                                    continue;
                                 }
                                 counter++;
                                 if (counter % 10000 == 0)
                                 {
                                    BingoLog.logMessage("Processing record #{0} with id = {1}",
                                       counter, id);
                                 }

                                 byte[] record_data;
                                 if (cursor[1].GetType() == typeof(byte[]))
                                    record_data = (byte[])cursor[1];
                                 else
                                    record_data = encoding.GetBytes((string)cursor[1]);

                                 BingoCore.lib.bingoSetIndexRecordData(id.Value, record_data, record_data.Length);

                                 return 1;
                              }
                              // No records more
                              return 0;
                           }
                           catch (Exception ex)
                           {
                              if ((Thread.CurrentThread.ThreadState & ThreadState.AbortRequested) != 0)
                                 Thread.ResetAbort();

                              if (id.HasValue)
                                 BingoLog.logMessage("Failed on id = {0}", id);
                              else
                                 BingoLog.logMessage("Exception {0} in {1}:\n{2}", 
                                    ex.Message, ex.Source, ex.StackTrace);

                              BingoCore.lib.bingoIndexMarkTermintate();
                              exception = ex;
                              return 0;
                           }
                        };

                     BingoCore.ProcessResultHandler process_result =
                        (IntPtr context) =>
                        {
                           try
                           {
                              if (exception != null)
                                 return;
                              if (insert_records)
                              {
                                 BingoTimer add_timer = new BingoTimer("index.add_to_index");
                                 if (data.getIndexType() == BingoIndexData.IndexType.Molecule)
                                    _AddMoleculeToIndex(conn, data);
                                 else
                                    _AddReactionToIndex(conn, data);
                                 add_timer.end();
                              }
                           }
                           catch (Exception ex)
                           {
                              BingoLog.logMessage("Exception {0} in {1}:\n{2}", 
                                 ex.Message, ex.Source, ex.StackTrace);
                              BingoCore.lib.bingoIndexMarkTermintate();
                              if ((Thread.CurrentThread.ThreadState & ThreadState.AbortRequested) != 0)
                                 Thread.ResetAbort();
                              exception = ex;
                           }
                        };

                     BingoCore.ProcessErrorHandler process_error =
                        (int id_with_error, IntPtr context) =>
                        {
                           try
                           {
                              if (exception != null)
                                 return;
                              string message =
                                 String.Format("Record with ID={0} wasn't added to the index: {1}",
                                    id_with_error, BingoCore.lib.bingoGetWarning());
                              if (SqlContext.Pipe != null)
                                 SqlContext.Pipe.Send(message);
                              BingoLog.logMessage(message);

                              if (error_list != null)
                                 error_list.Add(
                                    new FetchedData(id_with_error) { str = BingoCore.lib.bingoGetWarning() });
                           }
                           catch (Exception ex)
                           {
                              BingoLog.logMessage("Exception {0} in {1}:\n{2}", 
                                 ex.Message, ex.Source, ex.StackTrace);
                              BingoCore.lib.bingoIndexMarkTermintate();
                              if ((Thread.CurrentThread.ThreadState & ThreadState.AbortRequested) != 0)
                                 Thread.ResetAbort();
                              exception = ex;
                           }
                        };

                     try
                     {
                        int ret = BingoCore.lib.bingoIndexProcess(
                           data.getIndexType() == BingoIndexData.IndexType.Reaction,
                           get_next_record, process_result, process_error, IntPtr.Zero);
                        if (ret == -1)
                        {
                           string msg = BingoCore.lib.bingoGetError();
                           BingoLog.logMessage("Internal exception: {0}", BingoCore.lib.bingoGetError());
                           throw new Exception(msg);
                        }
                     }
                     catch (Exception ex)
                     {
                        // Terminate parallel indexing because it causes unhandled exception if not terminated
                        // Index termination should be here because function pointers must be valid
                        BingoCore.lib.bingoIndexEnd();
                        throw ex;
                     }

                     if (exception != null)
                        throw exception;
                  }
               }

               if (flush_and_create_index)
               {
                  data.flush(conn);

                  BingoTimer indices_timer = new BingoTimer("index.create_indices");
                  data.createIndices(conn);
                  indices_timer.end();

                  BingoTimer fp_indices_timer = new BingoTimer("index.create_fp_indices");
                  data.fingerprints.createIndices(conn);
                  fp_indices_timer.end();

                  data.CreateTriggers(conn);
               }
            };
         return opWithIndex;
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
        SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunction]
      public static void CreateMoleculeIndex (SqlString table, SqlString id_column,
         SqlString data_column, SqlString bingo_schema)
      {
         CreateIndex(table, id_column, data_column, bingo_schema, false);
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
        SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunction]
      public static void SetKeepCache (SqlString table, SqlBoolean value, SqlString bingo_schema)
      {
         bingoGetIndexDataDelegate getDataDelegate =
            (ctx_conn, conn, schema) =>
               BingoIndexData.GetIndexData(conn, bingo_schema.Value, table.Value, getSPID(ctx_conn));

         bingoOperationDelegate setOperation =
            (ctx_conn, conn, index_data) =>
            {
               index_data.setKeepCache(conn, bingo_schema.Value, value.Value);
            };

         _ExecuteBingoOperation(bingo_schema, setOperation, getDataDelegate, 0);
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
        SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunction]
      public static void CreateReactionIndex (SqlString table, SqlString id_column,
         SqlString data_column, SqlString bingo_schema)
      {
         CreateIndex(table, id_column, data_column, bingo_schema, true);
      }

      private static bool _AddMoleculeToIndex (SqlConnection conn, BingoIndexData bingo_data)
      {
         MangoIndexData data = (MangoIndexData)bingo_data;

         MangoIndex index = new MangoIndex();
         int id;
         if (!index.readPrepared(out id))
         {
            string message =
               String.Format("Molecule with ID={0} wasn't added to the index: {1}",
                  id, BingoCore.lib.bingoGetWarning());
            SqlContext.Pipe.Send(message);
            BingoLog.logMessage(message);
            return false;
         }
         // 4 bytes for Id, 2 bytes for similarity fingerprints bits counts
         byte[] new_data = new byte[index.cmf.Length + 4 + 2];
         index.cmf.CopyTo(new_data, 6);

         MemoryStream stream = new MemoryStream(new_data, 0, 6, true);
         BinaryWriter writer = new BinaryWriter(stream);
         writer.Write(id);
         writer.Write((short)index.sim_fp_bits_count);

         int storage_id = data.storage.add(new_data, conn);
         data.fingerprints.addFingerprint(conn, index.fingerprint, storage_id);

         data.addToShadowTable(conn, index, id, storage_id);
         return true;
      }

      private static bool _AddReactionToIndex (SqlConnection conn, BingoIndexData bingo_data)
      {
         RingoIndexData data = (RingoIndexData)bingo_data;

         int id;
         RingoIndex index = new RingoIndex();
         if (!index.readPrepared(out id))
         {
            string message =
               String.Format("Reaction with ID={0} wasn't added to the index: {1}",
                  id, BingoCore.lib.bingoGetWarning());
            SqlContext.Pipe.Send(message);
            BingoLog.logMessage(message);
            return false;
         }

         byte[] new_data = new byte[index.crf.Length + 4];
         index.crf.CopyTo(new_data, 4);

         MemoryStream stream = new MemoryStream(new_data, 0, 4, true);
         BinaryWriter writer = new BinaryWriter(stream);
         writer.Write(id);

         int storage_id = data.storage.add(new_data, conn);
         data.fingerprints.addFingerprint(conn, index.fingerprint, storage_id);

         data.addToShadowTable(conn, index, id, storage_id);
         return false;
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
        SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunction]
      public static void DropIndex (SqlString table, SqlString bingo_schema)
      {
         using (SqlConnection conn = new SqlConnection("context connection=true"))
         {
            conn.Open();        
            BingoIndexData.DropIndexData(conn, bingo_schema.Value, table.Value, true);
         }
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
        SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunction]
      public static void _DropIndexByID (SqlInt32 table_id, SqlInt32 database_id, SqlString bingo_schema)
      {
         using (SqlConnection conn = new SqlConnection("context connection=true"))
         {
            conn.Open();
            BingoIndexID id = new BingoIndexID(table_id.Value, database_id.Value);
            BingoIndexData.DropIndexData(conn, bingo_schema.Value, id, false);
         }
      }

      public static void FillRowInt (Object obj, out SqlInt32 id)
      {
         FetchedData data = (FetchedData)obj;
         id = new SqlInt32((int)data.id);
      }

      public static void FillRowIntString (Object obj, out SqlInt32 id, out SqlString str)
      {
         FetchedData data = (FetchedData)obj;
         id = new SqlInt32(data.id);
         str = new SqlString(data.str);
      }
      public static void FillRowIntFloat (Object obj, out SqlInt32 id, out SqlDouble value)
      {
         FetchedData data = (FetchedData)obj;
         id = new SqlInt32(data.id);
         value = new SqlDouble(data.value);
      }

      private static Dictionary<string, string> ParseOptions (ref string options)
      {
         string specific_options = null;
         Dictionary<string, string> result = new Dictionary<string,string>();

         string[] allowed_parameters = { "TOP", "NEXT", "START" };

         foreach (string part in options.Split(';'))
         {
            string part_trimmed = part.TrimStart();

            bool found = false;
            foreach (string p in allowed_parameters)
               if (part_trimmed.StartsWith(p, StringComparison.OrdinalIgnoreCase))
               {
                  if (result.ContainsKey(p))
                     throw new Exception(String.Format("Only one specification of '{0}' option is allowed", p));
                  result.Add(p, part_trimmed.Substring(p.Length).Trim());
                  found = true;
                  break;
               }
            if (!found)
            {
               if (specific_options != null)
                  throw new Exception(String.Format("Search-specific option was specified twice: '{0}' and '{1}'",
                     specific_options, part_trimmed));

               specific_options = part_trimmed;
            }
         }

         options = specific_options;
         return result;
      }

      private static IEnumerable _MakeSearch (SqlString query, SqlString table,
                 SqlString options,    SqlString bingo_schema, string search_type,
                  bool highlighting, params object[] ext_options)
      {
         BingoLog.logMessage("{0} query started for {1} table", search_type, table.Value);

         ArrayList res_list = new ArrayList();

         BingoOp op_flags = BingoOp.NEED_STAT | BingoOp.LOAD_CMF;
         if (search_type != "SIM")
         {
            if (options.Value.Contains("TAU"))
               op_flags |= BingoOp.LOAD_TAU_RULES;
         }

         bingoGetIndexDataDelegate getDataDelegate = 
            (ctx_conn, conn, schema) => 
               BingoIndexData.GetIndexData(conn, bingo_schema.Value, table.Value, getSPID(ctx_conn));

         bingoOperationDelegate searchOperation =
            (ctx_conn, conn, index_data) =>
            {
               if (index_data.needFlush())
                  throw new Exception("Index has been changed. FlushOperations must be called before search");

               string options_str = options.Value;

               Dictionary<string, string> common_options = ParseOptions(ref options_str);

               int max_count = -1;
               if (common_options.ContainsKey("TOP"))
               {
                  max_count = Convert.ToInt32(common_options["TOP"]);
                  if (max_count < 0)
                     throw new Exception("Limit for 'TOP' option cannot be negative");
               }

               int? next_from = null;
               if (common_options.ContainsKey("NEXT"))
                  next_from = Convert.ToInt32(common_options["NEXT"]);
               if (common_options.ContainsKey("START"))
                  next_from = -1;

               IEnumerable<FetchedData> fetched;
               fetched = _Fetch(query, search_type, highlighting, ext_options,
                  conn, index_data, options_str, next_from);

               foreach (FetchedData id in fetched)
               {
                  res_list.Add(id);
                  if (res_list.Count == max_count)
                     break;
               }
            };


         _ExecuteBingoOperation(bingo_schema, searchOperation, getDataDelegate, op_flags);

         return res_list;
      }

      private static IEnumerable<FetchedData> _Fetch (SqlString query, string search_type,
         bool highlighting, object[] ext_options, SqlConnection conn,
         BingoIndexData index_data, string options_str, int? id_next_from)
      {
         int? storage_id_next_from = null;
         if (id_next_from.HasValue)
         {
            // -1 means start of the iterations
            if (id_next_from.Value != -1)
            {
               storage_id_next_from = index_data.getStorageIdById(conn, id_next_from.Value);
               if (!storage_id_next_from.HasValue)
                  throw new Exception(String.Format("Cannot find record with id={0}", id_next_from));
            }
            else
               storage_id_next_from = -1;
         }

         IEnumerable<FetchedData> fetched;
         if (search_type == "SUB" || search_type == "SMARTS")
         {
            MangoFastIndexFetch fetch_sub = new MangoFastIndexFetch((MangoIndexData)index_data);
            fetch_sub.prepareSub(query.Value, options_str, highlighting, search_type == "SMARTS");
            fetch_sub.nextAfterStorageId = storage_id_next_from;
            fetched = fetch_sub.fetch(conn);
         }
         else if (search_type == "EXACT")
         {
            MangoShadowFetch fetch_exact = new MangoShadowFetch((MangoIndexData)index_data);
            fetch_exact.prepareExact(query.Value, options_str);
            fetch_exact.nextAfterStorageId = storage_id_next_from;
            fetched = fetch_exact.fetch(conn);
         }
         else if (search_type == "SIM")
         {
            MangoFastIndexFetch fetch_sim = new MangoFastIndexFetch((MangoIndexData)index_data);
            double min = (double)ext_options[0];
            double max = (double)ext_options[1];

            fetch_sim.prepareSimilarity(query.Value, options_str, min, max);
            fetch_sim.nextAfterStorageId = storage_id_next_from;
            fetched = fetch_sim.fetch(conn);
         }
         else if (search_type == "MASS")
         {
            MangoShadowFetch fetch_mass = new MangoShadowFetch((MangoIndexData)index_data);
            double? min = (double?)ext_options[0];
            double? max = (double?)ext_options[1];

            fetch_mass.prepareMass(min, max);
            fetch_mass.nextAfterStorageId = storage_id_next_from;
            fetched = fetch_mass.fetch(conn);
         }
         else if (search_type == "GROSS")
         {
            MangoShadowFetch fetch_gross = new MangoShadowFetch((MangoIndexData)index_data);
            fetch_gross.prepareGross(query.Value);
            fetch_gross.nextAfterStorageId = storage_id_next_from;
            fetched = fetch_gross.fetch(conn);
         }
         else if (search_type == "RSUB" || search_type == "RSMARTS")
         {
            RingoFastIndexFetch fetch_sub = new RingoFastIndexFetch((RingoIndexData)index_data);
            fetch_sub.prepareSub(query.Value, options_str, highlighting, search_type == "RSMARTS");
            fetch_sub.nextAfterStorageId = storage_id_next_from;
            fetched = fetch_sub.fetch(conn);
         }
         else if (search_type == "REXACT")
         {
            RingoShadowFetch fetch_exact = new RingoShadowFetch((RingoIndexData)index_data);
            fetch_exact.prepareExact(query.Value, options_str);
            fetch_exact.nextAfterStorageId = storage_id_next_from;
            fetched = fetch_exact.fetch(conn);
         }
         else
            throw new Exception("Unknown search type: " + search_type);
         return fetched;
      }

      [SqlFunction(FillRowMethodName = "FillRowInt",
         DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read,
         TableDefinition = "id int")]
      [BingoSqlFunctionForReader]
      public static IEnumerable SearchSub (SqlString table, SqlString query,
                 SqlString options, SqlString bingo_schema)
      {                  
         return _MakeSearch(query, table, options, bingo_schema, "SUB", false);
      }

      [SqlFunction(FillRowMethodName = "FillRowIntString",
         DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read,
         TableDefinition = "id int, highlighting nvarchar(max)")]
      [BingoSqlFunctionForReader]
      public static IEnumerable SearchSubHi (SqlString table, SqlString query,
                 SqlString options, SqlString bingo_schema)
      {
         return _MakeSearch(query, table, options, bingo_schema, "SUB", true);
      }

      [SqlFunction(FillRowMethodName = "FillRowInt",
         DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read,
         TableDefinition = "id int")]
      [BingoSqlFunctionForReader]
      public static IEnumerable SearchSMARTS (SqlString table, SqlString query,
                 SqlString options, SqlString bingo_schema)
      {
         return _MakeSearch(query, table, options, bingo_schema, "SMARTS", false);
      }

      [SqlFunction(FillRowMethodName = "FillRowIntString",
         DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read,
         TableDefinition = "id int, highlighting nvarchar(max)")]
      [BingoSqlFunctionForReader]
      public static IEnumerable SearchSMARTSHi (SqlString table, SqlString query,
                 SqlString options, SqlString bingo_schema)
      {
         return _MakeSearch(query, table, options, bingo_schema, "SMARTS", true);
      }

      [SqlFunction(FillRowMethodName = "FillRowInt",
         DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read,
         TableDefinition = "id int")]
      [BingoSqlFunctionForReader]
      public static IEnumerable SearchRSub (SqlString table, SqlString query,
                 SqlString options, SqlString bingo_schema)
      {
         return _MakeSearch(query, table, options, bingo_schema, "RSUB", false);
      }

      [SqlFunction(FillRowMethodName = "FillRowIntString",
         DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read,
         TableDefinition = "id int, highlighting nvarchar(max)")]
      [BingoSqlFunctionForReader]
      public static IEnumerable SearchRSubHi (SqlString table, SqlString query,
                 SqlString options, SqlString bingo_schema)
      {
         return _MakeSearch(query, table, options, bingo_schema, "RSUB", true);
      }

      [SqlFunction(FillRowMethodName = "FillRowInt",
         DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read,
         TableDefinition = "id int")]
      [BingoSqlFunctionForReader]
      public static IEnumerable SearchRSMARTS (SqlString table, SqlString query,
                 SqlString options, SqlString bingo_schema)
      {
         return _MakeSearch(query, table, options, bingo_schema, "RSMARTS", false);
      }

      [SqlFunction(FillRowMethodName = "FillRowIntString",
         DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read,
         TableDefinition = "id int, highlighting nvarchar(max)")]
      [BingoSqlFunctionForReader]
      public static IEnumerable SearchRSMARTSHi (SqlString table, SqlString query,
                 SqlString options, SqlString bingo_schema)
      {
         return _MakeSearch(query, table, options, bingo_schema, "RSMARTS", true);
      }

      [SqlFunction(FillRowMethodName = "FillRowInt",
         DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read,
         TableDefinition = "id int")]
      [BingoSqlFunctionForReader]
      public static IEnumerable SearchExact (SqlString table, SqlString query,
                 SqlString options, SqlString bingo_schema)
      {
         return _MakeSearch(query, table, options, bingo_schema, "EXACT", false);
      }

      [SqlFunction(FillRowMethodName = "FillRowInt",
         DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read,
         TableDefinition = "id int")]
      [BingoSqlFunctionForReader]
      public static IEnumerable SearchRExact (SqlString table, SqlString query,
                 SqlString options, SqlString bingo_schema)
      {
         return _MakeSearch(query, table, options, bingo_schema, "REXACT", false);
      }


      [SqlFunction(FillRowMethodName = "FillRowInt",
         DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read,
         TableDefinition = "id int")]
      [BingoSqlFunctionForReader]
      public static IEnumerable SearchGross (SqlString table, SqlString query,
                 SqlString options, SqlString bingo_schema)
      {
         return _MakeSearch(query, table, options, bingo_schema, "GROSS", false);
      }

      [SqlFunction(FillRowMethodName = "FillRowIntFloat",
         DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read,
         TableDefinition = "id int, similarity real")]
      [BingoSqlFunctionForReader]
      public static IEnumerable SearchSim (SqlString table, SqlString query,
                 SqlString metric, SqlString bingo_schema, SqlDouble min_bound, SqlDouble max_bound)
      {
         double min = -0.1f, max = 1.1f;
         if (!min_bound.IsNull)
            min = (double)min_bound.Value;
         if (!max_bound.IsNull)
            max = (double)max_bound.Value;

         return _MakeSearch(query, table, metric, bingo_schema, "SIM", false, min, max);
      }

      [SqlFunction(FillRowMethodName = "FillRowIntFloat",
         DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read,
         TableDefinition = "id int, weight real")]
      [BingoSqlFunctionForReader]
      public static IEnumerable SearchMolecularWeight (SqlString table, SqlDouble min_bound,
         SqlDouble max_bound, SqlString options, SqlString bingo_schema)
      {
         double? min = null, max = null;
         if (!min_bound.IsNull)
            min = (double)min_bound.Value;
         if (!max_bound.IsNull)
            max = (double)max_bound.Value;

         return _MakeSearch(null, table, options.Value, bingo_schema, "MASS", false, min, max);
      }

      private delegate bool bingoImportPopulateDataRow 
         (DataRow row, List<string[]> parameters);

      private static void _ImportData (string table_name, string data_column_name,
         string additional_parameters, bingoImportPopulateDataRow populateRowFunc)
      {
         using (SqlConnection ctx_conn = new SqlConnection("context connection=true"))
         {
            ctx_conn.Open();

            List<string[]> parameters = new List<string[]>();
            string[] params_array = additional_parameters.Split(new char[] {',', ';'});
            foreach (string p in params_array)
            {
               if (p != "")
               {
                  string[] name_and_column = p.Trim().Split(' ');
                  parameters.Add(name_and_column);
               }
            }

            DataTable dt = new DataTable();

            dt.Columns.Add(new DataColumn(data_column_name, 
               Type.GetType("System.String")));

            foreach (string[] p in parameters)
               dt.Columns.Add(new DataColumn(p[1], Type.GetType("System.String")));

            using (BingoSession session = new BingoSession())
            {
               BingoCore.lib.bingoProfilingReset(true);
               BingoTimer timer = new BingoTimer("total");

               SqlConnection ext_conn = null;
               try
               {
                  string conn_string = String.Format(
                     "server={0};integrated security=true;database={1};enlist=false",
                     getServername(ctx_conn), ctx_conn.Database);
                  ext_conn = new SqlConnection(conn_string);
                  ext_conn.Open();

                  int imported_count = 0;

                  bool has_data = false;
                  do
                  {
                     DataRow new_row = dt.NewRow();
                     has_data = populateRowFunc(new_row, parameters);
                     if (has_data)
                     {
                        dt.Rows.Add(new_row);
                        imported_count++;
                     }

                     if (dt.Rows.Count >= 10000 || !has_data)
                     {
                        // Flush data table via SqlBulkCopy
                        BingoTimer timer_sql = new BingoTimer("import.sql_bulk_copy");

                        using (SqlTransaction transaction =
                                 ext_conn.BeginTransaction())
                        {
                           using (SqlBulkCopy bulkCopy = new SqlBulkCopy(ext_conn, 
                              SqlBulkCopyOptions.FireTriggers, transaction))
                           {
                              bulkCopy.DestinationTableName = table_name;
                              bulkCopy.ColumnMappings.Add(data_column_name, data_column_name);
                              foreach (string[] p in parameters)
                                 bulkCopy.ColumnMappings.Add(p[1], p[1]);

                              bulkCopy.BatchSize = dt.Rows.Count;
                              bulkCopy.BulkCopyTimeout = 3600;
                              bulkCopy.WriteToServer(dt);
                           }
                           transaction.Commit();
                        }
                        timer_sql.end();

                        BingoCore.lib.bingoProfIncCounter("import.sql_bulk_copy_size", dt.Rows.Count);
                        dt.Rows.Clear();

                        BingoLog.logMessage("  {0} molecules imported", imported_count);
                        BingoLog.logMessage("Intermediate statistics for import into table {0}:\n{1}\n",
                           table_name, BingoCore.bingoProfilingGetStatistics(true));
                        BingoCore.lib.bingoProfilingReset(false);
                     }
                  } while (has_data);

                  BingoLog.logMessage("  Done.");
                  timer.end();

                  BingoLog.logMessage("Statistics for import into table {0}:\n{1}\n",
                     table_name, BingoCore.bingoProfilingGetStatistics(true));
               }
               catch (Exception ex)
               {
                  BingoLog.logMessage("Exception {0} in {1}:\n{2}", ex.Message, ex.Source, ex.StackTrace);
                  throw;
               }
               finally
               {
                  if (ext_conn != null)
                     ext_conn.Close();
                  // Close import
                  BingoCore.lib.bingoSDFImportClose();
                  BingoCore.lib.bingoRDFImportClose();
                  BingoCore.lib.bingoSMILESImportClose();
               }
            }
         }
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunction]
      public static void ImportSDF(SqlString table_name, SqlString mol_column_name,
         SqlString file_name, SqlString additional_parameters, SqlString bingo_schema)
      {
         bool reader_opened = false;

         bingoImportPopulateDataRow populateRowFunc =
            (row, parameters) =>
            {
               if (!reader_opened)
               {
                  if (BingoCore.lib.bingoSDFImportOpen(file_name.Value) != 1)
                     throw new Exception(BingoCore.lib.bingoGetError());
                  reader_opened = true;
               }

               if (BingoCore.lib.bingoSDFImportEOF() != 0)
                  return false;

               BingoTimer timer_mol = new BingoTimer("import.read_mol");
               row[mol_column_name.Value] = BingoCore.bingoSDFImportGetNext();
               timer_mol.end();

               foreach (string[] p in parameters)
                  row[p[1]] = BingoCore.bingoSDFImportGetParameter(p[0]);

               return true;
            };

         BingoLog.logMessage("Importing into {0} from {1}", table_name, file_name);
         _ImportData(table_name.Value, mol_column_name.Value, 
            additional_parameters.Value, populateRowFunc);
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunction]
      public static void ImportRDF (SqlString table_name, SqlString react_column_name,
         SqlString file_name, SqlString additional_parameters, SqlString bingo_schema)
      {
         bool reader_opened = false;

         bingoImportPopulateDataRow populateRowFunc =
            (row, parameters) =>
            {
               if (!reader_opened)
               {
                  if (BingoCore.lib.bingoRDFImportOpen(file_name.Value) != 1)
                     throw new Exception(BingoCore.lib.bingoGetError());
                  reader_opened = true;
               }

               if (BingoCore.lib.bingoRDFImportEOF() != 0)
                  return false;

               BingoTimer timer_mol = new BingoTimer("import.read_mol");
               row[react_column_name.Value] = BingoCore.bingoRDFImportGetNext();
               timer_mol.end();

               foreach (string[] p in parameters)
                  row[p[1]] = BingoCore.bingoRDFImportGetParameter(p[0]);

               return true;
            };

         BingoLog.logMessage("Importing into {0} from {1}", table_name, file_name);
         _ImportData(table_name.Value, react_column_name.Value,
            additional_parameters.Value, populateRowFunc);
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunction]
      public static void ImportSMILES (SqlString table_name, SqlString mol_column_name,
         SqlString file_name, SqlString id_column_name, SqlString bingo_schema)
      {
         bool have_id_col = !id_column_name.IsNull && id_column_name.Value.Length > 1;

         bool reader_opened = false;

         Regex regex1 = new Regex(@"[\s]*[^\s]*[\s]*\|.*\|[\s]+([^\s].*)", RegexOptions.Compiled);
         Regex regex2 = new Regex(@"[\s]*[^\s]*[\s]+([^\s\|].*)", RegexOptions.Compiled);

         bingoImportPopulateDataRow populateRowFunc =
               (row, parameters) =>
               {
                  if (!reader_opened)
                  {
                     if (BingoCore.lib.bingoSMILESImportOpen(file_name.Value) != 1)
                        throw new Exception(BingoCore.lib.bingoGetError());
                     reader_opened = true;
                  }
                  if (BingoCore.lib.bingoSMILESImportEOF() != 0)
                     return false;

                  String line = BingoCore.lib.bingoSMILESImportGetNext();
                  row[mol_column_name.Value] = line;

                  if (have_id_col)
                  {
                     Match match = regex1.Match(line);

                     string id_data;

                     if (match != Match.Empty)
                        id_data = match.Groups[1].Value;
                     else
                     {
                        match = regex2.Match(line);

                        if (match != Match.Empty)
                           id_data = match.Groups[1].Value;
                        else
                           id_data = null;
                     }
                     row[id_column_name.Value] = id_data;
                  }

                  return true;
               };

         string additional_parameters = "";
         if (have_id_col)
            additional_parameters = id_column_name.Value + " " + id_column_name.Value;

         _ImportData(table_name.Value, mol_column_name.Value, additional_parameters, populateRowFunc);
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunction]
      public static void ExportSDF (SqlString table_name, SqlString mol_column_name,
         SqlString file_name, SqlString additional_parameters)
      {
         FileInfo exported_file_info = new FileInfo(file_name.Value);
         using (StreamWriter exported_file = exported_file_info.CreateText())
         {
            using (SqlConnection conn = new SqlConnection("context connection=true"))
            {
               conn.Open();

               string parameters_str = additional_parameters.Value;
               if (parameters_str.Length > 0)
                  parameters_str = ", " + parameters_str;

               SqlDataReader reader =
                  BingoSqlUtils.ExecReader(conn, "select {0}{1} from {2}", 
                  mol_column_name, parameters_str, table_name);
               using (reader)
               {
                  while (reader.Read())
                  {
                     exported_file.WriteLine(reader[0]);
                     for (int i = 1; i < reader.FieldCount; i++)
                     {
                        exported_file.WriteLine(">  <{0}>", reader.GetName(i));
                        exported_file.WriteLine("{0}", reader[i]);
                        exported_file.WriteLine("");
                     }

                     exported_file.WriteLine("$$$$");
                  }
               }
            }
         }
      }

      [Flags]
      enum ContextFlags
      {
         TAU_RULES = 0x01,
         FINGERPRINTS = 0x02,
         CMF = 0x04,
         NTHREADS = 0x08,
      };

      private static void prepareContext (SqlConnection connection, string bingo_schema,
         int id, ContextFlags flags)
      {
         BingoCore.setContext(id);

         // Set basic paremeters for every request
         // TODO: cache settings
         BingoCore.setConfigInt("treat-x-as-pseudoatom",
             BingoConfig.getInt(connection, bingo_schema, "treat-x-as-pseudoatom", id));
         BingoCore.setConfigInt("ignore-stereocenter-errors",
             BingoConfig.getInt(connection, bingo_schema, "ignore-stereocenter-errors", id));
         BingoCore.setConfigInt("ignore-cistrans-errors",
             BingoConfig.getInt(connection, bingo_schema, "ignore-cistrans-errors", id));
         BingoCore.setConfigInt("allow-non-unique-dearomatization",
             BingoConfig.getInt(connection, bingo_schema, "allow-non-unique-dearomatization", id));
         BingoCore.setConfigInt("zero-unknown-aromatic-hydrogens",
             BingoConfig.getInt(connection, bingo_schema, "zero-unknown-aromatic-hydrogens", id));
         BingoCore.setConfigInt("stereochemistry-bidirectional-mode",
             BingoConfig.getInt(connection, bingo_schema, "stereochemistry-bidirectional-mode", id));
         BingoCore.setConfigInt("stereochemistry-detect-haworth-projection",
             BingoConfig.getInt(connection, bingo_schema, "stereochemistry-detect-haworth-projection", id));
         BingoCore.setConfigInt("ignore-closing-bond-direction-mismatch",
             BingoConfig.getInt(connection, bingo_schema, "ignore-closing-bond-direction-mismatch", id));
         BingoCore.setConfigInt("reject-invalid-structures",
             BingoConfig.getInt(connection, bingo_schema, "reject-invalid-structures", id));
         BingoCore.setConfigInt("ignore-bad-valence",
             BingoConfig.getInt(connection, bingo_schema, "ignore-bad-valence", id));

         if ((flags & ContextFlags.NTHREADS) != 0)
         {
            BingoCore.setConfigInt("nthreads",
                BingoConfig.getInt(connection, bingo_schema, "nthreads", id));
         }

         if ((flags & ContextFlags.TAU_RULES) != 0)
         {
            ArrayList rules = BingoConfig.getTautomerRules(connection, bingo_schema);

            BingoCore.clearTautomerRules();
            foreach (BingoConfig.TautomerRule rule in rules)
            {
               BingoCore.addTautomerRule(rule.n, rule.beg, rule.end);
            }
            BingoCore.tautomerRulesReady();
         }

         if ((flags & ContextFlags.FINGERPRINTS) != 0)
         {
            BingoCore.setConfigInt("FP_ORD_SIZE",
                BingoConfig.getInt(connection, bingo_schema, "FP_ORD_SIZE", id));
            BingoCore.setConfigInt("FP_ANY_SIZE",
                BingoConfig.getInt(connection, bingo_schema, "FP_ANY_SIZE", id));
            BingoCore.setConfigInt("FP_TAU_SIZE",
                BingoConfig.getInt(connection, bingo_schema, "FP_TAU_SIZE", id));
            BingoCore.setConfigInt("FP_SIM_SIZE",
                BingoConfig.getInt(connection, bingo_schema, "FP_SIM_SIZE", id));

            BingoCore.setConfigInt("SUB_SCREENING_MAX_BITS",
                BingoConfig.getInt(connection, bingo_schema, "SUB_SCREENING_MAX_BITS", id));
            BingoCore.setConfigInt("SIM_SCREENING_PASS_MARK",
                BingoConfig.getInt(connection, bingo_schema, "SIM_SCREENING_PASS_MARK", id));
         }
         if ((flags & ContextFlags.CMF) != 0)
         {
            BingoCore.setConfigBin("cmf-dict",
                BingoConfig.getBinary(connection, bingo_schema, "cmf-dict", id));
         }
      }

      private static void saveContext (SqlConnection connection, string bingo_schema,
         int id, ContextFlags flags)
      {
         BingoCore.setContext(id);

         if ((flags & ContextFlags.CMF) != 0)
         {
            BingoConfig.setBinary(connection, bingo_schema, "cmf-dict", id, 
               BingoCore.getConfigBin("cmf-dict"));
         }
      }

      [SqlProcedure]
      [BingoSqlFunction(access_level = AccessLevelKind.None)]
      public static void OnSessionClose (SqlString spid_str)
      {
         int spid = Convert.ToInt32(spid_str.Value);

         BingoIndexData.OnSessionClose(spid);
      }

      private static int getSPID (SqlConnection conn)
      {
         return BingoSqlUtils.ExecIntQuery(conn, "SELECT @@spid AS spid").Value;
      }

      private static string getServername (SqlConnection conn)
      {
         return BingoSqlUtils.ExecStringQuery(conn, "SELECT @@SERVERNAME AS spid");
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "molecule")]
      public static SqlString Smiles (SqlBinary molecule, SqlString bingo_schema)
      {
         using (BingoSession session = new BingoSession())
         {
            using (SqlConnection conn = new SqlConnection("context connection=true"))
            {
               conn.Open();
               prepareContext(conn, bingo_schema.Value, 0, 0);
            }

            return BingoCore.mangoSMILES(molecule.Value, false);
         }
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "reaction")]
      public static SqlString RSmiles (SqlBinary reaction, SqlString bingo_schema)
      {
         using (BingoSession session = new BingoSession())
         {
            using (SqlConnection conn = new SqlConnection("context connection=true"))
            {
               conn.Open();
               prepareContext(conn, bingo_schema.Value, 0, 0);
            }

            return BingoCore.ringoRSMILES(reaction.Value);
         }
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "molecule")]
      public static SqlString Molfile (SqlBinary molecule, SqlString bingo_schema)
      {
         using (BingoSession session = new BingoSession())
         {
            using (SqlConnection conn = new SqlConnection("context connection=true"))
            {
               conn.Open();
               prepareContext(conn, bingo_schema.Value, 0, 0);
            }

            return BingoCore.mangoMolfile(molecule.Value);
         }
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "molecule")]
      public static SqlString InChI(SqlBinary molecule, SqlString options, SqlString bingo_schema)
      {
         using (BingoSession session = new BingoSession())
         {
            using (SqlConnection conn = new SqlConnection("context connection=true"))
            {
               conn.Open();
               prepareContext(conn, bingo_schema.Value, 0, 0);
            }

            return BingoCore.mangoInChI(molecule.Value, options.Value);
         }
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader]
      public static SqlString InChIKey(SqlString inchi, SqlString bingo_schema)
      {
         using (BingoSession session = new BingoSession())
         {
            return BingoCore.mangoInChIKey(inchi.Value);
         }
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "molecule")]
      public static SqlBinary Fingerprint(SqlBinary molecule, SqlString options, SqlString bingo_schema)
      {
         using (BingoSession session = new BingoSession())
         {
            using (SqlConnection conn = new SqlConnection("context connection=true"))
            {
               conn.Open();
               prepareContext(conn, bingo_schema.Value, 0, ContextFlags.FINGERPRINTS);
            }

            return BingoCore.mangoFingerprint(molecule.Value, options.Value);
         }
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "reaction")]
      public static SqlBinary RFingerprint(SqlBinary reaction, SqlString options, SqlString bingo_schema)
      {
         using (BingoSession session = new BingoSession())
         {
            using (SqlConnection conn = new SqlConnection("context connection=true"))
            {
               conn.Open();
               prepareContext(conn, bingo_schema.Value, 0, ContextFlags.FINGERPRINTS);
            }

            return BingoCore.ringoFingerprint(reaction.Value, options.Value);
         }
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "molecule")]
      public static SqlString CML (SqlBinary molecule, SqlString bingo_schema)
      {
         using (BingoSession session = new BingoSession())
         {
            using (SqlConnection conn = new SqlConnection("context connection=true"))
            {
               conn.Open();
               prepareContext(conn, bingo_schema.Value, 0, 0);
            }

            return BingoCore.mangoCML(molecule.Value);
         }
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "reaction")]
      public static SqlString RCML (SqlBinary reaction, SqlString bingo_schema)
      {
         using (BingoSession session = new BingoSession())
         {
            using (SqlConnection conn = new SqlConnection("context connection=true"))
            {
               conn.Open();
               prepareContext(conn, bingo_schema.Value, 0, 0);
            }

            return BingoCore.ringoRCML(reaction.Value);
         }
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "reaction")]
      public static SqlString Rxnfile (SqlBinary reaction, SqlString bingo_schema)
      {
         using (BingoSession session = new BingoSession())
         {
            using (SqlConnection conn = new SqlConnection("context connection=true"))
            {
               conn.Open();
               prepareContext(conn, bingo_schema.Value, 0, 0);
            }

            return BingoCore.ringoRxnfile(reaction.Value);
         }
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "molecule")]
      public static SqlString CanSmiles (SqlBinary molecule, SqlString bingo_schema)
      {
         using (BingoSession session = new BingoSession())
         {
            using (SqlConnection conn = new SqlConnection("context connection=true"))
            {
               conn.Open();
               prepareContext(conn, bingo_schema.Value, 0, 0);
            }

            return BingoCore.mangoSMILES(molecule.Value, true);
         }
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "molecule")]
      public static SqlBinary CompactMolecule (SqlBinary molecule, SqlBoolean save_xyz, SqlString bingo_schema)
      {
         using (BingoSession session = new BingoSession())
         {
            using (SqlConnection conn = new SqlConnection("context connection=true"))
            {
               conn.Open();
               prepareContext(conn, bingo_schema.Value, 0, 0);
            }

            return BingoCore.mangoICM(molecule.Value, save_xyz.Value);
         }
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "reaction")]
      public static SqlBinary CompactReaction (SqlBinary reaction, SqlBoolean save_xyz, SqlString bingo_schema)
      {
         using (BingoSession session = new BingoSession())
         {
            using (SqlConnection conn = new SqlConnection("context connection=true"))
            {
               conn.Open();
               prepareContext(conn, bingo_schema.Value, 0, 0);
            }

            return BingoCore.ringoICR(reaction.Value, save_xyz.Value);
         }
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "molecule")]
      public static SqlDouble Mass (SqlBinary molecule, SqlString type, SqlString bingo_schema)
      {
         using (BingoSession session = new BingoSession())
         {
            using (SqlConnection conn = new SqlConnection("context connection=true"))
            {
               conn.Open();
               prepareContext(conn, bingo_schema.Value, 0, 0);
            }

            double mass;
            int ret = BingoCore.lib.mangoMass(molecule.Value, molecule.Value.Length, type.Value, out mass);
            if (ret != 1)
               return SqlDouble.Null;
            return mass;
         }
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "molecule")]
      public static SqlString Gross (SqlBinary molecule, SqlString bingo_schema)
      {
         using (BingoSession session = new BingoSession())
         {
            using (SqlConnection conn = new SqlConnection("context connection=true"))
            {
               conn.Open();
               prepareContext(conn, bingo_schema.Value, 0, 0);
            }

            return BingoCore.mangoGross(molecule.Value);
         }
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "molecule")]
      public static SqlString Name(SqlBinary molecule, SqlString bingo_schema)
      {
         using (BingoSession session = new BingoSession())
         {
            return BingoCore.bingoGetNameCore(molecule.Value);
         }
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
        SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunction]
      public static void _OnInsertRecordTrigger (SqlInt32 table_id, SqlInt32 database_id,
         SqlString tmp_table_name, SqlString bingo_schema)
      {
         bingoGetIndexDataDelegate getDataDelegate =
            (ctx_conn, conn, schema) => BingoIndexData.GetIndexData(conn, bingo_schema.Value,
               table_id.Value, database_id.Value, getSPID(ctx_conn));

         bingoOperationDelegate insertOp = getInsertRecordsDelegate(tmp_table_name.Value, true, false, true, null);

         _ExecuteBingoOperationChangeIndex(bingo_schema, insertOp,
            getDataDelegate, BingoOp.LOAD_CMF | BingoOp.NON_CONTEXT_CONN | BingoOp.LOCK_INDEX);
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
        SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunction]
      public static void _OnDeleteRecordTrigger (SqlInt32 table_id, SqlInt32 database_id,
         SqlString tmp_table_name, SqlString bingo_schema)
      {
         bingoGetIndexDataDelegate getDataDelegate =
            (ctx_conn, conn, schema) => BingoIndexData.GetIndexData(conn, bingo_schema.Value,
               table_id.Value, database_id.Value, getSPID(ctx_conn));

         bingoOperationDelegate deleteOp =
            (ctx_conn, conn, index_data) =>
            {
               index_data.prepareForDeleteRecord(conn);
               String select_command = 
                  String.Format("SELECT {0} FROM {1}", index_data.id_column, tmp_table_name.Value);
               using (SqlCommand command = new SqlCommand(select_command, ctx_conn))
               {
                  command.CommandTimeout = 3600 * 10;
                  using (SqlDataReader cursor = command.ExecuteReader())
                  {
                     while (cursor.Read())
                     {
                        int id = Convert.ToInt32(cursor[0]);
                        index_data.deleteRecordById(id, conn);
                     }
                  }
               }
            };

         _ExecuteBingoOperationChangeIndex(bingo_schema, deleteOp,
            getDataDelegate, BingoOp.LOAD_CMF | BingoOp.LOCK_INDEX | BingoOp.NON_CONTEXT_CONN);
      }

      [SqlFunction(FillRowMethodName = "FillRowIntString",
         DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read,
         TableDefinition = "id int, msg nvarchar(max)")]
      [BingoSqlFunctionForReader]
      public static IEnumerable CheckMoleculeTable (SqlString table, SqlString id_column,
                 SqlString data_column, SqlString bingo_schema)
      {
         return CheckChemicalTable(table, id_column, data_column, bingo_schema, false);
      }

      [SqlFunction(FillRowMethodName = "FillRowIntString",
         DataAccess = DataAccessKind.Read,
         SystemDataAccess = SystemDataAccessKind.Read,
         TableDefinition = "id int, msg nvarchar(max)")]
      [BingoSqlFunctionForReader]
      public static IEnumerable CheckReactionTable (SqlString table, SqlString id_column,
                 SqlString data_column, SqlString bingo_schema)
      {
         return CheckChemicalTable(table, id_column, data_column, bingo_schema, true);
      }

      private static IEnumerable CheckChemicalTable (SqlString table, SqlString id_column,
                 SqlString data_column, SqlString bingo_schema, bool is_reaction)
      {
         BingoLog.logMessage("Checking chemical table {0}", table.Value);
         using (BingoSession session = new BingoSession())
         {
            using (SqlConnection conn = new SqlConnection("context connection=true"))
            {
               conn.Open();
               prepareContext(conn, bingo_schema.Value, 0,
                  ContextFlags.NTHREADS | ContextFlags.FINGERPRINTS);

               BingoIndexID id = new BingoIndexID(conn, table.Value);
               BingoIndexData dummy_data;
               if (is_reaction)
                  dummy_data = new RingoIndexData(id, id_column.Value, data_column.Value, bingo_schema.Value);
               else
                  dummy_data = new MangoIndexData(id, id_column.Value, data_column.Value, bingo_schema.Value);

               ArrayList errors = new ArrayList();

               bingoOperationDelegate check_insert_op =
                  getInsertRecordsDelegate(table.Value, false, false, false, errors);
               if (BingoCore.lib.bingoIndexBegin() != 1)
                  throw new Exception(BingoCore.lib.bingoGetError());
               BingoCore.lib.bingoIndexSetSkipFP(true);
               check_insert_op(conn, conn, dummy_data);
               BingoCore.lib.bingoIndexEnd();

               return errors;
            }
         }
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
        SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunction]
      public static void FlushOperations (SqlString table_name, SqlString bingo_schema)
      {
         using (SqlConnection conn = new SqlConnection("context connection=true"))
         {
            conn.Open();
            using (SqlConnection ext_conn = new SqlConnection("server=" + getServername(conn) +
                  ";integrated security=true;database=" + conn.Database))
            {
               ext_conn.Open();

               using (BingoSession session = new BingoSession())
               {
                  BingoIndexData index_data = BingoIndexData.GetIndexData(conn,
                     bingo_schema.Value, table_name.Value, getSPID(conn));
                  if (index_data.locked)
                  {
                     string msg = String.Format("Chemical index for the table '{0}' is locked.", table_name);
                     BingoLog.logMessage(msg);
                     throw new Exception(msg);
                  }

                  index_data.flush(ext_conn);
               }
            }
         }
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
        SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunction(access_level=AccessLevelKind.None)]
      public static void _DropAllIndices (SqlString bingo_schema)
      {
         using (SqlConnection conn = new SqlConnection("context connection=true"))
         {
            conn.Open();
            using (BingoSession session = new BingoSession())
            {
               BingoIndexData.DropAllIndices(conn, bingo_schema.Value, false);
            }
         }
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
        SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunction(access_level = AccessLevelKind.None)]
      public static void _FlushInAllSessions (SqlString bingo_schema)
      {
         BingoIndexData.FlushInAllSessions(-1, false);
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
        SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunction]
      public static void DropInvalidIndices (SqlString bingo_schema)
      {
         using (SqlConnection conn = new SqlConnection("context connection=true"))
         {
            conn.Open();
            using (BingoSession session = new BingoSession())
            {
               BingoIndexData.DropAllIndices(conn, bingo_schema.Value, true);
            }
         }
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
        SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "molecule")]
      public static int GetAtomCount (SqlBinary molecule, SqlString bingo_schema)
      {
         using (BingoSession session = new BingoSession())
         {
            return BingoCore.lib.mangoGetAtomCount(molecule.Value, molecule.Value.Length);
         }
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
        SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunctionForReader(str_bin = "molecule")]
      public static int GetBondCount (SqlBinary molecule, SqlString bingo_schema)
      {
         using (BingoSession session = new BingoSession())
         {
            return BingoCore.lib.mangoGetBondCount(molecule.Value, molecule.Value.Length);
         }
      }

      /*
       * Profiling functions
       */
      [SqlFunction(DataAccess = DataAccessKind.Read,
        SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunction]
      public static SqlString GetStatistics (SqlString bingo_schema)
      {
         using (BingoSession session = new BingoSession())
         {
            return BingoCore.bingoProfilingGetStatistics(true);
         }
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
        SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunction]
      public static void ResetStatistics (SqlString bingo_schema)
      {
         using (BingoSession session = new BingoSession())
         {
            BingoCore.lib.bingoProfilingReset(true);
         }
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
        SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunction]
      public static double ProfilingGetTime (SqlString counter_name,
         SqlBoolean whole_session, SqlString bingo_schema)
      {
         using (BingoSession session = new BingoSession())
         {
            return BingoCore.lib.bingoProfilingGetTime(counter_name.Value, 
               whole_session.Value);
         }
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
        SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunction]
      public static long ProfilingGetValue (SqlString counter_name,
         SqlBoolean whole_session, SqlString bingo_schema)
      {
         using (BingoSession session = new BingoSession())
         {
            return BingoCore.lib.bingoProfilingGetValue(counter_name.Value,
               whole_session.Value);
         }
      }

      [SqlFunction(DataAccess = DataAccessKind.Read,
        SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunction]
      public static long ProfilingGetCount (SqlString counter_name,
         SqlBoolean whole_session, SqlString bingo_schema)
      {
         using (BingoSession session = new BingoSession())
         {
            return BingoCore.lib.bingoProfilingGetCount(counter_name.Value,
               whole_session.Value);
         }
      }

      /*
       * Test functions 
       */

      [SqlFunction(DataAccess = DataAccessKind.Read,
        SystemDataAccess = SystemDataAccessKind.Read)]
      [BingoSqlFunction]
      public static void _CheckMemoryAllocate(SqlInt32 dotnet_size_mb, SqlInt32 block_size_mb, 
         SqlInt32 core_size_mb, SqlString bingo_schema)
      {
         using (BingoSession session = new BingoSession())
         {
            long block_size = block_size_mb.Value;
            BingoLog.logMessage("Allocating {0} Mb in .NET by {1}Mb blocks...", 
               dotnet_size_mb.Value, block_size);

            int sum = 0;

            List<object> mem_blocks = new List<object>();
            long mem_left = dotnet_size_mb.Value;

            int index = 0;
            while (mem_left != 0)
            {
               long cur_alloc;
               if (mem_left > block_size)
                  cur_alloc = block_size;
               else
                  cur_alloc = mem_left;

               byte[] data = new byte[cur_alloc * 1024 * 1024];
               foreach (byte b in data)
                  sum += b;
               mem_blocks.Add(data);
               index++;

               mem_left -= cur_alloc;
               BingoLog.logMessage("   block {0} allocated, {1}Mb is left to allocate...", 
                  index, mem_left);
            }

            BingoLog.logMessage("Check sum is {0}", sum);
            BingoLog.logMessage("Allocating {0} Mb in bingo-core...", core_size_mb.Value);

            int ret = BingoCore.lib.bingoCheckMemoryAllocate(core_size_mb.Value * 1024 * 1024);
            if (ret != 1)
            {
               BingoLog.logMessage("  Failed: {0}", BingoCore.lib.bingoGetError());
               throw new Exception("Failed. See Bingo log for details");
            }
            BingoLog.logMessage("  OK");

            BingoLog.logMessage("  Waiting 2 sec...");
            Thread.Sleep(2000);

            BingoCore.lib.bingoCheckMemoryFree();

            BingoLog.logMessage("  Done.");
            SqlContext.Pipe.Send("Done.");
         }
      }

   }
}
