using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.IO;
using System.Drawing;

namespace com.gga.indigo
{
   public unsafe class Indigo : IDisposable
   {
      [DllImport("kernel32")]
      public static extern IntPtr LoadLibrary(string lpFileName);

      private long _sid = -1;

      public long getSID ()
      {
         return _sid;
      }

      public String version ()
      {
         return new String(indigoVersion());
      }

      private static void _handleError (sbyte* message, Indigo self)
      {
         throw new IndigoException(new String(message));
      }

      private String dllpath;

      public String getDllPath ()
      {
         return dllpath;
      }

      private ErrorHandler _errh;

      private void init (string prefix)
      {
         string subprefix = (IntPtr.Size == 8) ? "Win/x64/" : "Win/x86/";

         dllpath = prefix + "\\" + subprefix;
         LoadLibrary(dllpath + "zlib.dll");
         LoadLibrary(dllpath + "indigo.dll");

         _sid = indigoAllocSessionId();
         indigoSetSessionId(_sid);
         _errh = new ErrorHandler(_handleError);
         indigoSetErrorHandler(_errh, this);
      }

      public Indigo (string prefix)
      {
         init(prefix);
      }

      public Indigo ()
      {
         init("./lib");
      }

      ~Indigo ()
      {
         Dispose();
      }

      public void Dispose()
      {
         if (_sid >= 0)
         {
            indigoReleaseSessionId(_sid);
            _sid = -1;
         }
      }

      public void setSessionID ()
      {
         indigoSetSessionId(_sid);
      }

      private void onError()
      {
         throw new IndigoException(new String(indigoGetLastError()));
      }

      public void free (int id)
      {
         setSessionID();
         indigoFree(id);
      }

      public int countReferences ()
      {
         setSessionID();
         return indigoCountReferences();
      }

      public void setOption (string name, string value)
      {
         setSessionID();
         indigoSetOption(name, value);
      }

      public void setOption (string name, int x, int y)
      {
         setSessionID();
         indigoSetOptionXY(name, x, y);
      }

      public void setOption (string name, bool value)
      {
         setSessionID();
         indigoSetOptionBool(name, value ? 1 : 0);
      }

      public void setOption (string name, float value)
      {
         setSessionID();
         indigoSetOptionFloat(name, value);
      }

      public void setOption (string name, Color value)
      {
         setSessionID();
         indigoSetOptionColor(name, value.R / 255.0f, value.G / 255.0f, value.B / 255.0f);
      }

      public IndigoObject writeFile (String filename)
      {
         setSessionID();
         return new IndigoObject(this, indigoWriteFile(filename));
      }

      public IndigoObject writeBuffer ()
      {
         setSessionID();
         return new IndigoObject(this, indigoWriteBuffer());
      }

      public IndigoObject loadMolecule (String str)
      {
         setSessionID();
         return new IndigoObject(this, indigoLoadMoleculeFromString(str));
      }

      public IndigoObject loadMolecule (byte[] buf)
      {
         setSessionID();
         return new IndigoObject(this, indigoLoadMoleculeFromBuffer(buf, buf.Length));
      }

      public IndigoObject loadMoleculeFromFile (String path)
      {
         setSessionID();
         return new IndigoObject(this, indigoLoadMoleculeFromFile(path));
      }

      public IndigoObject loadQueryMolecule (String str)
      {
         setSessionID();
         return new IndigoObject(this, indigoLoadQueryMoleculeFromString(str));
      }

      public IndigoObject loadQueryMolecule (byte[] buf)
      {
         setSessionID();
         return new IndigoObject(this, indigoLoadQueryMoleculeFromBuffer(buf, buf.Length));
      }

      public IndigoObject loadQueryMoleculeFromFile (String path)
      {
         setSessionID();
         return new IndigoObject(this, indigoLoadQueryMoleculeFromFile(path));
      }

      public IndigoObject loadSmarts (String str)
      {
         setSessionID();
         return new IndigoObject(this, indigoLoadSmartsFromString(str));
      }

      public IndigoObject loadSmarts (byte[] buf)
      {
         setSessionID();
         return new IndigoObject(this, indigoLoadSmartsFromBuffer(buf, buf.Length));
      }

      public IndigoObject loadSmartsFromFile (String path)
      {
         setSessionID();
         return new IndigoObject(this, indigoLoadSmartsFromFile(path));
      }

      public IndigoObject loadReaction (String str)
      {
         setSessionID();
         return new IndigoObject(this, indigoLoadReactionFromString(str));
      }

      public IndigoObject loadReaction (byte[] buf)
      {
         return new IndigoObject(this, indigoLoadReactionFromBuffer(buf, buf.Length));
      }

      public IndigoObject loadReactionFromFile (String path)
      {
         setSessionID();
         return new IndigoObject(this, indigoLoadReactionFromFile(path));
      }

      public IndigoObject loadQueryReaction (String str)
      {
         setSessionID();
         return new IndigoObject(this, indigoLoadQueryReactionFromString(str));
      }

      public IndigoObject loadQueryReaction (byte[] buf)
      {
         return new IndigoObject(this, indigoLoadQueryReactionFromBuffer(buf, buf.Length));
      }

      public IndigoObject loadQueryReactionFromFile (String path)
      {
         setSessionID();
         return new IndigoObject(this, indigoLoadQueryReactionFromFile(path));
      }

      public IndigoObject createReaction ()
      {
         setSessionID();
         return new IndigoObject(this, indigoCreateReaction());
      }

      public IndigoObject createQueryReaction ()
      {
         setSessionID();
         return new IndigoObject(this, indigoCreateQueryReaction());
      }

      public bool exactMatch (IndigoObject obj1, IndigoObject obj2)
      {
         setSessionID();
         return indigoExactMatch(obj1.self, obj2.self) == 1;
      }

      public IndigoObject createArray ()
      {
         setSessionID();
         return new IndigoObject(this, indigoCreateArray());
      }

      public float similarity (IndigoObject obj1, IndigoObject obj2, string metrics)
      {
         setSessionID();
         if (metrics == null)
            metrics = "";
         return indigoSimilarity(obj1.self, obj2.self, metrics);
      }

      public int commonBits (IndigoObject obj1, IndigoObject obj2)
      {
         setSessionID();
         return indigoCommonBits(obj1.self, obj2.self);
      }

      public System.Collections.IEnumerable iterateSDFile (string filename)
      {
         setSessionID();
         return new IndigoObject(this, Indigo.indigoIterateSDFile(filename));
      }

      public System.Collections.IEnumerable iterateRDFile (string filename)
      {
         setSessionID();
         return new IndigoObject(this, Indigo.indigoIterateRDFile(filename));
      }

      public System.Collections.IEnumerable iterateSmilesFile (string filename)
      {
         setSessionID();
         return new IndigoObject(this, Indigo.indigoIterateSmilesFile(filename));
      }

      public IndigoObject matchSubstructure (IndigoObject query, IndigoObject target)
      {
         setSessionID();
         int res = indigoMatchSubstructure(query.self, target.self);
         if (res == 0)
            return null;
         return new IndigoObject(this, res);
      }

      public int countSubstructureMatches (IndigoObject query, IndigoObject target)
      {
         setSessionID();
         return indigoCountSubstructureMatches(query.self, target.self);
      }

      public IndigoObject extractCommonScaffold (IndigoObject structures, string options)
      {
         setSessionID();
         int res = indigoExtractCommonScaffold(structures.self, options);
         if (res == 0)
            return null;
         return new IndigoObject(this, res);
      }

      public IndigoObject decomposeMolecules (IndigoObject scaffold, IndigoObject structures)
      {
         setSessionID();
         return new IndigoObject(this, indigoDecomposeMolecules(scaffold.self, structures.self));
      }

      public IndigoObject reactionProductEnumerate (IndigoObject reaction, IndigoObject monomers)
      {
         setSessionID();
         return new IndigoObject(this, indigoReactionProductEnumerate(reaction.self, monomers.self));
      }

      [DllImport("indigo.dll")]
      public static extern sbyte * indigoVersion ();

      public delegate void ErrorHandler (sbyte* message, Indigo context);

      [DllImport("indigo.dll")]
      public static extern long indigoAllocSessionId ();
      [DllImport("indigo.dll")]
      public static extern void indigoSetSessionId (long id);
      [DllImport("indigo.dll")]
      public static extern void indigoReleaseSessionId (long id);
      [DllImport("indigo.dll")]
      public static extern sbyte * indigoGetLastError ();
      [DllImport("indigo.dll")]
      public static extern void indigoSetErrorHandler (ErrorHandler handler, Indigo context);
      [DllImport("indigo.dll")]
      public static extern void indigoSetErrorMessage (String message);
      [DllImport("indigo.dll")]
      public static extern int indigoFree (int id);
      [DllImport("indigo.dll")]
      public static extern int indigoClone (int id);
      [DllImport("indigo.dll")]
      public static extern int indigoCountReferences ();

      [DllImport("indigo.dll")]
      public static extern int indigoSetOption (string name, string value);
      [DllImport("indigo.dll")]
      public static extern int indigoSetOptionInt (string name, int value);
      [DllImport("indigo.dll")]
      public static extern int indigoSetOptionBool (string name, int value);
      [DllImport("indigo.dll")]
      public static extern int indigoSetOptionFloat (string name, float value);
      [DllImport("indigo.dll")]
      public static extern int indigoSetOptionColor (string name, float r, float g, float b);
      [DllImport("indigo.dll")]
      public static extern int indigoSetOptionXY (string name, int x, int y);

      [DllImport("indigo.dll")]
      public static extern int indigoReadFile (string filename);
      [DllImport("indigo.dll")]
      public static extern int indigoReadString (string str);
      [DllImport("indigo.dll")]
      public static extern int indigoLoadString (string str);
      [DllImport("indigo.dll")]
      public static extern int indigoReadBuffer (string buffer, int size);
      [DllImport("indigo.dll")]
      public static extern int indigoLoadBuffer (string buffer, int size);
      [DllImport("indigo.dll")]
      public static extern int indigoWriteFile (string filename);
      [DllImport("indigo.dll")]
      public static extern int indigoWriteBuffer ();

      [DllImport("indigo.dll")]
      public static extern int indigoLoadMoleculeFromString (string str);
      [DllImport("indigo.dll")]
      public static extern int indigoLoadMoleculeFromFile (string path);
      [DllImport("indigo.dll")]
      public static extern int indigoLoadMoleculeFromBuffer (byte[] buf, int bufsize);
      [DllImport("indigo.dll")]
      public static extern int indigoLoadQueryMoleculeFromString (string str);
      [DllImport("indigo.dll")]
      public static extern int indigoLoadQueryMoleculeFromFile (string path);
      [DllImport("indigo.dll")]
      public static extern int indigoLoadQueryMoleculeFromBuffer (byte[] buf, int bufsize);
      [DllImport("indigo.dll")]
      public static extern int indigoLoadSmartsFromString (string str);
      [DllImport("indigo.dll")]
      public static extern int indigoLoadSmartsFromFile (string filename);
      [DllImport("indigo.dll")]
      public static extern int indigoLoadSmartsFromBuffer (byte[] buffer, int size);
      [DllImport("indigo.dll")]
      public static extern int indigoSaveMolfile (int molecule, int output);
      [DllImport("indigo.dll")]
      public static extern int indigoSaveMolfileToFile (int molecule, string filename);
      [DllImport("indigo.dll")]
      public static extern sbyte * indigoMolfile (int molecule);
      [DllImport("indigo.dll")]
      public static extern int indigoSaveCml(int molecule, int output);
      [DllImport("indigo.dll")]
      public static extern int indigoSaveCmlToFile (int molecule, string filename);
      [DllImport("indigo.dll")]
      public static extern sbyte * indigoCml (int molecule);
      [DllImport("indigo.dll")]
      public static extern int indigoSaveMDLCT (int item, int output);

      [DllImport("indigo.dll")]
      public static extern int indigoLoadReactionFromString (string str);
      [DllImport("indigo.dll")]
      public static extern int indigoLoadReactionFromFile (string path);
      [DllImport("indigo.dll")]
      public static extern int indigoLoadReactionFromBuffer (byte[] buf, int bufsize);
      [DllImport("indigo.dll")]
      public static extern int indigoLoadQueryReactionFromString (string str);
      [DllImport("indigo.dll")]
      public static extern int indigoLoadQueryReactionFromFile (string path);
      [DllImport("indigo.dll")]
      public static extern int indigoLoadQueryReactionFromBuffer (byte[] buf, int bufsize);
      [DllImport("indigo.dll")]
      public static extern int indigoCreateReaction ();
      [DllImport("indigo.dll")]
      public static extern int indigoCreateQueryReaction ();
      [DllImport("indigo.dll")]
      public static extern int indigoAddReactant (int reaction, int molecule);
      [DllImport("indigo.dll")]
      public static extern int indigoAddProduct (int reaction, int molecule);
      [DllImport("indigo.dll")]
      public static extern int indigoCountReactants (int handler);
      [DllImport("indigo.dll")]
      public static extern int indigoCountProducts (int handler);
      [DllImport("indigo.dll")]
      public static extern int indigoCountMolecules (int handler);
      [DllImport("indigo.dll")]
      public static extern int indigoIterateReactants (int reaction);
      [DllImport("indigo.dll")]
      public static extern int indigoIterateProducts (int reaction);
      [DllImport("indigo.dll")]
      public static extern int indigoIterateMolecules (int reader);
      [DllImport("indigo.dll")]
      public static extern int indigoSaveRxnfile (int reaction, int output);
      [DllImport("indigo.dll")]
      public static extern int indigoSaveRxnfileToFile (int reaction, string filename);
      [DllImport("indigo.dll")]
      public static extern sbyte * indigoRxnfile (int reaction);
      [DllImport("indigo.dll")]
      public static extern int indigoAutomap (int reaction, string filename);

      [DllImport("indigo.dll")]
      public static extern int indigoIterateAtoms (int molecule);
      [DllImport("indigo.dll")]
      public static extern int indigoIteratePseudoatoms (int molecule);
      [DllImport("indigo.dll")]
      public static extern int indigoIterateRSites (int molecule);
      [DllImport("indigo.dll")]
      public static extern int indigoIterateRGroups (int molecule);
      [DllImport("indigo.dll")]
      public static extern int indigoIterateRGroupFragments (int rgroup);
      [DllImport("indigo.dll")]
      public static extern int indigoCountAttachmentPoints (int rgroup);
      [DllImport("indigo.dll")]
      public static extern int indigoIsPseudoatom (int atom);
      [DllImport("indigo.dll")]
      public static extern int indigoIsRSite (int atom);
      [DllImport("indigo.dll")]
      public static extern int indigoSingleAllowedRGroup (int rsite);
      [DllImport("indigo.dll")]
      public static extern sbyte * indigoPseudoatomLabel (int atom);
      [DllImport("indigo.dll")]
      public static extern int indigoDegree (int atom);
      [DllImport("indigo.dll")]
      public static extern int indigoGetCharge (int atom, int* charge);
      [DllImport("indigo.dll")]
      public static extern int indigoGetExplicitValence (int atom, int* valence);
      [DllImport("indigo.dll")]
      public static extern int indigoGetRadicalElectrons (int atom, int* electrons);
      [DllImport("indigo.dll")]
      public static extern int indigoAtomNumber (int atom);
      [DllImport("indigo.dll")]
      public static extern int indigoAtomIsotope (int atom);
      [DllImport("indigo.dll")]
      public static extern int indigoCountAtoms (int molecule);
      [DllImport("indigo.dll")]
      public static extern int indigoCountBonds (int molecule);
      [DllImport("indigo.dll")]
      public static extern int indigoCountPseudoatoms (int molecule);
      [DllImport("indigo.dll")]
      public static extern int indigoCountRSites (int molecule);
      [DllImport("indigo.dll")]
      public static extern int indigoIterateBonds (int molecule);
      [DllImport("indigo.dll")]
      public static extern int indigoBondOrder (int molecule);
      [DllImport("indigo.dll")]
      public static extern int indigoBondStereo (int molecule);
      [DllImport("indigo.dll")]
      public static extern int indigoIterateNeighbors (int atom);
      [DllImport("indigo.dll")]
      public static extern int indigoBond (int nei);
      [DllImport("indigo.dll")]
      public static extern int indigoGetAtom (int molecule, int idx);
      [DllImport("indigo.dll")]
      public static extern int indigoGetBond (int molecule, int idx);
      [DllImport("indigo.dll")]
      public static extern int indigoCisTransClear (int molecule);
      [DllImport("indigo.dll")]
      public static extern int indigoStereocentersClear (int molecule);
      [DllImport("indigo.dll")]
      public static extern int indigoCountStereocenters (int molecule);

      [DllImport("indigo.dll")]
      public static extern int   indigoGrossFormula    (int molecule);
      [DllImport("indigo.dll")]
      public static extern float indigoMolecularWeight (int molecule);
      [DllImport("indigo.dll")]
      public static extern float indigoMostAbundantMass (int molecule);
      [DllImport("indigo.dll")]
      public static extern float indigoMonoisotopicMass (int molecule);
      [DllImport("indigo.dll")]
      public static extern sbyte * indigoCanonicalSmiles (int molecule);
      [DllImport("indigo.dll")]
      public static extern sbyte * indigoLayeredCode (int molecule);
      [DllImport("indigo.dll")]
      public static extern int indigoCountComponents (int molecule);
      [DllImport("indigo.dll")]
      public static extern int indigoCreateSubmolecule (int molecule, int nvertices, int[] vertices);
      [DllImport("indigo.dll")]
      public static extern int indigoCreateEdgeSubmolecule (int molecule, int nvertices, int[] vertices,
                                         int nedges, int[] edges);

      [DllImport("indigo.dll")]
      public static extern int indigoAromatize (int item);
      [DllImport("indigo.dll")]
      public static extern int indigoDearomatize (int item);
      [DllImport("indigo.dll")]
      public static extern int indigoLayout (int item);
      [DllImport("indigo.dll")]
      public static extern sbyte * indigoSmiles (int item);
      [DllImport("indigo.dll")]
      public static extern int indigoExactMatch (int item1, int item2);
      [DllImport("indigo.dll")]
      public static extern sbyte* indigoName (int item);
      [DllImport("indigo.dll")]
      public static extern int indigoSetName (int item, string name);
      [DllImport("indigo.dll")]
      public static extern int indigoHasProperty (int handle, string field);
      [DllImport("indigo.dll")]
      public static extern sbyte * indigoGetProperty (int handle, string field);
      [DllImport("indigo.dll")]
      public static extern int indigoSetProperty (int handle, string field, string value);
      [DllImport("indigo.dll")]
      public static extern int indigoIterateProperties (int handle);
      [DllImport("indigo.dll")]
      public static extern sbyte * indigoCheckBadValence (int handle);
      [DllImport("indigo.dll")]
      public static extern sbyte * indigoCheckAmbiguousH (int handle);

      [DllImport("indigo.dll")]
      public static extern int indigoFingerprint (int item, string type);
      [DllImport("indigo.dll")]
      public static extern int indigoCountBits (int fingerprint);
      [DllImport("indigo.dll")]
      public static extern int indigoCommonBits (int fingerprint1, int fingerprint2);
      [DllImport("indigo.dll")]
      public static extern float indigoSimilarity (int molecule1, int molecule2, string metrics);

      [DllImport("indigo.dll")]
      public static extern int indigoIterateSDF    (int reader);
      [DllImport("indigo.dll")]
      public static extern int indigoIterateRDF    (int reader);
      [DllImport("indigo.dll")]
      public static extern int indigoIterateSmiles (int reader);
      [DllImport("indigo.dll")]
      public static extern int indigoIterateSDFile (string filename);
      [DllImport("indigo.dll")]
      public static extern int indigoIterateRDFile (string filename);
      [DllImport("indigo.dll")]
      public static extern int indigoIterateSmilesFile (string filename);
      [DllImport("indigo.dll")]
      public static extern sbyte * indigoRawData (int item);
      [DllImport("indigo.dll")]
      public static extern int indigoTell (int item);
      [DllImport("indigo.dll")]
      public static extern int indigoSdfAppend (int output, int item);
      [DllImport("indigo.dll")]
      public static extern int indigoSmilesAppend (int output, int item);

      [DllImport("indigo.dll")]
      public static extern int indigoCreateArray ();
      [DllImport("indigo.dll")]
      public static extern int indigoArrayAdd (int arr, int item);
      [DllImport("indigo.dll")]
      public static extern int indigoArrayAt (int arr, int index);
      [DllImport("indigo.dll")]
      public static extern int indigoArrayCount (int arr);
      [DllImport("indigo.dll")]
      public static extern int indigoArrayClear (int arr);
      [DllImport("indigo.dll")]
      public static extern int indigoIterateArray (int arr);

      [DllImport("indigo.dll")]
      public static extern int indigoMatchSubstructure (int query, int target);
      [DllImport("indigo.dll")]
      public static extern int indigoMatchHighlight (int match);
      [DllImport("indigo.dll")]
      public static extern int indigoCountSubstructureMatches (int query, int target);

      [DllImport("indigo.dll")]
      public static extern int indigoExtractCommonScaffold (int structures, string options);
      [DllImport("indigo.dll")]
      public static extern int indigoAllScaffolds (int extracted);

      [DllImport("indigo.dll")]
      public static extern int indigoDecomposeMolecules (int scaffold, int structures);
      [DllImport("indigo.dll")]
      public static extern int indigoDecomposedMoleculeScaffold (int decomp);
      [DllImport("indigo.dll")]
      public static extern int indigoIterateDecomposedMolecules (int decomp);
      [DllImport("indigo.dll")]
      public static extern int indigoDecomposedMoleculeHighlighted (int decomp);
      [DllImport("indigo.dll")]
      public static extern int indigoDecomposedMoleculeSubstituents (int decomp);
      [DllImport("indigo.dll")]
      public static extern int indigoDecomposedMoleculeWithRGroups (int decomp);

      [DllImport("indigo.dll")]
      public static extern int indigoNext (int iter);
      [DllImport("indigo.dll")]
      public static extern int indigoIndex (int item);

      [DllImport("indigo.dll")]
      public static extern sbyte * indigoToString (int handle);
      [DllImport("indigo.dll")]
      public static extern int indigoToBuffer (int handle, byte **buf, int *size);

      [DllImport("indigo.dll")]
      public static extern int indigoReactionProductEnumerate (int reaction, int monomers);
   }
}
