using System.IO;
using System;
using System.Collections.Generic;
using System.Text;
using System.Data;
using System.Data.SqlClient;
using System.Data.SqlTypes;

namespace indigo
{
   public class BingoFingerprints
   {
      private object _sync_object = new Object();
      private BingoIndexData _index_data;
      private int _fp_bytes, _chunk_bytes;

      private int _fp_sub_bits_used = 10, _sim_screening_pass_mark = 128, _fp_sim_bits_group = 10;

      class Block
      {
         public Block (bool pending, int fp_bytes)
         {
            this.pending = pending;
            counters = new int[fp_bytes * 8];
         }

         public int part = -1;
         public int minimum_index = int.MaxValue, maximum_index = int.MinValue;
         public List<int> indices = new List<int>();
         public int[] counters;
         public List<byte[]> bits = null; // If not null then bits are in memory
         public bool pending = false;

         public void validateMinMax ()
         {
            minimum_index = int.MaxValue;
            maximum_index = int.MinValue;
            foreach (int v in indices)
               validateMinMax(v);
         }
         public void validateMinMax (int value)
         {
            if (minimum_index > value)
               minimum_index = value;
            if (maximum_index < value)
               maximum_index = value;
         }
      };
      List<Block> _all_blocks = new List<Block>();

      public BingoFingerprints (BingoIndexData index_data)
      {
         _index_data = index_data;
         _fp_bytes = -1;
         _chunk_bytes = 8000;
      }

      public void createTables (SqlConnection conn)
      {
         BingoSqlUtils.ExecNonQuery(conn,
            @"CREATE TABLE {0} (
                [part] INTEGER not null,
                [used] INTEGER not null,
                [mapping] VARBINARY(MAX),
                [counters] VARBINARY(MAX))",
            _index_data.fingerprintsTable);

         BingoSqlUtils.ExecNonQuery(conn,
            @"CREATE TABLE {0} (
                [part] INTEGER not null,
                [bit] INTEGER not null,
                [bits_chunk] BINARY({1}))",
            _index_data.fingerprintBitsTable, _chunk_bytes);
      }

      public void createIndices (SqlConnection conn)
      {
         BingoSqlUtils.ExecNonQuery(conn, "ALTER TABLE {0} ADD PRIMARY KEY (part)",
            _index_data.fingerprintsTable);
         BingoSqlUtils.ExecNonQuery(conn, "ALTER TABLE {0} ADD PRIMARY KEY (part, bit)", 
            _index_data.fingerprintBitsTable);
         BingoSqlUtils.ExecNonQuery(conn, "CREATE INDEX part ON {0}(part)",
            _index_data.fingerprintBitsTable);
      }

      public void dropTables (SqlConnection conn)
      {
         BingoSqlUtils.ExecNonQueryNoThrow(conn, "DROP TABLE {0}", _index_data.fingerprintsTable);
         BingoSqlUtils.ExecNonQueryNoThrow(conn, "DROP TABLE {0}", _index_data.fingerprintBitsTable);
      }

      public void init (SqlConnection conn)
      {
         // Read blocks
         int? max_part = BingoSqlUtils.ExecIntQuery(conn,
            "SELECT MAX(part) from {0}", _index_data.fingerprintsTable);
         if (max_part != null && _all_blocks.Count != max_part.Value)
         {
            lock (_sync_object)
            {
               // Check again
               max_part = BingoSqlUtils.ExecIntQuery(conn,
                  "SELECT MAX(part) from {0}", _index_data.fingerprintsTable);
               if (_all_blocks.Count == max_part.Value)
                  return;

               if (_all_blocks.Count != 0)
                  throw new Exception("Internal error: blocks were read partially");

               using (SqlCommand command =
                  new SqlCommand("SELECT [part], [used], [counters], [mapping] from " +
                     _index_data.fingerprintsTable + " ORDER BY [part]", conn))
               {
                  command.CommandTimeout = 3600;
                  using (SqlDataReader reader = command.ExecuteReader())
                  {
                     while (reader.Read())
                     {
                        Block new_block = new Block(false, _fp_bytes);
                        new_block.part = Convert.ToInt32(reader[0]);
                        int used = Convert.ToInt32(reader[1]);

                        byte[] counters = (byte[])reader[2];
                        byte[] mapping = (byte[])reader[3];

                        // Copy mapping
                        int[] data = new int[used];
                        Buffer.BlockCopy(mapping, 0, data, 0, mapping.Length);
                        new_block.indices.AddRange(data);
                        new_block.validateMinMax();

                        // Copy counters
                        Buffer.BlockCopy(counters, 0, new_block.counters, 0, counters.Length);

                        _all_blocks.Add(new_block);
                     }
                  }
               }
            }
         }
      }

      private void _validateBlockIndices (SqlConnection conn, Block block)
      {
         if (block.indices != null)
            return;

         lock (_sync_object)
         {
            if (block.indices != null)
               return;

            BingoLog.logMessage("validating fingerprint {0} block indices", block.part);

            using (SqlCommand command =
               new SqlCommand("SELECT [mapping], [used] from " +
                  _index_data.fingerprintsTable + " WHERE [part] = " + block.part, conn))
            {
               command.CommandTimeout = 3600;
               using (SqlDataReader reader = command.ExecuteReader())
               {
                  if (!reader.Read())
                     throw new Exception("Cannot read fingerprint " + block.part + " block");

                  byte[] mapping = (byte[])reader[0];
                  int used = Convert.ToInt32(reader[1]);

                  // Copy mapping
                  int[] data = new int[used];
                  Buffer.BlockCopy(mapping, 0, data, 0, mapping.Length);
                  block.indices = new List<int>();
                  block.indices.AddRange(data);
                  block.validateMinMax();
               }
            }
         }
      }

      private void _allocatePendingBlock (SqlConnection conn)
      {
         Block block = null;

         if (_all_blocks.Count == 0)
         {
            block = new Block(false, _fp_bytes);
            _all_blocks.Add(block);
         }

         if (block == null)
         {
            Block last = _all_blocks[_all_blocks.Count - 1];
            _validateBlockIndices(conn, last);
            if (last.indices.Count == _chunk_bytes * 8)
            {
               flush(conn);

               block = new Block(false, _fp_bytes);
               _all_blocks.Add(block);
            }
            else
               block = last;
         }

         if (block.pending)
            return;

         _readBlockBits(ref block, conn);
         block.pending = true;
      }

      private void _readBlockBits (ref Block block, SqlConnection conn)
      {
         if (block.bits != null)
            return; // Bits have already been loaded

         BingoLog.logMessage("_readBlockBits: allocating block {1} buffer {0} size...", 8 * _fp_bytes * _chunk_bytes,
            _all_blocks.Count);

         block.bits = new List<byte[]>(8 * _fp_bytes);
         for (int i = 0; i < 8 * _fp_bytes; i++)
            block.bits.Add(new byte[_chunk_bytes]);

         BingoLog.logMessage("  Done.");

         string command_text =
            String.Format("SELECT bit, bits_chunk from {0} where part = {1}",
                          _index_data.fingerprintBitsTable, block.part);

         using (SqlCommand command = new SqlCommand(command_text, conn))
         {
            command.CommandTimeout = 3600;
            using (SqlDataReader reader = command.ExecuteReader())
            {
               while (reader.Read())
               {
                  int bit = (int)reader[0];
                  byte[] bit_chunk = (byte[])reader[1];

                  if (bit_chunk.Length != _chunk_bytes)
                     throw new Exception("Bits block length is incorrect");

                  bit_chunk.CopyTo(block.bits[bit], 0);
               }
            }
         }
      }

      public void addFingerprint (SqlConnection conn, byte[] fp, int id)
      {
         lock (_sync_object)
         {
            _allocatePendingBlock(conn);

            Block last = _all_blocks[_all_blocks.Count - 1];
            last = _all_blocks[_all_blocks.Count - 1];

            _validateBlockIndices(conn, last);

            int index = last.indices.Count;
            last.indices.Add(id);
            last.validateMinMax(id);

            for (int i = 0; i < _fp_bytes; i++)
            {
               byte cur_byte = fp[i];
               for (int j = 0; j < 8; j++)
               {
                  int bit = 8 * i + j;
                  //int offset_chunk = () * _chunk_bytes;
                  int offset_bytes = index / 8;
                  int offset_bit = index % 8;
                  if ((cur_byte & (1 << j)) == 0)
                  {
                     last.bits[bit][offset_bytes] &= (byte)(~(1 << offset_bit));
                  }
                  else
                  {
                     last.counters[bit]++;
                     last.bits[bit][offset_bytes] |= (byte)(1 << offset_bit);
                  }
               }
            }
         }
      }

      public bool needFlush()
      {
         lock (_sync_object)
         {
            foreach (Block b in _all_blocks)
            {
               if (!b.pending || b.indices == null || b.indices.Count == 0)
                  continue;
               return true;
            }
            return false;
         }
      }

      public void flush (SqlConnection conn)
      {
         lock (_sync_object)
         {
            foreach (Block b in _all_blocks)
            {
               if (!b.pending || b.indices == null || b.indices.Count == 0)
                  continue;
               _flushBlock(b, conn);
            }
         }
      }

      private void _flushBlock (Block block, SqlConnection conn)
      {
         BingoTimer timer = new BingoTimer("fingerprints.flush");

         if (block.part == -1)
         {
            // Add new block
            int? max_id = BingoSqlUtils.ExecIntQuery(conn,
               "SELECT MAX(part) from {0}", _index_data.fingerprintsTable);
            if (max_id == null)
               max_id = 0;

            block.part = max_id.Value + 1;

            BingoSqlUtils.ExecNonQuery(conn, "INSERT INTO {0} values ({1}, 0, null, null)",
               _index_data.fingerprintsTable, block.part);
         }

         BingoLog.logMessage("Flushing fingerprints block {0}...", block.part);

         // Update used column and counters column
         string update_command_text = String.Format(@"UPDATE {0} SET used = @used, 
                              counters = @counters, mapping = @mapping
                              where part = {1}", _index_data.fingerprintsTable, block.part);

         using (SqlCommand command = new SqlCommand(update_command_text, conn))
         {
            command.CommandTimeout = 3600;
            command.Parameters.AddWithValue("@used", block.indices.Count);

            byte[] countes_bytes = new byte[8 * _fp_bytes * sizeof(int)];
            Buffer.BlockCopy(block.counters, 0, countes_bytes, 0, countes_bytes.Length);

            SqlBinary countes = new SqlBinary(countes_bytes);
            command.Parameters.AddWithValue("@counters", countes);

            byte[] mapping_bytes = new byte[block.indices.Count * sizeof(int)];
            Buffer.BlockCopy(block.indices.ToArray(), 0, mapping_bytes, 0, mapping_bytes.Length);

            SqlBinary mapping = new SqlBinary(mapping_bytes);
            command.Parameters.AddWithValue("@mapping", mapping);

            command.ExecuteNonQuery();
         }

         // Update bit chunks
         BingoSqlUtils.ExecNonQuery(conn, "DELETE FROM {0} WHERE part = {1}",
            _index_data.fingerprintBitsTable, block.part);

         string update_bits_text = String.Format(@"INSERT INTO {0} VALUES ({1}, @bit, @bit_chunk)",
            _index_data.fingerprintBitsTable, block.part);

         using (SqlCommand command = new SqlCommand(update_bits_text, conn))
         {
            command.CommandTimeout = 3600;
            command.Parameters.Add("@bit", SqlDbType.Int);
            command.Parameters.Add("@bit_chunk", SqlDbType.Binary);

            byte[] chunk = new byte[_chunk_bytes];

            for (int i = 0; i < 8 * _fp_bytes; i++)
            {
               command.Parameters["@bit"].Value = i;

               Buffer.BlockCopy(block.bits[i], 0, chunk, 0, chunk.Length);

               SqlBinary sql_chunk = new SqlBinary(chunk);
               command.Parameters["@bit_chunk"].Value = sql_chunk;

               command.ExecuteNonQuery();
            }
         }

         block.pending = false;
         block.bits = null;
         block.indices = null;

         BingoLog.logMessage("  Done.");
         timer.end();
      }

      public bool ableToScreen (byte[] fp)
      {
         foreach (byte b in fp)
            if (b != 0)
               return true;
         return false;
      }

      public IEnumerable<int> screenSub (SqlConnection conn, byte[] fp, int? next_after_storate_id)
      {
         screenInBlockDelegate screenBlockSub =
            new screenInBlockDelegate(
               (List<int> fp_ones, Block block, byte[] chunk, byte[] chunk2, SqlConnection conn2) =>
                  _screenInBlockSub(fp_ones, block, chunk, chunk2, conn2, next_after_storate_id));
         return _screen(conn, fp, next_after_storate_id, screenBlockSub);
      }

      // Get bounds on the number of bits
      public delegate void getBoundsDelegate (IList<int> storage_id,
         ref int[] min_common_ones, ref int[] max_common_ones, SqlConnection conn);

      public IEnumerable<int> screenSim (SqlConnection conn, byte[] fp,
         int? next_after_storate_id, getBoundsDelegate boundsDelegate)
      {
         screenInBlockDelegate screenBlockSim =
            new screenInBlockDelegate(
               (List<int> fp_ones, Block block, byte[] chunk, byte[] chunk2, SqlConnection conn2) =>
                  _screenInBlockSim(fp_ones, block, chunk, chunk2, conn2, boundsDelegate, next_after_storate_id));

         return _screen(conn, fp, next_after_storate_id, screenBlockSim);
      }

      // Delegate for screening in block
      private delegate List<int> screenInBlockDelegate (List<int> fp_ones, Block block,
         byte[] chunk, byte[] chunk2, SqlConnection conn);

      private IEnumerable<int> _screen (SqlConnection conn, byte[] fp, int? next_after_storate_id,
         screenInBlockDelegate screenInBlockFunc)
      {
         // Find ones
         List<int> fp_ones = new List<int>();
         for (int i = 0; i < _fp_bytes; i++)
         {
            byte b = fp[i];
            for (int j = 0; j < 8; j++)
               if ((b & (1 << j)) != 0)
                  fp_ones.Add(8 * i + j);
         }

         byte[] working_chunk1 = new byte[_chunk_bytes];
         byte[] working_chunk2 = new byte[_chunk_bytes];

         // Screen      
         foreach (Block block in _all_blocks)
         {
            List<int> results = null;

            _validateBlockIndices(conn, block);
            results = screenInBlockFunc(fp_ones, block, working_chunk1, working_chunk2, conn);

            foreach (int index in results)
               yield return index;
         }
      }

      private void _getBitChunk (Block block, int bit_index, SqlConnection conn, ref byte[] chunk)
      {
         BingoTimer timer = new BingoTimer("fingerprints.read");

         if (block.bits != null)
            Buffer.BlockCopy(block.bits[bit_index], 0, chunk, 0, chunk.Length);
         else
         {
            chunk = (byte[])BingoSqlUtils.ExecObjQuery(conn, 
               "SELECT bits_chunk from {0} where part = {1} and bit = {2}",
               _index_data.fingerprintBitsTable, block.part, bit_index);
         }
         timer.end();
      }

      private List<int> _screenInBlockSub (List<int> fp_ones, Block block,
         byte[] chunk, byte[] chunk2, SqlConnection conn, int? next_after_storate_id)
      {
         List<int> results = new List<int>();
         if (next_after_storate_id.HasValue && block.maximum_index < next_after_storate_id.Value)
            return results;

         int min_storate_id_bound = -1;
         if (next_after_storate_id.HasValue)
            min_storate_id_bound = next_after_storate_id.Value;

         // Sort ones
         fp_ones.Sort(
            (i1, i2) => block.counters[i1].CompareTo(block.counters[i2]));
         
         if (fp_ones.Count == 0)
            throw new Exception("Internal error: ableToScreen wasn't checked");

         BingoTimer timer = new BingoTimer("fingerprints.screening_sub");

         BingoCore.lib.bingoProfIncCounter("fingerprints.bits_total", fp_ones.Count);

         List<int> fp_ones_used = new List<int>();
         for (int i = 0; i < _fp_sub_bits_used; i++)
         {
            if (i >= fp_ones.Count)
               break;
            fp_ones_used.Add(fp_ones[i]);
         }

         int iteration = 0;
         foreach (BitChunk bit_chunk in bitChunksReaderGrouped(conn, block,
            fp_ones_used, _fp_sub_bits_used))
         {
            if (iteration == 0)
            {
               bit_chunk.chunk.CopyTo(chunk, 0);
               iteration++;
               continue;
            }
            else
               bit_chunk.chunk.CopyTo(chunk2, 0);

            iteration++;

            bool has_nonzero = false;
            for (int i = 0; i < chunk.Length; i++)
            {
               chunk[i] &= chunk2[i];
               if (chunk[i] != 0)
                  has_nonzero = true;
            }
            if (!has_nonzero)
               break;
         }
         BingoCore.lib.bingoProfIncCounter("fingerprints.bits_used", iteration);

         int max_byte_index = (block.indices.Count + 7) / 8;
         for (int i = 0; i < max_byte_index; i++)
         {
            byte b = chunk[i];
            if (b == 0)
               continue;
            for (int j = 0; j < 8; j++)
               if ((b & (1 << j)) != 0)
               {
                  int id = block.indices[8 * i + j];
                  if (id <= min_storate_id_bound)
                     continue;
                  results.Add(id);
               }
         }

         timer.end();

         return results;
      }

      private List<int> _screenInBlockSim (List<int> fp_ones, Block block,
         byte[] chunk, byte[] chunk2, SqlConnection conn, getBoundsDelegate boundsDelegate,
         int? next_after_storate_id)
      {
         List<int> passed_screening = new List<int>();
         if (next_after_storate_id.HasValue && block.maximum_index < next_after_storate_id.Value)
            return passed_screening;

         int min_storate_id_bound = -1;
         if (next_after_storate_id.HasValue)
            min_storate_id_bound = next_after_storate_id.Value;

         BingoTimer timer = new BingoTimer("fingerprints.screening_sim");

         int[] max_common_ones = new int[block.indices.Count];
         int[] min_common_ones = new int[block.indices.Count];
         int[] one_counters = new int[block.indices.Count];

         // Calculate max and min bounds
         BingoTimer timer2 = new BingoTimer("fingerprints.screening_bounds");

         boundsDelegate(block.indices, ref min_common_ones, ref max_common_ones, conn);

         timer2.end();

         List<int> passed_screening_tmp = new List<int>();

         BingoCore.lib.bingoProfIncCounter("fingerprints.bits_total", fp_ones.Count);

         if (fp_ones.Count == 0)  
         {
            for (int i = 0; i < block.indices.Count; i++)
            {
               if (block.indices[i] <= min_storate_id_bound)
                  continue;
               if (min_common_ones[i] == 0)
                  passed_screening.Add(i);
            }
            timer.end();
            return passed_screening;
         }
         int iteration = 0;
         foreach (BitChunk bit_chunk in bitChunksReaderGrouped(conn, block, 
            fp_ones, _fp_sim_bits_group))
         {
            chunk = bit_chunk.chunk;

            BingoTimer timer3 = new BingoTimer("fingerprints.screening_one_counters");
            // Calculate ones count
            int max_byte_index = (block.indices.Count + 7) / 8;
            for (int i = 0; i < max_byte_index; i++)
            {
               byte b = chunk[i];
               if (b == 0)
                  continue;

               for (int j = 0; j < 8; j++)
                  if ((b & (1 << j)) != 0)
                     one_counters[8 * i + j]++;
            }
            timer3.end();

            BingoTimer timer4 = new BingoTimer("fingerprints.screening_process");
            if (iteration == 0)
            {
               for (int i = 0; i < block.indices.Count; i++)
               {
                  if (block.indices[i] <= min_storate_id_bound)
                     continue;
                  int min_possible_ones = one_counters[i];
                  int max_possible_ones = one_counters[i] + fp_ones.Count;

                  if (min_possible_ones <= max_common_ones[i] && max_possible_ones >= min_common_ones[i])
                     passed_screening.Add(i);
               }
            }
            else
            {
               passed_screening_tmp.Clear();

               foreach (int i in passed_screening)
               {
                  int min_possible_ones = one_counters[i];
                  int max_possible_ones = one_counters[i] + fp_ones.Count - iteration;

                  if (min_possible_ones <= max_common_ones[i] && max_possible_ones >= min_common_ones[i])
                     passed_screening_tmp.Add(i);
               }

               // Swap then
               List<int> tmp = passed_screening;
               passed_screening = passed_screening_tmp;
               passed_screening_tmp = tmp;
            }
            timer4.end();

            iteration++;

            if (passed_screening.Count < _sim_screening_pass_mark)
               break;
         }

         BingoCore.lib.bingoProfIncCounter("fingerprints.bits_used", iteration);

         for (int i = 0; i < passed_screening.Count; i++)
            passed_screening[i] = block.indices[passed_screening[i]];

         timer.end();

         return passed_screening;
      }

      public void syncContextParameters (bool is_reaction)
      {
         lock (_sync_object)
         {
            if (is_reaction)
               _fp_bytes = BingoCore.getConfigInt("reaction-fp-size-bytes");
            else
               _fp_bytes = BingoCore.getConfigInt("fp-size-bytes");

            _fp_sub_bits_used = BingoCore.getConfigInt("SUB_SCREENING_MAX_BITS");
            _sim_screening_pass_mark = BingoCore.getConfigInt("SIM_SCREENING_PASS_MARK");
         }
      }

      class BitChunk
      {
         public int bit_index;
         public byte[] chunk;
      }

      IEnumerable<BitChunk> bitChunksReaderGrouped (SqlConnection conn, Block block, 
         List<int> bits, int size)
      {
         int offset = 0;
         while (offset < bits.Count)
         {
            int cur_size = size;
            if (offset + cur_size >= bits.Count)
               cur_size = bits.Count - offset;

            StringBuilder indices = new StringBuilder();
            for (int i = offset; i < offset + cur_size; i++)
            {
               if (indices.Length != 0)
                  indices.Append(", ");
               indices.AppendFormat("{0}", bits[i]);
            }

            string command_text = String.Format("select bit, bits_chunk from {0} where part={1} and bit in ({2})",
               _index_data.fingerprintBitsTable, block.part, indices.ToString());

            using (SqlCommand command = new SqlCommand(command_text, conn))
            {
               command.CommandTimeout = 3600;
               BingoTimer timer = new BingoTimer("fingerprints.read_grouped_exec");
               using (SqlDataReader reader = command.ExecuteReader())
               {
                  timer.end();

                  while (true)
                  {
                     timer = new BingoTimer("fingerprints.read_grouped_read");
                     bool ret = reader.Read();
                     if (!ret)
                        break;

                     BitChunk bit_chunk = new BitChunk();
                     int bit = (int)reader[0];
                     bit_chunk.bit_index = bit;
                     bit_chunk.chunk = (byte[])reader[1];

                     timer.end();
                     yield return bit_chunk;

                  }
               }
            }

            offset += size;
         }
      }
   }
}
