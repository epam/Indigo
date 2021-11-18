using System;
using System.Runtime.InteropServices; 

namespace indigo
{
   public class RingoIndex
   {
      public double mass;
      public byte[] crf;

      public byte[] fingerprint;

      public int hash;

      public bool readPrepared (out int id)
      {
         IntPtr crf_ptr, fingerprint_ptr;
         int crf_buf_len, fingerprint_buf_len;

         int ret = BingoCore.lib.ringoIndexReadPreparedReaction(out id, 
            out crf_ptr, out crf_buf_len, out fingerprint_ptr, out fingerprint_buf_len);

         BingoCore.lib.ringoGetHash(true, out hash);

         if (ret == -2)
            throw new Exception(BingoCore.lib.bingoGetError());
         if (ret == -1)
            return false;

         crf = new byte[crf_buf_len];
         fingerprint = new byte[fingerprint_buf_len];

         Marshal.Copy(crf_ptr, crf, 0, crf_buf_len);
         Marshal.Copy(fingerprint_ptr, fingerprint, 0, fingerprint_buf_len);

         return true;
      }
   }
}
