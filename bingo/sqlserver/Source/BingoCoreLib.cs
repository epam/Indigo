using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

namespace indigo
{
   public unsafe interface BingoCoreLib
   {
      String bingoGetVersion ();
      String bingoGetError ();
      String bingoGetWarning ();

      int bingoSetContext (int id);

      int bingoSetConfigInt (
         [MarshalAs(UnmanagedType.LPStr)] string name, int value);

      int bingoGetConfigInt (
         [MarshalAs(UnmanagedType.LPStr)] string name, out int value);

      int bingoSetConfigBin (
         [MarshalAs(UnmanagedType.LPStr)] string name,
         [MarshalAs(UnmanagedType.LPArray)] byte[] value, int len);

      int bingoGetConfigBin (
         [MarshalAs(UnmanagedType.LPStr)] string name, out IntPtr value, out int len);

      int bingoClearTautomerRules ();

      int bingoAddTautomerRule (int n,
                [MarshalAs(UnmanagedType.LPStr)] string beg,
                [MarshalAs(UnmanagedType.LPStr)] string end);

      int bingoTautomerRulesReady ();

      int mangoSetupMatch (
         [MarshalAs(UnmanagedType.LPStr)] string search_type,
         [MarshalAs(UnmanagedType.LPStr)] string query,
         [MarshalAs(UnmanagedType.LPStr)] string options);

      int ringoSetupMatch (
         [MarshalAs(UnmanagedType.LPStr)] string search_type,
         [MarshalAs(UnmanagedType.LPStr)] string query,
         [MarshalAs(UnmanagedType.LPStr)] string options);

      int mangoMatchTarget (
         [MarshalAs(UnmanagedType.LPStr)] string target,
         int target_buf_len);

      int mangoMatchTargetBinary (
         [MarshalAs(UnmanagedType.LPArray)] byte[] target_cmf, int target_cmf_len,
         [MarshalAs(UnmanagedType.LPArray)] byte[] target_xyz, int target_xyz_len);

      int ringoMatchTarget (
         [MarshalAs(UnmanagedType.LPStr)] string target,
         int target_buf_len);

      int ringoMatchTargetBinary (
         [MarshalAs(UnmanagedType.LPArray)] byte[] target_cmf, int target_cmf_len);

      int bingoSDFImportOpen (
         [MarshalAs(UnmanagedType.LPStr)] string filename);

      int bingoSDFImportClose ();

      int bingoSDFImportEOF ();

      sbyte* bingoSDFImportGetNext ();

      sbyte* bingoSDFImportGetProperty (
         [MarshalAs(UnmanagedType.LPStr)] string param_name);

      int bingoRDFImportOpen (
         [MarshalAs(UnmanagedType.LPStr)] string filename);

      int bingoRDFImportClose ();

      int bingoRDFImportEOF ();

      sbyte* bingoRDFImportGetNext ();

      sbyte* bingoRDFImportGetProperty (
         [MarshalAs(UnmanagedType.LPStr)] string param_name);

      int mangoIndexBegin ();

      int ringoIndexBegin ();

      int mangoIndexEnd ();

      int ringoIndexEnd ();

      int mangoIndexReadPreparedMolecule (
         out int id,
         out IntPtr cmf_buf, out int cmf_buf_len,
         out IntPtr xyz_buf, out int xyz_buf_len,
         out IntPtr gross_str,
         out IntPtr counter_elements_str,
         out IntPtr fingerprint_buf, out int fingerprint_buf_len,
         out IntPtr fingerprint_sim_str,
         out float mass, out int sim_fp_bits_count);

      int ringoIndexPrepareReaction (
         [MarshalAs(UnmanagedType.LPStr)] string reaction, int reaction_len,
         out IntPtr crf_buf, out int crf_buf_len,
         out IntPtr fingerprint_buf, out int fingerprint_buf_len);

      int mangoGetHash (
         [MarshalAs(UnmanagedType.I1)] bool for_index,
         int index, out int count, out int hash);

      int mangoGetQueryFingerprint (out IntPtr query_fp, out int query_fp_len);

      int ringoGetQueryFingerprint (out IntPtr query_fp, out int query_fp_len);

      int mangoGetAtomCount (
         [MarshalAs(UnmanagedType.LPStr)] string target_buf, int target_buf_len);

      int mangoGetBondCount (
         [MarshalAs(UnmanagedType.LPStr)] string target_buf, int target_buf_len);

      sbyte* mangoSMILES (
         [MarshalAs(UnmanagedType.LPStr)] string target_buf, int target_buf_len, int canonical);

      sbyte* ringoRSMILES (
         [MarshalAs(UnmanagedType.LPStr)] string target_buf, int target_buf_len);

      sbyte* mangoMolfile (
         [MarshalAs(UnmanagedType.LPStr)] string molecule);

      sbyte* ringoRxnfile (
         [MarshalAs(UnmanagedType.LPStr)] string reaction);

      sbyte* mangoGetCountedElementName (int index);

      int mangoNeedCoords ();

      [return: MarshalAs(UnmanagedType.I1)]
      bool mangoExactNeedComponentMatching ();

      sbyte* mangoTauGetQueryGross ();

      int mangoSimilarityGetBitMinMaxBoundsArray (
         int count,
         [MarshalAs(UnmanagedType.LPArray)] int[] target_ones,
         out IntPtr min_bound,
         out IntPtr max_bound);

      int mangoSimilaritySetMinMaxBounds (
         float min_bound, float max_bound);

      int mangoSimilarityGetScore (out Single score);

      int mangoSetHightlightingMode (int enable);

      int mangoLoadTargetBinaryXyz (
         [MarshalAs(UnmanagedType.LPArray)] byte[] target_xyz, int target_xyz_len);

      sbyte* mangoGetHightlightedMolecule ();

      int ringoSetHightlightingMode (int enable);

      sbyte* ringoGetHightlightedReaction ();

      sbyte* ringoAAM (
         [MarshalAs(UnmanagedType.LPStr)] string reaction,
         [MarshalAs(UnmanagedType.LPStr)] string options);

      sbyte* ringoCheckReaction (
         [MarshalAs(UnmanagedType.LPStr)] string reaction);

      sbyte* mangoCheckMolecule (
         [MarshalAs(UnmanagedType.LPStr)] string molecule);

      sbyte* mangoGross (
         [MarshalAs(UnmanagedType.LPStr)] string target_buf, int target_buf_len);

      float mangoMass (
         [MarshalAs(UnmanagedType.LPStr)] string target_buf, int target_buf_len,
         [MarshalAs(UnmanagedType.LPStr)] string type);

      sbyte* mangoGrossGetConditions ();

      /* Profiling */
      void bingoProfilingReset (
         [MarshalAs(UnmanagedType.I1)] bool reset_whole_session);

      sbyte* bingoProfilingGetStatistics ([MarshalAs(UnmanagedType.I1)] bool for_session);

      float bingoProfilingGetTime (
         [MarshalAs(UnmanagedType.LPStr)] string counter_name,
         [MarshalAs(UnmanagedType.I1)] bool whole_session);

      long bingoProfilingGetValue (
         [MarshalAs(UnmanagedType.LPStr)] string counter_name,
         [MarshalAs(UnmanagedType.I1)] bool whole_session);

      long bingoProfilingGetCount (
         [MarshalAs(UnmanagedType.LPStr)] string counter_name,
         [MarshalAs(UnmanagedType.I1)] bool whole_session);

      UInt64 bingoProfNanoClock ();

      void bingoProfIncTimer (
         [MarshalAs(UnmanagedType.LPStr)] string target_buf, UInt64 dt);

      void bingoProfIncCounter (
         [MarshalAs(UnmanagedType.LPStr)] string target_buf, int value);

      sbyte* bingoGetNameCore (
         [MarshalAs(UnmanagedType.LPStr)] string target_buf, int target_buf_len);

      /* Parallel indexing */

      int bingoSetIndexRecordData (int id, byte[] data, int data_size);

      int mangoIndexProcess (
         BingoCore.GetNextRecordHandler get_next_record,
         BingoCore.ProcessResultHandler process_result,
         BingoCore.ProcessErrorHandler process_error, IntPtr context);

      /* Test functions */
      int bingoCheckMemoryAllocate (int mem);
      int bingoCheckMemoryFree ();
   }
}
