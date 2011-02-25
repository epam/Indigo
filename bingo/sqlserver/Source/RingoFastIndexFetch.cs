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
   public class RingoFastIndexFetch
   {
      RingoIndexData _index_data;
      bool highlighting;

      public int? nextAfterStorageId { get; set; }

      public RingoFastIndexFetch (RingoIndexData index_data)
      {
         _index_data = index_data;
      }

      public void prepareSub (string query, string options, bool highlighting)
      {
         int res = BingoCore.ringoSetupMatch("RSUB", query, options);
         if (res < 0)
            throw new Exception(BingoCore.bingoGetError());
         BingoCore.ringoSetHightlightingMode(highlighting ? 1 : 0);
         this.highlighting = highlighting;
      }

      public IEnumerable<FetchedData> fetch (SqlConnection conn)
      {
         byte[] fp;
         BingoCore.ringoGetQueryFingerprint(out fp);

         // Search using fast index
         _index_data.fingerprints.init(conn);
         _index_data.storage.validate(conn);

         IEnumerable<int> screened;
         if (!_index_data.fingerprints.ableToScreen(fp))
            screened = _index_data.storage.enumerateStorageIds(nextAfterStorageId);
         else
            screened = _index_data.fingerprints.screenSub(conn, fp, nextAfterStorageId);

         int cache_index = 0;
         foreach (int storage_id in screened)
         {
            byte[] data_with_cmf = _index_data.storage.get(storage_id, 4, -1, conn, ref cache_index);

            int ret = BingoCore.ringoMatchTargetBinary(data_with_cmf,
               data_with_cmf.Length);

            if (ret == -2)
               throw new Exception(BingoCore.bingoGetError());
            if (ret == -1)
               throw new Exception(BingoCore.bingoGetWarning());

            if (ret == 1)
            {
               int id = _index_data.storage.getInt(storage_id, 0, conn, ref cache_index);
               FetchedData data = new FetchedData(id);
               if (highlighting)
                  data.subhi = BingoCore.ringoGetHightlightedReaction();
               yield return data;
            }
         }
      }
   }
}