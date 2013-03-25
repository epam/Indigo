using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.IO;
using System.Drawing;
using System.Diagnostics;
using System.Collections;

namespace com.ggasoftware.indigo
{
   [Flags]
   public enum ReactingCenter
   {
      NOT_CENTER = -1,
      UNMARKED = 0,
      CENTER = 1,
      UNCHANGED = 2,
      MADE_OR_BROKEN = 4,
      ORDER_CHANGED = 8
   }

   public unsafe class Indigo : IDisposable
   {
      public const int ABS = 1;
      public const int OR = 2;
      public const int AND = 3;
      public const int EITHER = 4;
      public const int UP = 5;
      public const int DOWN = 6;
      public const int CIS = 7;
      public const int TRANS = 8;
      public const int CHAIN = 9;
      public const int RING = 10;

      public const int ALLENE = 11;
  
      public const int SINGLET = 101;
      public const int DOUBLET = 102;
      public const int TRIPLET = 103;
      
      public const int RC_NOT_CENTER = -1;
      public const int RC_UNMARKED =  0;
      public const int RC_CENTER = 1;
      public const int RC_UNCHANGED = 2;
      public const int RC_MADE_OR_BROKEN = 4;
      public const int RC_ORDER_CHANGED = 8;

      public long getSID ()
      {
         return _sid;
      }

      public String version ()
      {
         return new String(_indigo_lib.indigoVersion());
      }

      public Indigo (string lib_path)
      {
         init(lib_path);
      }

      public Indigo ()
         : this(null)
      {
      }

      public int countReferences ()
      {
         setSessionID();
         return _indigo_lib.indigoCountReferences();
      }

      public void setOption (string name, string value)
      {
         setSessionID();
         _indigo_lib.indigoSetOption(name, value);
      }

      public void setOption (string name, int x, int y)
      {
         setSessionID();
         _indigo_lib.indigoSetOptionXY(name, x, y);
      }

      public void setOption (string name, bool value)
      {
         setSessionID();
         _indigo_lib.indigoSetOptionBool(name, value ? 1 : 0);
      }

      public void setOption (string name, float value)
      {
         setSessionID();
         _indigo_lib.indigoSetOptionFloat(name, value);
      }

      public void setOption (string name, int value)
      {
         setSessionID();
         _indigo_lib.indigoSetOptionInt(name, value);
      }

      public void setOption (string name, Color value)
      {
         setSessionID();
         _indigo_lib.indigoSetOptionColor(name, value.R / 255.0f, value.G / 255.0f, value.B / 255.0f);
      }

      public IndigoObject writeFile (String filename)
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoWriteFile(filename));
      }

      public IndigoObject writeBuffer ()
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoWriteBuffer());
      }

      public IndigoObject createMolecule ()
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoCreateMolecule());
      }

      public IndigoObject createQueryMolecule ()
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoCreateQueryMolecule());
      }

      public IndigoObject loadMolecule (String str)
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoLoadMoleculeFromString(str));
      }

      public IndigoObject loadMolecule (byte[] buf)
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoLoadMoleculeFromBuffer(buf, buf.Length));
      }

      public IndigoObject loadMoleculeFromFile (String path)
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoLoadMoleculeFromFile(path));
      }

      public IndigoObject loadQueryMolecule (String str)
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoLoadQueryMoleculeFromString(str));
      }

      public IndigoObject loadQueryMolecule (byte[] buf)
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoLoadQueryMoleculeFromBuffer(buf, buf.Length));
      }

      public IndigoObject loadQueryMoleculeFromFile (String path)
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoLoadQueryMoleculeFromFile(path));
      }

      public IndigoObject loadSmarts (String str)
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoLoadSmartsFromString(str));
      }

      public IndigoObject loadSmarts (byte[] buf)
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoLoadSmartsFromBuffer(buf, buf.Length));
      }

      public IndigoObject loadSmartsFromFile (String path)
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoLoadSmartsFromFile(path));
      }

      public IndigoObject loadReaction (String str)
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoLoadReactionFromString(str));
      }

      public IndigoObject loadReaction (byte[] buf)
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoLoadReactionFromBuffer(buf, buf.Length));
      }

      public IndigoObject loadReactionFromFile (String path)
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoLoadReactionFromFile(path));
      }

      public IndigoObject loadQueryReaction (String str)
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoLoadQueryReactionFromString(str));
      }

      public IndigoObject loadQueryReaction (byte[] buf)
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoLoadQueryReactionFromBuffer(buf, buf.Length));
      }

      public IndigoObject loadQueryReactionFromFile (String path)
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoLoadQueryReactionFromFile(path));
      }

      public IndigoObject loadReactionSmarts (String str)
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoLoadReactionSmartsFromString(str));
      }

      public IndigoObject loadReactionSmarts (byte[] buf)
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoLoadReactionSmartsFromBuffer(buf, buf.Length));
      }

      public IndigoObject loadReactionSmartsFromFile (String path)
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoLoadReactionSmartsFromFile(path));
      }

      public IndigoObject createReaction ()
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoCreateReaction());
      }

      public IndigoObject createQueryReaction ()
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoCreateQueryReaction());
      }

      public IndigoObject exactMatch (IndigoObject obj1, IndigoObject obj2, string flags)
      {
         setSessionID();

         if (flags == null)
            flags = "";

         int match = _indigo_lib.indigoExactMatch(obj1.self, obj2.self, flags);

         if (match == 0)
            return null;
         return new IndigoObject(this, new IndigoObject[] { obj1, obj2 }, match);
      }

      public IndigoObject exactMatch (IndigoObject obj1, IndigoObject obj2)
      {
         return exactMatch(obj1, obj2, "");
      }

      public void setTautomerRule (int id, string beg, string end)
      {
         setSessionID();
         _indigo_lib.indigoSetTautomerRule(id, beg, end);
      }

      public void removeTautomerRule (int id)
      {
         setSessionID();
         _indigo_lib.indigoRemoveTautomerRule(id);
      }

      public void clearTautomerRules ()
      {
         setSessionID();
         _indigo_lib.indigoClearTautomerRules();
      }

      public IndigoObject unserialize (byte[] buf)
      {
         return new IndigoObject(this, _indigo_lib.indigoUnserialize(buf, buf.Length));
      }

      public IndigoObject createArray ()
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoCreateArray());
      }

      public float similarity (IndigoObject obj1, IndigoObject obj2, string metrics)
      {
         setSessionID();
         if (metrics == null)
            metrics = "";
         return _indigo_lib.indigoSimilarity(obj1.self, obj2.self, metrics);
      }

      public int commonBits (IndigoObject obj1, IndigoObject obj2)
      {
         setSessionID();
         return _indigo_lib.indigoCommonBits(obj1.self, obj2.self);
      }

      public IndigoObject iterateSDFile (string filename)
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoIterateSDFile(filename));
      }

      public IndigoObject iterateRDFile (string filename)
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoIterateRDFile(filename));
      }

      public IndigoObject iterateSmilesFile (string filename)
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoIterateSmilesFile(filename));
      }

      public IndigoObject iterateCMLFile (string filename)
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoIterateCMLFile(filename));
      }

      public IndigoObject substructureMatcher (IndigoObject target, string mode)
      {
         setSessionID();
         return new IndigoObject(this, target, _indigo_lib.indigoSubstructureMatcher(target.self, mode));
      }

      public IndigoObject substructureMatcher (IndigoObject target)
      {
         return substructureMatcher(target, "");
      }

      public IndigoObject extractCommonScaffold (IndigoObject structures, string options)
      {
         setSessionID();
         int res = _indigo_lib.indigoExtractCommonScaffold(structures.self, options);
         if (res == 0)
            return null;
         return new IndigoObject(this, res);
      }

      public IndigoObject toIndigoArray (IEnumerable collection)
      {
         setSessionID();

         IndigoObject arr = createArray();
         foreach (IndigoObject obj in collection)
            arr.arrayAdd(obj);

         return arr;
      }

      public static int[] toIntArray (ICollection collection)
      {
         if (collection == null)
            return new int[0];

         int[] res = new int[collection.Count];
         int i = 0;

         foreach (object x in collection)
            res[i++] = Convert.ToInt32(x);

         return res;
      }

      public static float[] toFloatArray (ICollection collection)
      {
         if (collection == null)
            return new float[0];

         float[] res = new float[collection.Count];
         int i = 0;

         foreach (object x in collection)
            res[i++] = Convert.ToSingle(x);

         return res;
      }

      public IndigoObject extractCommonScaffold (IEnumerable structures, string options)
      {
         return extractCommonScaffold(toIndigoArray(structures), options);
      }

      public IndigoObject decomposeMolecules (IndigoObject scaffold, IndigoObject structures)
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoDecomposeMolecules(scaffold.self, structures.self));
      }

      public IndigoObject decomposeMolecules (IndigoObject scaffold, IEnumerable structures)
      {
         return decomposeMolecules(scaffold, toIndigoArray(structures));
      }
      
      public IndigoObject createDecomposer(IndigoObject scaffold)
      {
          setSessionID();
          return new IndigoObject(this, _indigo_lib.indigoCreateDecomposer(scaffold.self));
      }

      public IndigoObject reactionProductEnumerate (IndigoObject reaction, IndigoObject monomers)
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoReactionProductEnumerate(reaction.self, monomers.self));
      }

      public IndigoObject createSaver (IndigoObject output, string format)
      {
         setSessionID();
         return new IndigoObject(this, output, _indigo_lib.indigoCreateSaver(output.self, format));
      }

      public IndigoObject createFileSaver (string filename, string format)
      {
         setSessionID();
         return new IndigoObject(this, _indigo_lib.indigoCreateFileSaver(filename, format));
      }

      public void transform (IndigoObject reaction, IndigoObject monomer)
      {
         setSessionID();
         _indigo_lib.indigoTransform(reaction.self, monomer.self);
      }

      public void free (int id)
      {
         setSessionID();
         _indigo_lib.indigoFree(id);
      }

      public String getDllPath ()
      {
         return _dllpath;
      }

      private static void _handleError (sbyte* message, Indigo self)
      {
         throw new IndigoException(new String(message));
      }

      private void init (string lib_path)
      {
         IndigoDllLoader dll_loader = IndigoDllLoader.Instance;

         dll_loader.loadLibrary(lib_path, "msvcr100.dll", "com.ggasoftware.indigo.Properties.Resources", false);
         dll_loader.loadLibrary(lib_path, "msvcp100.dll", "com.ggasoftware.indigo.Properties.Resources", false);
         //dll_loader.loadLibrary(lib_path, "zlib.dll", "com.ggasoftware.indigo.Properties.Resources", false);
         dll_loader.loadLibrary(lib_path, "indigo.dll", "com.ggasoftware.indigo.Properties.Resources", false);

         // Save instance id to check if session was allocated for this instance
         _dll_loader_id = IndigoDllLoader.InstanceId;

         _dllpath = lib_path;

         _indigo_lib = dll_loader.getInterface<IndigoLib>("indigo.dll");

         _sid = _indigo_lib.indigoAllocSessionId();
         _indigo_lib.indigoSetSessionId(_sid);
         _errh = new ErrorHandler(_handleError);
         _indigo_lib.indigoSetErrorHandler(_errh, this);
      }

      ~Indigo ()
      {
         Dispose();
      }

      public void Dispose ()
      {
         if (_sid >= 0)
         {
            if (IndigoDllLoader.InstanceId == _dll_loader_id)
               _indigo_lib.indigoReleaseSessionId(_sid);
            _sid = -1;
         }
      }

      public void setSessionID ()
      {
         _indigo_lib.indigoSetSessionId(_sid);
      }

      private void onError ()
      {
         throw new IndigoException(new String(_indigo_lib.indigoGetLastError()));
      }

      private ErrorHandler _errh;
      private long _sid = -1;
      private String _dllpath;
      private int _dll_loader_id;

      public IndigoLib _indigo_lib = null;

      public delegate void ErrorHandler (sbyte* message, Indigo context);
   }
}
