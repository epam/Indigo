using System;
using System.IO;
using System.Collections.Generic;
using System.Collections;
using System.Text;
using System.Data;
using System.Data.SqlClient;
using System.Data.SqlTypes;
using System.Runtime.Serialization.Formatters.Binary;
using System.Runtime.InteropServices; 

namespace indigo
{
   public class MangoFastIndexFetch
   {
      MangoIndexData _index_data;

      enum SearchType { UNDEF, SUB, SIM } ;
      SearchType search_type = SearchType.UNDEF;
      bool highlighting = false;

      public MangoFastIndexFetch (MangoIndexData index_data)
      {
         _index_data = index_data;
      }

      public int? nextAfterStorageId { get; set; }

      public void prepareSub (string query, string options, bool highlighting, bool smarts)
      {
         int res = BingoCore.lib.mangoSetupMatch(smarts ? "SMARTS" : "SUB", query, options);
         if (res < 0)
            throw new Exception(BingoCore.lib.bingoGetError());
         search_type = SearchType.SUB;

         BingoCore.lib.mangoSetHightlightingMode(highlighting ? 1 : 0);
         this.highlighting = highlighting;
      }

      public void prepareSimilarity (string query, string options, double min, double max)
      {
         int res = BingoCore.lib.mangoSetupMatch("SIM", query, options);
         if (res < 0)
            throw new Exception(BingoCore.lib.bingoGetError());
         BingoCore.lib.mangoSimilaritySetMinMaxBounds(min, max);
         search_type = SearchType.SIM;
      }

      private void simGetMinMaxBounds (IList<int> storage_ids, ref int[] min_common_ones, 
         ref int[] max_common_ones, SqlConnection conn)
      {
         BingoTimer timer2 = new BingoTimer("fingerprints.screening_bounds_ones");

         int cache_index = 0;
         int[] targets_ones = new int[storage_ids.Count];
         for (int i = 0; i < storage_ids.Count; i++)
         {
            int storage_id = storage_ids[i];
            targets_ones[i] = _index_data.storage.getShort(storage_id, 4, conn, ref cache_index);
         }
         timer2.end();

         BingoTimer timer3 = new BingoTimer("fingerprints.screening_bounds_get");

         IntPtr min_common_ones_ptr, max_common_ones_ptr;
         BingoCore.lib.mangoSimilarityGetBitMinMaxBoundsArray(targets_ones.Length, targets_ones,
            out min_common_ones_ptr, out max_common_ones_ptr);

         if (min_common_ones.Length != storage_ids.Count ||
            max_common_ones.Length != storage_ids.Count)
            throw new Exception("Internal error in simGetMinMaxBounds: array lengths mismatch");

         Marshal.Copy(min_common_ones_ptr, min_common_ones, 0, targets_ones.Length);
         Marshal.Copy(max_common_ones_ptr, max_common_ones, 0, targets_ones.Length);

         timer3.end();
      }

      public IEnumerable<FetchedData> fetch (SqlConnection conn)
      {
         byte[] fp;
         BingoCore.mangoGetQueryFingerprint(out fp);

         // Search using fast index
         _index_data.fingerprints.init(conn);
         _index_data.storage.validate(conn);

         int need_coords = BingoCore.lib.mangoNeedCoords();
         byte[] xyz = new byte[0];

         IEnumerable<int> screened;
         if (search_type == SearchType.SUB)
         {
            if (!_index_data.fingerprints.ableToScreen(fp))
               screened = _index_data.storage.enumerateStorageIds(nextAfterStorageId);
            else
               screened = _index_data.fingerprints.screenSub(conn, fp, nextAfterStorageId);
         }
         else
         {
            screened = _index_data.fingerprints.screenSim(conn, fp, nextAfterStorageId,
               new BingoFingerprints.getBoundsDelegate(simGetMinMaxBounds));
         }

         int cache_index = 0;
         foreach (int storage_id in screened)
         {
            if (_index_data.storage.isDeleted(storage_id, conn, ref cache_index))
               continue;

            // TODO: add match with xyz test for binary molecules
            byte[] data_with_cmf = _index_data.storage.get(storage_id, 6, -1, conn, ref cache_index);

            if (need_coords != 0)
            {
               xyz = _index_data.getXyz(storage_id, conn);
            }
            int ret = BingoCore.lib.mangoMatchTargetBinary(data_with_cmf,
               data_with_cmf.Length, xyz, xyz.Length);

            if (ret < 0)
            {
               // Exception has happend
               // Extract id
               int id = _index_data.storage.getInt(storage_id, 0, conn, ref cache_index);

               string msg = "Undef";
               if (ret == -2)
                  msg = BingoCore.lib.bingoGetError();
               if (ret == -1)
                  msg = BingoCore.lib.bingoGetWarning();
               throw new Exception(String.Format("Id = {0}: {1}", id, msg));
            }

            if (ret == 1)
            {
               // Extract id
               int id = _index_data.storage.getInt(storage_id, 0, conn, ref cache_index);
               FetchedData mol = new FetchedData(id);
               if (highlighting)
               {
                  if (need_coords == 0)
                  {
                     // Load xyz
                     byte[] xyz_found = _index_data.getXyz(storage_id, conn);
                     BingoCore.lib.mangoLoadTargetBinaryXyz(xyz_found, xyz_found.Length);
                  }

                  mol.str = BingoCore.mangoGetHightlightedMolecule();
               }
               if (search_type == SearchType.SIM)
                  BingoCore.lib.mangoSimilarityGetScore(out mol.value);
               yield return mol;
            }
         }
      }
   }
}