using System.IO;
using System;
using System.Collections.Generic;
using System.Collections;
using System.Text;
using System.Data;
using System.Data.SqlClient;
using System.Data.SqlTypes;
using System.Runtime.Serialization.Formatters.Binary;

namespace indigo
{
   public class BingoStorage
   {
      object _sync_object = new Object();
      BingoIndexData _index_data;
      const int MAX_BLOCK_SIZE = 8 * 1000 * 1000;

      class _Block
      {
         public int block_index; 
         public byte[] data;

         public int first_index, end_index; 
         public int[] offsets;
         public short[] lengths;

         public List<byte> pending_data;
         public List<int> pending_offsets;
         public List<short> pending_lengths;

         public bool dirty = false;
      }
      List<_Block> _blocks = new List<_Block>();
      int total_items = -1;

      public BingoStorage (BingoIndexData index_data)
      {
         _index_data = index_data;
      }

      public IEnumerable<int> enumerateStorageIds()
      {
         for (int i = 0; i < total_items; i++)
            yield return i;
      }

      public void createTables (SqlConnection conn)
      {
         BingoSqlUtils.ExecNonQuery(conn,
            @"CREATE TABLE {0} (
                [id] SMALLINT PRIMARY KEY,
                [first_index] INT,
                [count] INT,
                [offsets] VARBINARY(MAX),
                [lengths] VARBINARY(MAX),
                [data] VARBINARY(MAX))",
            _index_data.storageTable);
      }

      public void dropTables (SqlConnection conn)
      {
         BingoSqlUtils.ExecNonQueryNoThrow(conn, "DROP TABLE {0}", _index_data.storageTable);
      }

      private bool _blockLoaded (short block_index)
      {
         if (_blocks.Count > block_index)
         {
            _Block b = _blocks[block_index];
            if (b == null)
               return false;
            return true;
         }
         return false;
      }

      private void _validateBlock (short block_index, SqlConnection conn)
      {
         if (_blockLoaded(block_index))
            return;

         lock (_sync_object)
         {
            // Double-checked locking
            if (_blockLoaded(block_index))
               return;

            BingoTimer timer = new BingoTimer("storage.validate_block");

            BingoLog.logMessage("Loading storage block {0} for table {1}...",
               block_index, _index_data.id.FullTableName());

            string text = "SELECT [data], [first_index], [offsets], [lengths], [count]  from " + 
               _index_data.storageTable + " where id = " + block_index;
            using (SqlCommand command = new SqlCommand(text, conn))
            {
               command.CommandTimeout = 3600;
               using (SqlDataReader reader = command.ExecuteReader())
               {
                  if (!reader.Read())
                     throw new Exception("Block cannot be found");

                  while (_blocks.Count <= block_index)
                     _blocks.Add(new _Block());

                  if (_blocks[block_index] == null)
                     _blocks[block_index] = new _Block();

                  _Block block = _blocks[block_index];
                  block.block_index = block_index;
                  block.data = (byte[])reader["data"];
                  block.first_index = (int)reader["first_index"];

                  int count = (int)reader["count"];

                  block.offsets = new int[count];
                  MemoryStream mem_stream = new MemoryStream((byte[])reader["offsets"]);
                  BinaryReader bin_reader = new BinaryReader(mem_stream);
                  for (int i = 0; i < count; i++)
                     block.offsets[i] = bin_reader.ReadInt32();

                  block.lengths = new short[count];
                  mem_stream = new MemoryStream((byte[])reader["lengths"]);
                  bin_reader = new BinaryReader(mem_stream);
                  for (int i = 0; i < count; i++)
                     block.lengths[i] = bin_reader.ReadInt16();

                  block.end_index = block.first_index + count;
               }
            }
            BingoLog.logMessage("   Done.");

            timer.end();
         }
      }

      private void _validateLastBlock (SqlConnection conn)
      {
         _validateBlock((short)(_blocks.Count - 1), conn);
      }

      public void validate (SqlConnection conn)
      {
         if (total_items >= 0)
            return; // Index already loaded

         lock (_sync_object)
         {
            if (total_items >= 0)
               return; // Index already loaded

            int? ret = BingoSqlUtils.ExecIntQuery(conn, "select max(id) from {0}", 
               _index_data.storageTable);
            if (ret.HasValue)
            {
               while (_blocks.Count <= ret.Value)
                  _blocks.Add(null);
            }

            int? sum = BingoSqlUtils.ExecIntQuery(conn, "select sum(count) from {0}",
               _index_data.storageTable);
            if (!sum.HasValue)
               total_items = 0;
            else
               total_items = sum.Value;
         }
      }

      public bool needFlush()
      {
         lock (_sync_object)
         {
            foreach (_Block b in _blocks)
               if (b != null && b.dirty)
                  return true;
            return false;
         }
      }

      public void flush (SqlConnection conn)
      {
         lock (_sync_object)
         {
            for (int i = 0; i < _blocks.Count; i++)
            {
               _Block b = _blocks[i];
               if (b != null)
               {
                  if (b.dirty)
                  {
                     _flushBlock(conn, b);

                     BingoLog.logMessage("Disposing memory for block {0}...", b.block_index);
                     _blocks[i] = null;
                     BingoLog.logMessage("   Done");
                  }
               }
            }
         }
      }

      private void _flushBlock (SqlConnection conn, _Block b)
      {
         BingoLog.logMessage("Flushing storage block {0}", b.block_index);

         BingoSqlUtils.ExecNonQueryNoThrow(conn, "DELETE FROM {0} WHERE id={1}",
            _index_data.storageTable, b.block_index);
         string text = String.Format(@"INSERT INTO {0} 
            ([id], [first_index], [count], [offsets], [lengths], [data]) 
            VALUES ({1}, {2}, {3}, @offsets, @lengths, @data)",
            _index_data.storageTable, b.block_index, b.first_index, b.end_index - b.first_index);
         using (SqlCommand command = new SqlCommand(text, conn))
         {
            command.CommandTimeout = 3600;

            byte[] data;
            if (b.pending_data != null)
               data = b.pending_data.ToArray();
            else
               data = b.data;
            SqlBinary binary_data = new SqlBinary(data);
            command.Parameters.AddWithValue("@data", binary_data);

            ICollection<int> offsets;
            if (b.pending_offsets != null)
               offsets = b.pending_offsets;
            else
               offsets = b.offsets;

            MemoryStream mem_stream = new MemoryStream(offsets.Count * 4);
            BinaryWriter writer = new BinaryWriter(mem_stream);
            foreach (int offset in offsets)
               writer.Write(offset);
            byte[] buffer = mem_stream.GetBuffer();
            command.Parameters.AddWithValue("@offsets", buffer);

            ICollection<short> lengths;
            if (b.pending_lengths != null)
               lengths = b.pending_lengths;
            else
               lengths = b.lengths;

            mem_stream = new MemoryStream(offsets.Count * 2);
            writer = new BinaryWriter(mem_stream);
            foreach (short length in lengths)
               writer.Write(length);
            buffer = mem_stream.GetBuffer();
            command.Parameters.AddWithValue("@lengths", buffer);

            command.ExecuteNonQuery();
         }
         _convertPendingBlockToNormal(b);
      }

      public int add (byte[] data, SqlConnection conn)
      {
         lock (_sync_object)
         {
            _Block b = _getPendingBlock(data.Length, conn);

            // Add one byte for remove mark
            b.pending_data.Add(1);

            b.pending_offsets.Add(b.pending_data.Count);
            if (data.Length > short.MaxValue)
               throw new Exception("Data length is to long. Unexpected.");

            b.pending_lengths.Add((short)data.Length);

            b.pending_data.AddRange(data);

            total_items++;
            b.end_index++;

            b.dirty = true;
            return total_items - 1;
         }
      }

      private _Block _getPendingBlock (int data_length, SqlConnection conn)
      {
         bool need_new_block = false;
         if (_blocks.Count == 0)
            need_new_block = true;
         else
         {
            _Block last = _blocks[_blocks.Count - 1];
            if (last == null)
               _validateBlock((short)(_blocks.Count - 1), conn);

            last = _blocks[_blocks.Count - 1];
            ICollection collection = null;
            if (last.data != null)
               collection = last.data;
            else
               collection = last.pending_data;
            if (collection.Count + data_length > MAX_BLOCK_SIZE)
               need_new_block = true;
         }

         _Block block = null;
         if (need_new_block)
         {
            // Flush all dirty blocks and dispose memory
            flush(conn);

            _blocks.Add(new _Block());
            block = _blocks[_blocks.Count - 1];

            block.block_index = _blocks.Count - 1;
            if (_blocks.Count == 1)
               block.first_index = 0;
            else
               block.first_index = total_items;

            block.end_index = block.first_index;

            block.dirty = true;
         }
         else
            block = _blocks[_blocks.Count - 1];

         _convertBlockToPendingBlock(block);
         return block;
      }

      private void _convertBlockToPendingBlock (_Block block)
      {
         if (block.pending_data != null)
            return;
         block.pending_data = new List<byte>();
         if (block.data != null)
            block.pending_data.AddRange(block.data);
         block.data = null;

         block.pending_offsets = new List<int>();
         if (block.offsets != null)
            block.pending_offsets.AddRange(block.offsets);
         block.offsets = null;

         block.pending_lengths = new List<short>();
         if (block.lengths != null)
            block.pending_lengths.AddRange(block.lengths);
         block.lengths = null;
      }

      private void _convertPendingBlockToNormal (_Block block)
      {
         if (block.data != null)
            return;
         block.data = block.pending_data.ToArray();
         block.pending_data = null;

         block.offsets = block.pending_offsets.ToArray();
         block.pending_offsets = null;

         block.lengths = block.pending_lengths.ToArray();
         block.lengths = null;
      }

      private _Block _getBlockByIndex (SqlConnection conn, int index, ref int cache_index)
      {
         if (cache_index >= _blocks.Count || cache_index < 0)
            cache_index = 0;

         _validateBlock((short)cache_index, conn);
         _Block b = _blocks[cache_index];

         if (index < b.first_index || index >= b.end_index)
         {
            // Find another block
            cache_index = -1;
            do
            {
               cache_index++;
               if (cache_index >= _blocks.Count)
                  throw new Exception(
                     String.Format("index {0} wasn't found in the storage", index));

               _validateBlock((short)cache_index, conn);
               b = _blocks[cache_index];
            } while (index < b.first_index || index >= b.end_index);
         }
         return b;
      }

      // If length if -1 then all rest data is returned
      private void _getDataWithOffset(int index, int offset, int length,
         out byte[] dest_data, out int dest_offset, out int dest_len, 
         SqlConnection conn, ref int cache_index)
      {
         _Block b = _getBlockByIndex(conn, index, ref cache_index);
         int sub_offset = index - b.first_index;

         if (length == -1)
         {
            if (b.lengths != null)
               length = b.lengths[sub_offset] - offset;
            else
               length = b.pending_lengths[sub_offset] - offset;
         }

         int data_offset;
         if (b.offsets != null)
            data_offset = b.offsets[sub_offset];
         else
            data_offset = b.pending_offsets[sub_offset];

         if (b.data != null)
         {
            dest_data = b.data;
            dest_len = length;
            dest_offset = data_offset + offset;
         }
         else
         {
            dest_data = new byte[length];
            b.pending_data.CopyTo(data_offset + offset, dest_data, 0, dest_data.Length);
            dest_len = length;
            dest_offset = 0;
         }
      }

      // If length if -1 then all rest data is returned
      public byte[] get(int index, int offset, int length, SqlConnection conn, ref int cache_index)
      {
         byte[] all_data;
         int data_offset, data_length;
         _getDataWithOffset(index, offset, length,
            out all_data, out data_offset, out data_length, conn, ref cache_index);

         if (data_offset == 0 && data_length == all_data.Length)
            // Copy isn't necessary
            return all_data;

         byte[] data = new byte[data_length];
         Array.Copy(all_data, data_offset, data, 0, data.Length);
         return data;
      }

      public int getInt(int index, int offset, SqlConnection conn, ref int cache_index)
      {
         byte[] all_data;
         int data_offset, data_length;
         _getDataWithOffset(index, offset, 4, out all_data,
            out data_offset, out data_length, conn, ref cache_index);

         return BitConverter.ToInt32(all_data, data_offset);
      }

      public short getShort(int index, int offset, SqlConnection conn, ref int cache_index)
      {
         byte[] all_data;
         int data_offset, data_length;
         _getDataWithOffset(index, offset, 2, out all_data,
            out data_offset, out data_length, conn, ref cache_index);

         return BitConverter.ToInt16(all_data, data_offset);
      }

      public byte getByte (int index, int offset, SqlConnection conn, ref int cache_index)
      {
         byte[] all_data;
         int data_offset, data_length;
         _getDataWithOffset(index, offset, 1, out all_data,
            out data_offset, out data_length, conn, ref cache_index);

         return all_data[data_offset];
      }

      public bool isDeleted (int storage_id, SqlConnection conn, ref int cache_index)
      {
         return getByte(storage_id, -1, conn, ref cache_index) == 0;
      }

      public void deleteRecord (int storage_id, SqlConnection conn)
      {
         lock (_sync_object)
         {
            int cache_index = 0;
            _Block b = _getBlockByIndex(conn, storage_id, ref cache_index);

            int data_offset;
            if (b.offsets != null)
               data_offset = b.offsets[storage_id - b.first_index];
            else
               data_offset = b.pending_offsets[storage_id - b.first_index];

            if (b.data != null)
               b.data[data_offset - 1] = 0;
            else if (b.pending_data != null)
               b.pending_data[data_offset - 1] = 0;
            else
               throw new Exception("Internal error: cannot delete record");

            b.dirty = true;
         }
      }
   }
}
