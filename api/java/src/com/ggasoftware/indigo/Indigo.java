/****************************************************************************
 * Copyright (C) 2010-2011 GGA Software Services LLC
 *
 * This file is part of Indigo toolkit.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

package com.ggasoftware.indigo;

import java.io.*;
import java.lang.*;
import java.util.*;

public class Indigo
{
   public static final int ABS = 1;
   public static final int OR = 2;
   public static final int AND = 3;
   public static final int EITHER = 4;
   public static final int UP = 5;
   public static final int DOWN = 6;
   public static final int CIS = 7;
   public static final int TRANS = 8;
   public static final int CHAIN = 9;
   public static final int RING = 10;

   public native String version ();

   public int countReferences ()
   {
      return indigoCountReferences();
   }

   public void setOption (String option, String value)
   {
      indigoSetOption(option, value);
   }

   public void setOption (String option, int value)
   {
      indigoSetOptionInt(option, value);
   }

   public void setOption (String option, int x, int y)
   {
      indigoSetOptionXY(option, x, y);
   }

   public void setOption (String option, float r, float g, float b)
   {
      indigoSetOptionColor(option, r, g, b);
   }

   public void setOption (String option, boolean value)
   {
      indigoSetOptionBool(option, value ? 1 : 0);
   }

   public void setOption (String option, float value)
   {
      indigoSetOptionFloat(option, value);
   }

   public void setOption (String option, double value)
   {
      indigoSetOptionFloat(option, (float)value);
   }

   public IndigoObject writeFile (String filename)
   {
      return new IndigoObject(this, indigoWriteFile(filename));
   }

   public IndigoObject writeBuffer ()
   {
      return new IndigoObject(this, indigoWriteBuffer());
   }

   public IndigoObject createMolecule ()
   {
      return new IndigoObject(this, indigoCreateMolecule());
   }

   public IndigoObject createQueryMolecule ()
   {
      return new IndigoObject(this, indigoCreateQueryMolecule());
   }

   public IndigoObject loadMolecule (String str)
   {
      return new IndigoObject(this, indigoLoadMoleculeFromString(str));
   }

   public IndigoObject loadMolecule (byte[] buf)
   {
      return new IndigoObject(this, indigoLoadMoleculeFromBuffer(buf));
   }

   public IndigoObject loadMoleculeFromFile (String path)
   {
      return new IndigoObject(this, indigoLoadMoleculeFromFile(path));
   }

   public IndigoObject loadQueryMolecule (String str)
   {
      return new IndigoObject(this, indigoLoadQueryMoleculeFromString(str));
   }

   public IndigoObject loadQueryMolecule (byte[] buf)
   {
      return new IndigoObject(this, indigoLoadQueryMoleculeFromBuffer(buf));
   }

   public IndigoObject loadQueryMoleculeFromFile (String path)
   {
      return new IndigoObject(this, indigoLoadQueryMoleculeFromFile(path));
   }

   public IndigoObject loadSmarts (String str)
   {
      return new IndigoObject(this, indigoLoadSmartsFromString(str));
   }

   public IndigoObject loadSmarts (byte[] buf)
   {
      return new IndigoObject(this, indigoLoadSmartsFromBuffer(buf));
   }

   public IndigoObject loadSmartsFromFile (String path)
   {
      return new IndigoObject(this, indigoLoadSmartsFromFile(path));
   }

   public IndigoObject loadReaction (String str)
   {
      return new IndigoObject(this, indigoLoadReactionFromString(str));
   }

   public IndigoObject loadReaction (byte[] buf)
   {
      return new IndigoObject(this, indigoLoadReactionFromBuffer(buf));
   }

   public IndigoObject loadReactionFromFile (String path)
   {
      return new IndigoObject(this, indigoLoadReactionFromFile(path));
   }

   public IndigoObject loadQueryReaction (String str)
   {
      return new IndigoObject(this, indigoLoadQueryReactionFromString(str));
   }

   public IndigoObject loadQueryReaction (byte[] buf)
   {
      return new IndigoObject(this, indigoLoadQueryReactionFromBuffer(buf));
   }

   public IndigoObject loadQueryReactionFromFile (String path)
   {
      return new IndigoObject(this, indigoLoadQueryReactionFromFile(path));
   }

   public IndigoObject createReaction ()
   {
      return new IndigoObject(this, indigoCreateReaction());
   }

   public IndigoObject createQueryReaction ()
   {
      return new IndigoObject(this, indigoCreateQueryReaction());
   }

   public boolean exactMatch (IndigoObject obj1, IndigoObject obj2)
   {
      return indigoExactMatch(obj1.self, obj2.self) == 1;
   }

   public float similarity (IndigoObject obj1, IndigoObject obj2, String metrics)
   {
      if (metrics == null)
         metrics = "";
      return indigoSimilarity(obj1.self, obj2.self, metrics);
   }

   public int commonBits (IndigoObject fingerprint1, IndigoObject fingerprint2)
   {
      return indigoCommonBits(fingerprint1.self, fingerprint2.self);
   }

   public IndigoObject createArray ()
   {
      return new IndigoObject(this, indigoCreateArray());
   }

   public IndigoObject iterateSDFile (String filename)
   {
      return new IndigoObject(this, indigoIterateSDFile(filename));
   }

   public IndigoObject iterateRDFile (String filename)
   {
      return new IndigoObject(this, indigoIterateRDFile(filename));
   }

   public IndigoObject iterateSmilesFile (String filename)
   {
      return new IndigoObject(this, indigoIterateSmilesFile(filename));
   }

   public IndigoObject substructureMatcher (IndigoObject target, String mode)
   {
      return new IndigoObject(this, indigoSubstructureMatcher(target.self, mode));
   }

   public IndigoObject substructureMatcher (IndigoObject target)
   {
      return substructureMatcher(target, "");
   }

   public IndigoObject extractCommonScaffold (IndigoObject structures, String options)
   {
      int res = indigoExtractCommonScaffold(structures.self, options);

      if (res == 0)
         return null;

      return new IndigoObject(this, res);
   }

   public IndigoObject decomposeMolecules (IndigoObject scaffold, IndigoObject structures)
   {
      int res = indigoDecomposeMolecules(scaffold.self, structures.self);

      if (res == 0)
         return null;

      return new IndigoObject(this, res);
   }

   public IndigoObject reactionProductEnumerate (IndigoObject reaction, IndigoObject monomers)
   {
      int res = indigoReactionProductEnumerate(reaction.self, monomers.self);

      if (res == 0)
         return null;

      return new IndigoObject(this, res);
   }

   private String _full_dll_path = null;

   private static boolean checkIfLoaded ()
   {
      try
      {
         Vector libraries = (Vector)LIBRARIES.get(ClassLoader.getSystemClassLoader());

         Enumeration e = libraries.elements();
         while (e.hasMoreElements())
         {
            if (((String)e.nextElement()).indexOf("indigo") != -1)
               return true;
         }
      }
      catch (Exception e)
      {
      }
      return false;
   }

   private static java.lang.reflect.Field LIBRARIES = null;
   static
   {
      try
      {
         LIBRARIES = ClassLoader.class.getDeclaredField("loadedLibraryNames");
         LIBRARIES.setAccessible(true);
      } catch (NoSuchFieldException e) { }
   }

   private synchronized static void loadIndigo (String path)
   {
      if (checkIfLoaded())
         return;

      if (_os == OS_LINUX)
         System.load(path + File.separator + "libindigo-jni.so");
      else if (_os == OS_SOLARIS)
         System.load(path + File.separator + "libindigo-jni.so");
      else if (_os == OS_MACOS)
         System.load(path + File.separator + "libindigo-jni.dylib");
      else // _os == OS_WINDOWS
      {
         System.load(path + File.separator + "zlib.dll");
         System.load(path + File.separator + "indigo-jni.dll");
      }
   }

   public Indigo (String path)
   {
      path = path + File.separator + _dllpath;

      _full_dll_path = path;
      try
      {
         _full_dll_path = (new File(path)).getCanonicalPath();
      }
      catch (Exception e)
      {
      }

      loadIndigo(_full_dll_path);
      if (_os == OS_LINUX)
         indigoRtldGlobal(_full_dll_path + File.separator + "libindigo-jni.so");

      _sid = allocSessionId();
   }

   public Indigo ()
   {
      this("lib");
   }

   public String getFullDllPath ()
   {
      return _full_dll_path;
   }

   public long getSid ()
   {
      return _sid;
   }


   @Override
   protected void finalize () throws Throwable
   {
      releaseSessionId(_sid);
   }

   public static final int RTLD_LAZY = 0x00001;
   public static final int RTLD_NOW = 0x00002;
   public static final int RTLD_NOLOAD = 0x00004;
   public static final int RTLD_GLOBAL = 0x00100;

   
   private static native long allocSessionId ();
   private static native void releaseSessionId (long id);

   public native final int indigoRtldGlobal (String path);
   public native int indigoFree (int handle);
   public native int indigoClone (int handle);
   public native int indigoCountReferences ();

   public native int indigoSetOption (String option, String value);
   public native int indigoSetOptionInt (String option, int value);
   public native int indigoSetOptionBool (String option, int value);
   public native int indigoSetOptionFloat (String option, float value);
   public native int indigoSetOptionXY (String option, int x, int y);
   public native int indigoSetOptionColor (String option, float r, float g, float b);

   public native int indigoReadFile   (String filename);
   public native int indigoLoadString (String str);
   public native int indigoLoadBuffer (byte[] buf);

   public native int indigoWriteFile   (String filename);
   public native int indigoWriteBuffer ();
   public native int indigoClose       (int item);

   public native int indigoCreateMolecule ();
   public native int indigoCreateQueryMolecule ();
   public native int indigoLoadMolecule (int source);
   public native int indigoLoadMoleculeFromFile   (String filename);
   public native int indigoLoadMoleculeFromString (String source);
   public native int indigoLoadMoleculeFromBuffer (byte[] buf);

   public native int indigoLoadQueryMolecule (int source);
   public native int indigoLoadQueryMoleculeFromString (String source);
   public native int indigoLoadQueryMoleculeFromFile   (String filename);
   public native int indigoLoadQueryMoleculeFromBuffer (byte[] buf);
   
   public native int indigoLoadSmarts (int source);
   public native int indigoLoadSmartsFromString (String source);
   public native int indigoLoadSmartsFromFile   (String filename);
   public native int indigoLoadSmartsFromBuffer (byte[] buf);

   public native int indigoSaveMolfile (int molecule, int output);
   public native int indigoSaveMolfileToFile (int molecule, String filename);
   public native String indigoMolfile (int molecule);
   
   public native int indigoSaveCml (int molecule, int output);
   public native int indigoSaveCmlToFile (int molecule, String filename);
   public native String indigoCml (int molecule);

   public native int indigoSaveMDLCT (int molecule, int output);

   public native int indigoLoadReaction (int source);
   public native int indigoLoadReactionFromString (String source);
   public native int indigoLoadReactionFromFile   (String filename);
   public native int indigoLoadReactionFromBuffer (byte[] buf);

   public native int indigoLoadQueryReaction (int source);
   public native int indigoLoadQueryReactionFromString (String source);
   public native int indigoLoadQueryReactionFromFile   (String filename);
   public native int indigoLoadQueryReactionFromBuffer (byte[] buf);

   public native int indigoCreateReaction ();
   public native int indigoCreateQueryReaction ();
   
   public native int indigoAddReactant (int reaction, int molecule);
   public native int indigoAddProduct  (int reaction, int molecule);
   
   public native int indigoCountReactants (int handle);
   public native int indigoCountProducts  (int handle);
   public native int indigoCountMolecules (int handle);
   
   public native int indigoIterateReactants (int handle);
   public native int indigoIterateProducts  (int handle);
   public native int indigoIterateMolecules (int handle);

   public native int indigoSaveRxnfile (int molecule, int output);
   public native int indigoSaveRxnfileToFile (int molecule, String filename);
   public native String indigoRxnfile (int molecule);
   public native int indigoAutomap (int reaction, String mode);

   public native int indigoIterateAtoms (int molecule);
   public native int indigoIteratePseudoatoms (int molecule);
   public native int indigoIterateRSites (int molecule);
   public native int indigoIterateStereocenters (int molecule);
   public native int indigoIterateRGroups (int molecule);
   public native int indigoIterateRGroupFragments (int rgroup);
   public native int indigoCountAttachmentPoints (int rgroup);
   public native int indigoIsPseudoatom (int atom);
   public native int indigoIsRSite (int atom);
   public native int indigoStereocenterType (int atom);
   public native int indigoSingleAllowedRGroup (int atom);
   public native String indigoSymbol (int atom);

   public native int indigoDegree (int atom);
   public native Integer indigoGetCharge (int atom);
   public native Integer indigoGetExplicitValence (int atom);
   public native Integer indigoGetRadicalElectrons (int atom);
   public native int indigoAtomicNumber (int atom);
   public native int indigoIsotope (int atom);
   public native float[] indigoXYZ (int atom);
   public native int indigoCountSuperatoms (int molecule);
   public native int indigoCountDataSGroups (int molecule);
   public native int indigoIterateDataSGroups (int molecule);
   public native String indigoDescription (int data_sgroup);
   public native int indigoAddDataSGroup (int molecule, int[] atoms, int[] bonds, String description, String data);
   public native int indigoSetDataSGroupXY (int sgroup, float x, float y, String options);

   public native int indigoResetCharge (int atom);
   public native int indigoResetExplicitValence (int atom);
   public native int indigoResetRadical (int atom);
   public native int indigoResetIsotope (int atom);

   public native int indigoSetAttachmentPoint (int atom, int order);

   public native int indigoRemoveConstraints (int item, String type);
   public native int indigoAddConstraint     (int item, String type, String value);
   public native int indigoAddConstraintNot  (int item, String type, String value);

   public native int indigoResetStereo (int item);
   public native int indigoInvertStereo (int item);

   public native int indigoCountAtoms (int molecule);
   public native int indigoCountBonds (int molecule);
   public native int indigoCountPseudoatoms (int molecule);
   public native int indigoCountRSites (int molecule);

   public native int indigoIterateBonds (int molecule);
   public native int indigoBondOrder (int bond);
   public native int indigoBondStereo (int bond);
   public native int indigoTopology (int bond);

   public native int indigoIterateNeighbors (int atom);
   public native int indigoBond (int nei);
   public native int indigoGetAtom (int molecule, int idx);
   public native int indigoGetBond (int molecule, int idx);
   public native int indigoSource (int bond);
   public native int indigoDestination (int bond);

   public native int indigoClearCisTrans (int molecule);
   public native int indigoClearStereocenters (int molecule);
   public native int indigoCountStereocenters (int molecule);

   public native int indigoResetSymmetricCisTrans (int handle);

   public native int indigoAddAtom (int molecule, String symbol);
   public native int indigoSetCharge (int atom, int charge);
   public native int indigoSetIsotope (int atom, int isotope);
   public native int indigoAddBond (int source, int destination, int order);
   public native int indigoSetOrder (int bond, int order);
   public native int indigoMerge (int where, int what);

   public native int indigoCountComponents (int molecule);
   public native int indigoComponentIndex (int atom);
   public native int indigoIterateComponents (int molecule);
   public native int indigoComponent (int molecule, int index);

   public native int indigoCountHeavyAtoms (int molecule);
   public native int indigoGrossFormula (int molecule);
   public native float indigoMolecularWeight (int molecule);
   public native float indigoMostAbundantMass (int molecule);
   public native float indigoMonoisotopicMass (int molecule);

   public native String indigoCanonicalSmiles (int handle);
   public native String indigoLayeredCode (int handle);

   public native int indigoHasZCoord (int molecule);
   public native int indigoIsChiral (int molecule);

   public native int indigoCreateSubmolecule (int molecule, int[] vertices);
   public native int indigoCreateEdgeSubmolecule (int molecule, int[] vertices, int[] edges);
   public native int indigoRemoveAtoms (int molecule, int[] vertices);

   public native float indigoAlignAtoms (int molecule, int[] atom_ids, float[] desired_xyz);

   public native float indigoSimilarity (int molecule1, int molecule2, String metrics);
   
   public native int indigoAromatize (int item);
   public native int indigoDearomatize (int item);
   public native int indigoFoldHydrogens (int item);
   public native int indigoUnfoldHydrogens (int item);
   public native int indigoLayout (int item);

   public native String indigoSmiles (int item);
   
   public native int indigoExactMatch (int item1, int item2);

   public native String indigoName (int handle);
   public native int indigoSetName (int handle, String name);

   public native int indigoHasProperty (int handle, String prop);
   public native String indigoGetProperty (int handle, String prop);
   public native int indigoSetProperty (int handle, String prop, String value);
   public native int indigoRemoveProperty (int handle, String prop);
   public native int indigoIterateProperties (int handle);

   public native String indigoCheckBadValence (int handle);
   public native String indigoCheckAmbiguousH (int handle);

   public native int indigoFingerprint (int item, String type);
   public native int indigoCountBits (int fingerprint);
   public native int indigoCommonBits (int fingerprint1, int fingerprint2);

   public native int indigoIterateSDF (int reader);
   public native int indigoIterateRDF (int reader);
   public native int indigoIterateSmiles (int reader);

   public native int indigoIterateSDFile (String filename);
   public native int indigoIterateRDFile (String filename);
   public native int indigoIterateSmilesFile (String filename);
   
   public native String indigoRawData (int handle);
   public native int indigoTell (int handle);

   public native int indigoSdfAppend (int output, int item);
   public native int indigoSmilesAppend (int output, int item);
   public native int indigoRdfHeader (int output);
   public native int indigoRdfAppend (int output, int item);

   public native int indigoCreateArray ();
   public native int indigoArrayAdd (int arr, int obj);
   public native int indigoAt (int arr, int index);
   public native int indigoCount (int arr);
   public native int indigoClear (int arr);
   public native int indigoIterateArray (int arr);

   public native int indigoSubstructureMatcher (int target, String mode);
   public native int indigoMatch (int matcher, int query);
   public native int indigoCountMatches (int matcher, int query);
   public native int indigoIterateMatches (int matcher, int query);
   public native int indigoHighlightedTarget (int match);
   public native int indigoMapAtom (int match, int query_atom);
   public native int indigoMapBond (int match, int query_bond);

   public native int indigoExtractCommonScaffold (int structures, String options);
   public native int indigoAllScaffolds (int extracted);

   public native int indigoDecomposeMolecules (int scaffold, int structures);
   public native int indigoDecomposedMoleculeScaffold (int decomp);
   public native int indigoIterateDecomposedMolecules (int decomp);
   public native int indigoDecomposedMoleculeHighlighted (int decomp);
   public native int indigoDecomposedMoleculeWithRGroups (int decomp);

   public native int indigoNext (int iter);
   public native int indigoHasNext (int iter);
   public native int indigoIndex (int item);
   public native int indigoRemove (int item);
   
   public native String indigoToString (int handle);
   public native byte[] indigoToBuffer (int handle);

   public native int indigoReactionProductEnumerate (int reaction, int monomers);

   private long _sid;

   ////////////////////////////////////////////////////////////////
   // INITIALIZATION
   ////////////////////////////////////////////////////////////////
   public static final int OS_WINDOWS = 1;
   public static final int OS_MACOS = 2;
   public static final int OS_LINUX = 3;
   public static final int OS_SOLARIS = 4;

   private static int _os = 0;
   private static String _dllpath = "";

   public static int getOs ()
   {
      String namestr = System.getProperty("os.name");
       if (namestr.matches("^Windows.*"))
           return OS_WINDOWS;
       else if (namestr.matches("^Mac OS.*"))
           return OS_MACOS;
       else if (namestr.matches("^Linux.*"))
           return OS_LINUX;
       else if (namestr.matches("^SunOS.*"))
           return OS_SOLARIS;
       else
           throw new Error("Operating system not recognized");
   }

   private static String getDllPath ()
   {
      String path = "";
      switch (_os)
      {
         case OS_WINDOWS:
            path += "Win";
            break;
         case OS_LINUX:
            path += "Linux";
            break;
         case OS_SOLARIS:
            path += "Sun";
            break;
         case OS_MACOS:
            path += "Mac";
            break;
         default:
            throw new Error("OS not set");
      }
      path += File.separator;

      if (_os == OS_MACOS)
      {
         String version = System.getProperty("os.version");
         
         if (version.startsWith("10.5"))
            path += "10.5";
         else if (version.startsWith("10.6"))
            path += "10.6";
         else
            throw new Error("OS version not supported");
      }
      else if (_os == OS_SOLARIS)
      {
         String model = System.getProperty("sun.arch.data.model");

         if (model.equals("32"))
            path += "sparc32";
         else
            path += "sparc64";
      }
      else
      {
          String archstr = System.getProperty("os.arch");
          if (archstr.equals("x86") || archstr.equals("i386"))
               path += "x86";
          else if (archstr.equals("x86_64") || archstr.equals("amd64"))
               path += "x64";
          else
              throw new Error("architecture not recognized");
      }

      return path;
   }
   
   static
   {
      _os = getOs();
      _dllpath = getDllPath();
   }
}
