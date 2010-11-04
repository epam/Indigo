using System;
using System.Runtime.InteropServices;
using System.Data;
using Microsoft.SqlServer.Server;
using System.Data.SqlTypes;
using System.Data.SqlClient;
using System.Collections;
using System.IO;
using System.Text;
using Microsoft.Win32;

namespace indigo
{
   public unsafe class BingoCore
   {
      public static string bingoGetVersion() { return new String(_bingoGetVersion()); }
      [DllImport("bingo-core-c.dll", EntryPoint = "bingoGetVersion", CharSet = CharSet.Auto)]
      private static extern sbyte * _bingoGetVersion();

      public static string bingoGetError () { return new String(_bingoGetError()); }
      [DllImport("bingo-core-c.dll", EntryPoint = "bingoGetError", CharSet = CharSet.Auto)]
      private static extern sbyte * _bingoGetError ();

      public static string bingoGetWarning () { return new String(_bingoGetWarning()); }
      [DllImport("bingo-core-c.dll", EntryPoint = "bingoGetWarning", CharSet = CharSet.Auto)]
      private static extern sbyte * _bingoGetWarning ();

      public static void setContext(int id)
      {
         if (bingoSetContext(id) == 0)
            throw new Exception(bingoGetError());
      }

      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      private static extern int bingoSetContext (int id);

      public static void setConfigInt (string name, int value)
      {
         if (bingoSetConfigInt(name, value) == 0)
            throw new Exception(bingoGetError());
      }
      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      private static extern int bingoSetConfigInt (
         [MarshalAs(UnmanagedType.LPStr)] string name, int value);

      public static int getConfigInt (string name)
      {
         int value;
         if (bingoGetConfigInt(name, out value) == 0)
            throw new Exception(bingoGetError());
         return value;
      }
      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      private static extern int bingoGetConfigInt (
         [MarshalAs(UnmanagedType.LPStr)] string name, out int value);

      public static void setConfigBin (string name, byte[] value)
      {
         if (bingoSetConfigBin(name, value, value.Length) == 0)
            throw new Exception(bingoGetError());
      }
      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      private static extern int bingoSetConfigBin (
         [MarshalAs(UnmanagedType.LPStr)] string name, 
         [MarshalAs(UnmanagedType.LPArray)] byte[] value, int len);

      public static byte[] getConfigBin (string name)
      {
         IntPtr value_ptr;
         int value_len;

         if (bingoGetConfigBin(name, out value_ptr, out value_len) == 0)
            throw new Exception(bingoGetError());
         byte[] data = new byte[value_len];
         Marshal.Copy(value_ptr, data, 0, value_len);
         return data;
      }
      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      private static extern int bingoGetConfigBin (
         [MarshalAs(UnmanagedType.LPStr)] string name, out IntPtr value, out int len);

      public static void clearTautomerRules ()
      {
         if (bingoClearTautomerRules() == 0)
            throw new Exception(bingoGetError());
      }

      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      private static extern int bingoClearTautomerRules ();

      public static void addTautomerRule (int n, string beg, string end)
      {
         if (bingoAddTautomerRule(n, beg, end) == 0)
            throw new Exception(bingoGetError());
      }

      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      private static extern int bingoAddTautomerRule (int n,
                [MarshalAs(UnmanagedType.LPStr)] string beg,
                [MarshalAs(UnmanagedType.LPStr)] string end);

      public static void tautomerRulesReady ()
      {
         if (bingoTautomerRulesReady() == 0)
            throw new Exception(bingoGetError());
      }

      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      private static extern int bingoTautomerRulesReady();

      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      public static extern int mangoSetupMatch(
         [MarshalAs(UnmanagedType.LPStr)] string search_type,
         [MarshalAs(UnmanagedType.LPStr)] string query,
         [MarshalAs(UnmanagedType.LPStr)] string options);

      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      public static extern int ringoSetupMatch (
         [MarshalAs(UnmanagedType.LPStr)] string search_type,
         [MarshalAs(UnmanagedType.LPStr)] string query,
         [MarshalAs(UnmanagedType.LPStr)] string options);

      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      public static extern int mangoMatchTarget (
         [MarshalAs(UnmanagedType.LPStr)] string target,
         int target_buf_len);

      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      public static extern int mangoMatchTargetBinary (
         [MarshalAs(UnmanagedType.LPArray)] byte[] target_cmf, int target_cmf_len,
         [MarshalAs(UnmanagedType.LPArray)] byte[] target_xyz, int target_xyz_len);

      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      public static extern int ringoMatchTarget (
         [MarshalAs(UnmanagedType.LPStr)] string target,
         int target_buf_len);

      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      public static extern int ringoMatchTargetBinary (
         [MarshalAs(UnmanagedType.LPArray)] byte[] target_cmf, int target_cmf_len);

      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      public static extern int bingoSDFImportOpen( 
         [MarshalAs(UnmanagedType.LPStr)] string filename );

      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      public static extern int bingoSDFImportClose();

      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      public static extern int bingoSDFImportEOF();

      public static string bingoSDFImportGetNext()
      { return new String(_bingoSDFImportGetNext()); }
      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto, EntryPoint = "bingoSDFImportGetNext")]
      private static extern sbyte* _bingoSDFImportGetNext();

      public static string bingoSDFImportGetParameter(string param_name) 
      { return new String(_bingoSDFImportGetProperty(param_name)); }
      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto, EntryPoint = "bingoSDFImportGetProperty")]
      private static extern sbyte* _bingoSDFImportGetProperty(
         [MarshalAs(UnmanagedType.LPStr)] string param_name);

      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      public static extern int bingoRDFImportOpen (
         [MarshalAs(UnmanagedType.LPStr)] string filename);

      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      public static extern int bingoRDFImportClose ();

      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      public static extern int bingoRDFImportEOF ();

      public static string bingoRDFImportGetNext ()
      { return new String(_bingoRDFImportGetNext()); }
      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto, EntryPoint = "bingoRDFImportGetNext")]
      private static extern sbyte* _bingoRDFImportGetNext ();

      public static string bingoRDFImportGetParameter (string param_name)
      { return new String(_bingoRDFImportGetProperty(param_name)); }
      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto, EntryPoint = "bingoRDFImportGetProperty")]
      private static extern sbyte* _bingoRDFImportGetProperty (
         [MarshalAs(UnmanagedType.LPStr)] string param_name);

      [DllImport("bingo-core-c.dll")]
      public static extern int mangoIndexBegin ();

      [DllImport("bingo-core-c.dll")]
      public static extern int ringoIndexBegin ();

      [DllImport("bingo-core-c.dll")]
      public static extern int mangoIndexEnd ();

      [DllImport("bingo-core-c.dll")]
      public static extern int ringoIndexEnd ();

      [DllImport("bingo-core-c.dll")]
      public static extern int mangoIndexPrepareMolecule (
         [MarshalAs(UnmanagedType.LPStr)] string molfile, int molfile_len,
         out IntPtr cmf_buf, out int cmf_buf_len,
         out IntPtr xyz_buf, out int xyz_buf_len,
         out IntPtr gross_str,
         out IntPtr counter_elements_str,
         out IntPtr fingerprint_buf, out int fingerprint_buf_len,
         out IntPtr fingerprint_sim_str,
         out float mass, out int sim_fp_bits_count);

      [DllImport("bingo-core-c.dll")]
      public static extern int ringoIndexPrepareReaction (
         [MarshalAs(UnmanagedType.LPStr)] string reaction, int reaction_len,
         out IntPtr crf_buf, out int crf_buf_len,
         out IntPtr fingerprint_buf, out int fingerprint_buf_len);

      [DllImport("bingo-core-c.dll")]
      public static extern int mangoGetHash (
         [MarshalAs(UnmanagedType.I1)] bool for_index, 
         int index, out int count, out int hash);

      public static int mangoGetQueryFingerprint (out byte[] fp)
      {
         IntPtr fp_ptr;
         int fp_len;

         int ret = _mangoGetQueryFingerprint(out fp_ptr, out fp_len);
         fp = new byte[fp_len];
         Marshal.Copy(fp_ptr, fp, 0, fp_len);
         return ret;
      }
      [DllImport("bingo-core-c.dll", EntryPoint = "mangoGetQueryFingerprint")]
      private static extern int _mangoGetQueryFingerprint (out IntPtr query_fp, out int query_fp_len);

      public static int ringoGetQueryFingerprint (out byte[] fp)
      {
         IntPtr fp_ptr;
         int fp_len;

         int ret = _ringoGetQueryFingerprint(out fp_ptr, out fp_len);
         fp = new byte[fp_len];
         Marshal.Copy(fp_ptr, fp, 0, fp_len);
         return ret;
      }
      [DllImport("bingo-core-c.dll", EntryPoint = "ringoGetQueryFingerprint")]
      private static extern int _ringoGetQueryFingerprint (out IntPtr query_fp, out int query_fp_len);

      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      public static extern int mangoGetAtomCount (
         [MarshalAs(UnmanagedType.LPStr)] string target_buf, int target_buf_len);

      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      public static extern int mangoGetBondCount (
         [MarshalAs(UnmanagedType.LPStr)] string target_buf, int target_buf_len);

      public static string mangoSMILES (string buffer, bool canonical)
      {
         sbyte* res = _mangoSMILES(buffer, buffer.Length, canonical ? 1 : 0);

         if ((IntPtr)res == IntPtr.Zero)
            throw new Exception(bingoGetError());

         return new String(res); 
      }
      [DllImport("bingo-core-c.dll", EntryPoint = "mangoSMILES", CharSet = CharSet.Auto)]
      private static extern sbyte * _mangoSMILES(
         [MarshalAs(UnmanagedType.LPStr)] string target_buf, int target_buf_len, int canonical);

      public static string ringoRSMILES (string buffer)
      {
         sbyte* res = _ringoRSMILES(buffer, buffer.Length);

         if ((IntPtr)res == IntPtr.Zero)
            throw new Exception(bingoGetError());

         return new String(res);
      }
      [DllImport("bingo-core-c.dll", EntryPoint = "ringoRSMILES", CharSet = CharSet.Auto)]
      private static extern sbyte* _ringoRSMILES (
         [MarshalAs(UnmanagedType.LPStr)] string target_buf, int target_buf_len);

      public static string mangoMolfile (string molecule)
      {
         sbyte* res = _mangoMolfile(molecule);

         if ((IntPtr)res == IntPtr.Zero)
            throw new Exception(bingoGetError());

         return new String(res);
      }
      [DllImport("bingo-core-c.dll", EntryPoint = "mangoMolfile", CharSet = CharSet.Auto)]
      private static extern sbyte* _mangoMolfile (
         [MarshalAs(UnmanagedType.LPStr)] string molecule);

      public static string ringoRxnfile (string reaction)
      {
         sbyte* res = _ringoRxnfile(reaction);

         if ((IntPtr)res == IntPtr.Zero)
            throw new Exception(bingoGetError());

         return new String(res);
      }
      [DllImport("bingo-core-c.dll", EntryPoint = "ringoRxnfile", CharSet = CharSet.Auto)]
      private static extern sbyte* _ringoRxnfile (
         [MarshalAs(UnmanagedType.LPStr)] string reaction);

      [DllImport("bingo-core-c.dll")]
      public static extern void bingoProfilingReset (
         [MarshalAs(UnmanagedType.I1)] bool reset_whole_session);

      public static string bingoProfilingGetStatistics (bool for_session)
      { return new String(_bingoProfilingGetStatistics(for_session)); }
      [DllImport("bingo-core-c.dll", EntryPoint = "bingoProfilingGetStatistics", CharSet = CharSet.Auto)]
      private static extern sbyte* _bingoProfilingGetStatistics ([MarshalAs(UnmanagedType.I1)] bool for_session);

      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      public static extern float bingoProfilingGetTime (
         [MarshalAs(UnmanagedType.LPStr)] string counter_name,
         [MarshalAs(UnmanagedType.I1)] bool whole_session);

      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      public static extern long bingoProfilingGetValue (
         [MarshalAs(UnmanagedType.LPStr)] string counter_name,
         [MarshalAs(UnmanagedType.I1)] bool whole_session);

      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      public static extern long bingoProfilingGetCount (
         [MarshalAs(UnmanagedType.LPStr)] string counter_name,
         [MarshalAs(UnmanagedType.I1)] bool whole_session);

      public static string mangoGetCountedElementName (int index)
      { return new String(_mangoGetCountedElementName(index)); }
      [DllImport("bingo-core-c.dll", EntryPoint = "mangoGetCountedElementName", CharSet = CharSet.Auto)]
      private static extern sbyte* _mangoGetCountedElementName (int index);

      [DllImport("bingo-core-c.dll")]
      public static extern int mangoNeedCoords ();

      [DllImport("bingo-core-c.dll")]
      [return: MarshalAs(UnmanagedType.I1)]  
      public static extern bool mangoExactNeedComponentMatching ();

      public static string mangoTauGetQueryGross () { return new String(_mangoTauGetQueryGross()); }
      [DllImport("bingo-core-c.dll", EntryPoint = "mangoTauGetQueryGross", CharSet = CharSet.Auto)]
      public static extern sbyte* _mangoTauGetQueryGross ();

      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      public static extern int mangoSimilarityGetBitMinMaxBoundsArray (
         int count,
         [MarshalAs(UnmanagedType.LPArray)] int[] target_ones,
         out IntPtr min_bound,
         out IntPtr max_bound);

      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      public static extern int mangoSimilaritySetMinMaxBounds (
         float min_bound, float max_bound);

      [DllImport("bingo-core-c.dll")]
      public static extern int mangoSimilarityGetScore (out Single score);

      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      public static extern int mangoSetHightlightingMode (int enable);

      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      public static extern int mangoLoadTargetBinaryXyz (
         [MarshalAs(UnmanagedType.LPArray)] byte[] target_xyz, int target_xyz_len);

      public static string mangoGetHightlightedMolecule ()
      {
         sbyte* res = _mangoGetHightlightedMolecule();

         if ((IntPtr)res == IntPtr.Zero)
            throw new Exception(bingoGetError());

         return new String(res);
      }
      [DllImport("bingo-core-c.dll", EntryPoint = "mangoGetHightlightedMolecule", CharSet = CharSet.Auto)]
      private static extern sbyte * _mangoGetHightlightedMolecule ();

      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      public static extern int ringoSetHightlightingMode (int enable);

      public static string ringoGetHightlightedReaction ()
      {
         sbyte* res = _ringoGetHightlightedReaction();

         if ((IntPtr)res == IntPtr.Zero)
            throw new Exception(bingoGetError());

         return new String(res);
      }
      [DllImport("bingo-core-c.dll", EntryPoint = "ringoGetHightlightedReaction", CharSet = CharSet.Auto)]
      private static extern sbyte* _ringoGetHightlightedReaction ();

      public static string ringoAAM (string reaction, string options)
      {
         sbyte* res = _ringoAAM(reaction, options);

         if ((IntPtr)res == IntPtr.Zero)
            throw new Exception(bingoGetError());

         return new String(res);
      }
      [DllImport("bingo-core-c.dll", EntryPoint = "ringoAAM", CharSet = CharSet.Auto)]
      private static extern sbyte* _ringoAAM (
         [MarshalAs(UnmanagedType.LPStr)] string reaction,
         [MarshalAs(UnmanagedType.LPStr)] string options);

      public static string checkReaction (string reaction)
      {
         sbyte* res = ringoCheckReaction(reaction);

         if ((IntPtr)res == IntPtr.Zero)
            return null;

         return new String(res);
      }
      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      private static extern sbyte* ringoCheckReaction (
         [MarshalAs(UnmanagedType.LPStr)] string reaction);

      public static string checkMolecule (string molecule)
      {
         sbyte* res = mangoCheckMolecule(molecule);

         if ((IntPtr)res == IntPtr.Zero)
            return null;

         return new String(res);
      }
      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      private static extern sbyte* mangoCheckMolecule (
         [MarshalAs(UnmanagedType.LPStr)] string molecule);

      public static string mangoGross (string target_buf)
      {
         sbyte* res = _mangoGross(target_buf, target_buf.Length);

         if ((IntPtr)res == IntPtr.Zero)
            throw new Exception(bingoGetError());

         return new String(res);
      }
      [DllImport("bingo-core-c.dll", EntryPoint = "mangoGross", CharSet = CharSet.Auto)]
      private static extern sbyte* _mangoGross (
         [MarshalAs(UnmanagedType.LPStr)] string target_buf, int target_buf_len);

      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      public static extern float mangoMass (
         [MarshalAs(UnmanagedType.LPStr)] string target_buf, int target_buf_len,
         [MarshalAs(UnmanagedType.LPStr)] string type);

      public static string mangoGrossGetConditions ()
      {
         sbyte* res = _mangoGrossGetConditions();
         if ((IntPtr)res == IntPtr.Zero)
            throw new Exception(bingoGetError());
         return new String(res);
      }
      [DllImport("bingo-core-c.dll", EntryPoint = "mangoGrossGetConditions", CharSet = CharSet.Auto)]
      private static extern sbyte* _mangoGrossGetConditions ();

      /* Profiling */
      [DllImport("bingo-core-c.dll")]
      public static extern UInt64 bingoProfNanoClock ();

      [DllImport("bingo-core-c.dll")]
      public static extern void bingoProfIncTimer (
         [MarshalAs(UnmanagedType.LPStr)] string target_buf, UInt64 dt);

      [DllImport("bingo-core-c.dll")]
      public static extern void bingoProfIncCounter (
         [MarshalAs(UnmanagedType.LPStr)] string target_buf, int value);

      public static string bingoGetNameCore (string buffer)
      {
         sbyte* res = _bingoGetNameCore(buffer, buffer.Length);

         if ((IntPtr)res == IntPtr.Zero)
            throw new Exception(bingoGetError());

         return new String(res); 
      }
      [DllImport("bingo-core-c.dll", EntryPoint = "bingoGetNameCore", CharSet = CharSet.Auto)]
      private static extern sbyte * _bingoGetNameCore(
         [MarshalAs(UnmanagedType.LPStr)] string target_buf, int target_buf_len);


      /* Test functions */
      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      public static extern int bingoCheckMemoryAllocate (int mem);
      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      public static extern int bingoCheckMemoryFree ();
   }
}
