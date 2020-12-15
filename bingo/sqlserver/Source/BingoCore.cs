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
      [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
      public delegate int GetNextRecordHandler (IntPtr context);

      [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
      public delegate void ProcessResultHandler (IntPtr context);

      [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
      public delegate void ProcessErrorHandler (int id, IntPtr context);

      private static BingoCoreLib _lib = null;
      public static BingoCoreLib lib
      {
         get
         {
            if (_lib == null)
            {
               BingoDllLoader.Instance.loadLibrary(null, "bingo-core-c.dll", "indigo.resource", false);
               _lib = BingoDllLoader.Instance.getInterface<BingoCoreLib>("bingo-core-c.dll");
            }
            return _lib;
         }
      }

      public static string bingoGetNameCore (byte[] buffer)
      {
         sbyte* res = lib.bingoGetNameCore(buffer, buffer.Length);

         if ((IntPtr)res == IntPtr.Zero)
            throw new Exception(lib.bingoGetError());

         return new String(res);
      }
      public static string mangoGrossGetConditions ()
      {
         sbyte* res = lib.mangoGrossGetConditions();
         if ((IntPtr)res == IntPtr.Zero)
            throw new Exception(lib.bingoGetError());
         return new String(res);
      }

      public static string mangoGross (byte[] target_buf)
      {
         sbyte* res = lib.mangoGross(target_buf, target_buf.Length);

         if ((IntPtr)res == IntPtr.Zero)
            return null;

         return new String(res);
      }

      public static string checkMolecule (byte[] molecule)
      {
         sbyte* res = lib.mangoCheckMolecule(molecule, molecule.Length);

         if ((IntPtr)res == IntPtr.Zero)
            return null;

         return new String(res);
      }

      public static string checkReaction (byte[] reaction)
      {
         sbyte* res = lib.ringoCheckReaction(reaction, reaction.Length);

         if ((IntPtr)res == IntPtr.Zero)
            return null;

         return new String(res);
      }

      public static string ringoAAM (byte[] reaction, string options)
      {
         sbyte* res = lib.ringoAAM(reaction, reaction.Length, options);

         if ((IntPtr)res == IntPtr.Zero)
            throw new Exception(lib.bingoGetError());

         return new String(res);
      }

      public static string ringoGetHightlightedReaction ()
      {
         sbyte* res = lib.ringoGetHightlightedReaction();

         if ((IntPtr)res == IntPtr.Zero)
            throw new Exception(lib.bingoGetError());

         return new String(res);
      }

      public static string mangoGetHightlightedMolecule ()
      {
         sbyte* res = lib.mangoGetHightlightedMolecule();

         if ((IntPtr)res == IntPtr.Zero)
            throw new Exception(lib.bingoGetError());

         return new String(res);
      }

      public static string mangoTauGetQueryGross () 
      { 
         return new String(lib.mangoTauGetQueryGross()); 
      }

      public static string mangoGetCountedElementName (int index)
      { 
         return new String(lib.mangoGetCountedElementName(index)); 
      }
      public static string bingoProfilingGetStatistics (bool for_session)
      { 
         return new String(lib.bingoProfilingGetStatistics(for_session));
      }

      public static string ringoRxnfile (byte[] reaction)
      {
         sbyte* res = lib.ringoRxnfile(reaction, reaction.Length);

         if ((IntPtr)res == IntPtr.Zero)
            return null;

         return new String(res);
      }

      public static string ringoRCML (byte[] reaction)
      {
         sbyte* res = lib.ringoRCML(reaction, reaction.Length);

         if ((IntPtr)res == IntPtr.Zero)
            return null;

         return new String(res);
      }

      public static string mangoMolfile (byte[] molecule)
      {
         sbyte* res = lib.mangoMolfile(molecule, molecule.Length);

         if ((IntPtr)res == IntPtr.Zero)
            return null;

         return new String(res);
      }

      public static string mangoCML (byte[] molecule)
      {
         sbyte* res = lib.mangoCML(molecule, molecule.Length);

         if ((IntPtr)res == IntPtr.Zero)
            return null;

         return new String(res);
      }

      public static byte[] mangoICM (byte[] molecule, bool save_xyz)
      {
         int out_len;
         IntPtr icm_ptr = lib.mangoICM(molecule, molecule.Length, save_xyz, out out_len);
         if ((IntPtr)icm_ptr == IntPtr.Zero)
            return null;
         byte[] icm = new byte[out_len];
         Marshal.Copy(icm_ptr, icm, 0, out_len);
         return icm;
      }

      public static byte[] ringoICR (byte[] reaction, bool save_xyz)
      {
         int out_len;
         IntPtr icr_ptr = lib.ringoICR(reaction, reaction.Length, save_xyz, out out_len);
         if ((IntPtr)icr_ptr == IntPtr.Zero)
            return null;
         byte[] icr = new byte[out_len];
         Marshal.Copy(icr_ptr, icr, 0, out_len);
         return icr;
      }

      public static string ringoRSMILES (byte[] buffer)
      {
         sbyte* res = lib.ringoRSMILES(buffer, buffer.Length);

         if ((IntPtr)res == IntPtr.Zero)
            return null;

         return new String(res);
      }

      public static string mangoSMILES (byte[] buffer, bool canonical)
      {
         sbyte* res = lib.mangoSMILES(buffer, buffer.Length, canonical ? 1 : 0);

         if ((IntPtr)res == IntPtr.Zero)
            return null;

         return new String(res);
      }

      public static byte[] mangoFingerprint(byte[] molecule, string options)
      {
         int fp_len;
         IntPtr fp_ptr = lib.mangoFingerprint(molecule, molecule.Length, options, out fp_len);
         if (fp_ptr == IntPtr.Zero)
            return null;

         byte[] fp = new byte[fp_len];
         Marshal.Copy(fp_ptr, fp, 0, fp_len);
         return fp;
      }

      public static byte[] ringoFingerprint(byte[] reaction, string options)
      {
         int fp_len;
         IntPtr fp_ptr = lib.ringoFingerprint(reaction, reaction.Length, options, out fp_len);
         if (fp_ptr == IntPtr.Zero)
            return null;

         byte[] fp = new byte[fp_len];
         Marshal.Copy(fp_ptr, fp, 0, fp_len);
         return fp;
      }

      public static string mangoInChI(byte[] molecule, string options)
      {
         int out_len;
         sbyte* res = lib.mangoInChI(molecule, molecule.Length, options, out out_len);

         if ((IntPtr)res == IntPtr.Zero)
            return null;

         return new String(res);
      }

      public static string mangoInChIKey(string incho)
      {
         sbyte* res = lib.mangoInChIKey(incho);

         if ((IntPtr)res == IntPtr.Zero)
            return null;

         return new String(res);
      }

      public static int mangoGetQueryFingerprint(out byte[] fp)
      {
         IntPtr fp_ptr;
         int fp_len;

         int ret = lib.mangoGetQueryFingerprint(out fp_ptr, out fp_len);
         fp = new byte[fp_len];
         Marshal.Copy(fp_ptr, fp, 0, fp_len);
         return ret;
      }

      public static int ringoGetQueryFingerprint (out byte[] fp)
      {
         IntPtr fp_ptr;
         int fp_len;

         int ret = lib.ringoGetQueryFingerprint(out fp_ptr, out fp_len);
         fp = new byte[fp_len];
         Marshal.Copy(fp_ptr, fp, 0, fp_len);
         return ret;
      }

      public static string bingoRDFImportGetNext ()
      { 
         return new String(lib.bingoRDFImportGetNext());
      }

      public static string bingoRDFImportGetParameter (string param_name)
      {
         return new String(lib.bingoRDFImportGetProperty(param_name));
      }

      public static string bingoSDFImportGetNext ()
      { 
         return new String(lib.bingoSDFImportGetNext());
      }

      public static string bingoSDFImportGetParameter (string param_name)
      {
         return new String(lib.bingoSDFImportGetProperty(param_name));
      }

      public static void tautomerRulesReady ()
      {
         if (lib.bingoTautomerRulesReady() == 0)
            throw new Exception(lib.bingoGetError());
      }

      public static void addTautomerRule (int n, string beg, string end)
      {
         if (lib.bingoAddTautomerRule(n, beg, end) == 0)
            throw new Exception(lib.bingoGetError());
      }

      public static void clearTautomerRules ()
      {
         if (lib.bingoClearTautomerRules() == 0)
            throw new Exception(lib.bingoGetError());
      }

      public static byte[] getConfigBin (string name)
      {
         IntPtr value_ptr;
         int value_len;

         if (lib.bingoGetConfigBin(name, out value_ptr, out value_len) == 0)
            throw new Exception(lib.bingoGetError());
         byte[] data = new byte[value_len];
         Marshal.Copy(value_ptr, data, 0, value_len);
         return data;
      }

      public static void setConfigBin (string name, byte[] value)
      {
         if (lib.bingoSetConfigBin(name, value, value.Length) == 0)
            throw new Exception(lib.bingoGetError());
      }

      public static int getConfigInt (string name)
      {
         int value;
         if (lib.bingoGetConfigInt(name, out value) == 0)
            throw new Exception(lib.bingoGetError());
         return value;
      }

      public static void setConfigInt (string name, int value)
      {
         if (lib.bingoSetConfigInt(name, value) == 0)
            throw new Exception(lib.bingoGetError());
      }

      public static void setContext (int id)
      {
         if (lib.bingoSetContext(id) == 0)
            throw new Exception(lib.bingoGetError());
      }
   }
}
