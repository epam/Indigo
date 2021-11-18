using System;
using System.Runtime.InteropServices; 

namespace indigo
{
   public class MangoIndex
   {
      public const int COUNTED_ELEMENTS_COUNT = 6;

      public double mass;
      public byte[] cmf;
      public byte[] xyz;

      public string gross, counted_elements_str;

      public byte[] fingerprint;
      public string fingerprint_sim_str;

      public int sim_fp_bits_count;

      public MoleculeHash hash;

      public bool readPrepared (out int id)
      {
         IntPtr cmf_ptr, xyz_ptr, fingerprint_ptr;
         int cmf_buf_len, xyz_buf_len, fingerprint_buf_len;
         IntPtr gross_str, counter_elements_str, fingerprint_sim_str_ptr;

         int ret = BingoCore.lib.mangoIndexReadPreparedMolecule(out id,
            out cmf_ptr, out cmf_buf_len, out xyz_ptr, out xyz_buf_len,
            out gross_str, out counter_elements_str,
            out fingerprint_ptr, out fingerprint_buf_len,
            out fingerprint_sim_str_ptr, out mass, out sim_fp_bits_count);

         if (ret == -2)
            throw new Exception(BingoCore.lib.bingoGetError());
         if (ret == -1)
            return false;

         cmf = new byte[cmf_buf_len];
         xyz = new byte[xyz_buf_len];
         fingerprint = new byte[fingerprint_buf_len];

         Marshal.Copy(cmf_ptr, cmf, 0, cmf_buf_len);
         if (xyz_ptr != IntPtr.Zero)
            Marshal.Copy(xyz_ptr, xyz, 0, xyz_buf_len);

         Marshal.Copy(fingerprint_ptr, fingerprint, 0, fingerprint_buf_len);

         gross = Marshal.PtrToStringAnsi(gross_str);
         counted_elements_str = Marshal.PtrToStringAnsi(counter_elements_str);
         fingerprint_sim_str = Marshal.PtrToStringAnsi(fingerprint_sim_str_ptr);

         // Get each hash element
         hash = new MoleculeHash();
         hash.elements.Clear();
         int hash_count, tmp;
         BingoCore.lib.mangoGetHash(true, -1, out hash_count, out tmp);
         for (int i = 0; i < hash_count; i++)
         {
            MoleculeHashElement elem = new MoleculeHashElement();

            BingoCore.lib.mangoGetHash(true, i, out elem.count, out elem.hash);
            hash.elements.Add(elem);
         }

         return true;
      }
   }
}
