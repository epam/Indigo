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
import com.sun.jna.*;
import java.lang.reflect.*;
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

   public static final int RC_NOT_CENTER = -1;
   public static final int RC_UNMARKED = 0;
   public static final int RC_CENTER = 1;
   public static final int RC_UNCHANGED = 2;
   public static final int RC_MADE_OR_BROKEN = 4;
   public static final int RC_ORDER_CHANGED = 8;
           
   // JNA does not allow throwing exception from callbacks, thus we can not
   // use the error handler and we have to check the error codes. Below are
   // four functions to ease checking them.

   static public int checkResult (Object obj, int result)
   {
      if (result < 0)
         throw new IndigoException(obj, _lib.indigoGetLastError());
      return result;
   }

   static public float checkResultFloat (Object obj, float result)
   {
      if (result < 0)
         throw new IndigoException(obj, _lib.indigoGetLastError());
      return result;
   }

   static public String checkResultString (Object obj, Pointer result)
   {
      if (result == Pointer.NULL)
         throw new IndigoException(obj, _lib.indigoGetLastError());

      return result.getString(0, false);
   }

   static public Pointer checkResultPointer (Object obj, Pointer result)
   {
      if (result == Pointer.NULL)
         throw new IndigoException(obj, _lib.indigoGetLastError());

      return result;
   }

   public String version ()
   {
      return _lib.indigoVersion();
   }

   public int countReferences ()
   {
      setSessionID();
      return checkResult(this, _lib.indigoCountReferences());
   }

   public void setSessionID ()
   {
      _lib.indigoSetSessionId(_sid);
   }

   public void setOption (String option, String value)
   {
      setSessionID();
      checkResult(this, _lib.indigoSetOption(option, value));
   }

   public void setOption (String option, int value)
   {
      setSessionID();
      checkResult(this, _lib.indigoSetOptionInt(option, value));
   }

   public void setOption (String option, int x, int y)
   {
      setSessionID();
      checkResult(this, _lib.indigoSetOptionXY(option, x, y));
   }

   public void setOption (String option, float r, float g, float b)
   {
      setSessionID();
      checkResult(this, _lib.indigoSetOptionColor(option, r, g, b));
   }

   public void setOption (String option, boolean value)
   {
      setSessionID();
      checkResult(this, _lib.indigoSetOptionBool(option, value ? 1 : 0));
   }

   public void setOption (String option, float value)
   {
      setSessionID();
      checkResult(this, _lib.indigoSetOptionFloat(option, value));
   }

   public void setOption (String option, double value)
   {
      setSessionID();
      checkResult(this, _lib.indigoSetOptionFloat(option, (float)value));
   }

   public IndigoObject writeFile (String filename)
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this, _lib.indigoWriteFile(filename)));
   }

   public IndigoObject writeBuffer ()
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this, _lib.indigoWriteBuffer()));
   }

   public IndigoObject createMolecule ()
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this, _lib.indigoCreateMolecule()));
   }

   public IndigoObject createQueryMolecule ()
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this, _lib.indigoCreateQueryMolecule()));
   }

   public IndigoObject loadMolecule (String str)
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this, _lib.indigoLoadMoleculeFromString(str)));
   }

   public IndigoObject loadMolecule (byte[] buf)
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this, _lib.indigoLoadMoleculeFromBuffer(buf, buf.length)));
   }

   public IndigoObject loadMoleculeFromFile (String path)
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this, _lib.indigoLoadMoleculeFromFile(path)));
   }

   public IndigoObject loadQueryMolecule (String str)
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this, _lib.indigoLoadQueryMoleculeFromString(str)));
   }

   public IndigoObject loadQueryMolecule (byte[] buf)
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this, _lib.indigoLoadQueryMoleculeFromBuffer(buf, buf.length)));
   }

   public IndigoObject loadQueryMoleculeFromFile (String path)
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this, _lib.indigoLoadQueryMoleculeFromFile(path)));
   }

   public IndigoObject loadSmarts (String str)
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this, _lib.indigoLoadSmartsFromString(str)));
   }

   public IndigoObject loadSmarts (byte[] buf)
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this, _lib.indigoLoadSmartsFromBuffer(buf, buf.length)));
   }

   public IndigoObject loadSmartsFromFile (String path)
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this, _lib.indigoLoadSmartsFromFile(path)));
   }

   public IndigoObject loadReaction (String str)
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this, _lib.indigoLoadReactionFromString(str)));
   }

   public IndigoObject loadReaction (byte[] buf)
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this, _lib.indigoLoadReactionFromBuffer(buf, buf.length)));
   }

   public IndigoObject loadReactionFromFile (String path)
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this, _lib.indigoLoadReactionFromFile(path)));
   }

   public IndigoObject loadQueryReaction (String str)
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this, _lib.indigoLoadQueryReactionFromString(str)));
   }

   public IndigoObject loadQueryReaction (byte[] buf)
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this,  _lib.indigoLoadQueryReactionFromBuffer(buf, buf.length)));
   }

   public IndigoObject loadQueryReactionFromFile (String path)
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this, _lib.indigoLoadQueryReactionFromFile(path)));
   }

   public IndigoObject loadReactionSmarts (String str)
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this, _lib.indigoLoadReactionSmartsFromString(str)));
   }

   public IndigoObject loadReactionSmarts (byte[] buf)
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this,  _lib.indigoLoadReactionSmartsFromBuffer(buf, buf.length)));
   }

   public IndigoObject loadReactionSmartsFromFile (String path)
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this, _lib.indigoLoadReactionSmartsFromFile(path)));
   }

   public IndigoObject createReaction ()
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this, _lib.indigoCreateReaction()));
   }

   public IndigoObject createQueryReaction ()
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this, _lib.indigoCreateQueryReaction()));
   }

   public IndigoObject exactMatch (IndigoObject obj1, IndigoObject obj2, String flags)
   {
      setSessionID();
      if (flags == null)
         flags = "";

      int match = checkResult(this, _lib.indigoExactMatch(obj1.self, obj2.self, flags));

      if (match == 0)
         return null;

      return new IndigoObject(this, new IndigoObject[]{obj1, obj2}, match);
   }

   public IndigoObject exactMatch (IndigoObject obj1, IndigoObject obj2)
   {
      return exactMatch(obj1, obj2, "");
   }

   public void setTautomerRule (int id, String beg, String end)
   {
      setSessionID();
      checkResult(this, _lib.indigoSetTautomerRule(id, beg, end));
   }

   public void removeTautomerRule (int id)
   {
      setSessionID();
      checkResult(this, _lib.indigoRemoveTautomerRule(id));
   }

   public void clearTautomerRules ()
   {
      setSessionID();
      checkResult(this, _lib.indigoClearTautomerRules());
   }

   public float similarity (IndigoObject obj1, IndigoObject obj2)
   {
      return similarity(obj1, obj2, "");
   }

   public float similarity (IndigoObject obj1, IndigoObject obj2, String metrics)
   {
      if (metrics == null)
         metrics = "";
      setSessionID();
      return checkResultFloat(this, _lib.indigoSimilarity(obj1.self, obj2.self, metrics));
   }

   public int commonBits (IndigoObject fingerprint1, IndigoObject fingerprint2)
   {
      setSessionID();
      return checkResult(this, _lib.indigoCommonBits(fingerprint1.self, fingerprint2.self));
   }

   public IndigoObject unserialize (byte[] data)
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this, _lib.indigoUnserialize(data, data.length)));
   }
   
   public IndigoObject createArray ()
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this, _lib.indigoCreateArray()));
   }

   public IndigoObject iterateSDFile (String filename)
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this, _lib.indigoIterateSDFile(filename)));
   }

   public IndigoObject iterateRDFile (String filename)
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this, _lib.indigoIterateRDFile(filename)));
   }

   public IndigoObject iterateSmilesFile (String filename)
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this, _lib.indigoIterateSmilesFile(filename)));
   }

   public IndigoObject iterateCMLFile (String filename)
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this, _lib.indigoIterateCMLFile(filename)));
   }

   public IndigoObject substructureMatcher (IndigoObject target, String mode)
   {
      setSessionID();
      return new IndigoObject(this, target, checkResult(this, _lib.indigoSubstructureMatcher(target.self, mode)));
   }

   public IndigoObject substructureMatcher (IndigoObject target)
   {
      return substructureMatcher(target, "");
   }

   public IndigoObject extractCommonScaffold (IndigoObject structures, String options)
   {
      setSessionID();
      int res = checkResult(this, _lib.indigoExtractCommonScaffold(structures.self, options));

      if (res == 0)
         return null;

      return new IndigoObject(this, res);
   }

   public IndigoObject extractCommonScaffold (AbstractCollection<IndigoObject> structures, String options)
   {
      return extractCommonScaffold(toIndigoArray(structures), options);
   }

   public IndigoObject decomposeMolecules (IndigoObject scaffold, IndigoObject structures)
   {
      setSessionID();
      int res = checkResult(this, _lib.indigoDecomposeMolecules(scaffold.self, structures.self));

      if (res == 0)
         return null;

      return new IndigoObject(this, res);
   }

   public IndigoObject decomposeMolecules (IndigoObject scaffold, AbstractCollection<IndigoObject> structures)
   {
      return decomposeMolecules(scaffold, toIndigoArray(structures));
   }
   
   public IndigoObject reactionProductEnumerate (IndigoObject reaction, IndigoObject monomers)
   {
      setSessionID();
      int res = checkResult(this, _lib.indigoReactionProductEnumerate(reaction.self, monomers.self));

      if (res == 0)
         return null;

      return new IndigoObject(this, res);
   }

   public void transform (IndigoObject reaction, IndigoObject monomers)
   {
      setSessionID();
      _lib.indigoTransform(reaction.self, monomers.self);
   }
   
   public IndigoObject createSaver (IndigoObject output, String format)
   {
      setSessionID();
      return new IndigoObject(this, output, checkResult(this, _lib.indigoCreateSaver(output.self, format)));
   }

   public IndigoObject createFileSaver (String filename, String format)
   {
      setSessionID();
      return new IndigoObject(this, checkResult(this, _lib.indigoCreateFileSaver(filename, format)));
   }

   public void dbgBreakpoint ()
   {
      setSessionID();
      _lib.indigoDbgBreakpoint();
   }

   public IndigoObject toIndigoArray (AbstractCollection<IndigoObject> coll)
   {
      setSessionID();

      IndigoObject arr = createArray();
      for (IndigoObject obj : coll)
         arr.arrayAdd(obj);

      return arr;
   }

   public static int[] toIntArray (AbstractCollection<Integer> collection)
   {
      if (collection == null)
         return new int[0];

      int[] res = new int[collection.size()];
      int i = 0;

      for (Integer x : collection)
         res[i++] = x.intValue();

      return res;
   }

   public static float[] toFloatArray (AbstractCollection<Float> collection)
   {
      if (collection == null)
         return new float[0];

      float[] res = new float[collection.size()];
      int i = 0;

      for (Float x : collection)
         res[i++] = x.floatValue();

      return res;
   }
   
   public static class LibraryRemover
   {
      ArrayList<String> files = new ArrayList<String>();
      ArrayList<String> directories = new ArrayList<String>();
      
      public LibraryRemover ()
      {
         final LibraryRemover self = this;
         
         Runtime.getRuntime().addShutdownHook(new Thread () {
            @Override
            public void run ()
            {
               self.removeLibraries();
            }
            });
      }
      
      public synchronized void addLibrary (String directory, String fullpath)
      {
         files.add(fullpath);
         directories.add(directory);
         
         if (_os == OS_WINDOWS)
         {
            // The caller can load our DLL file with System.load() OR with
            // Native.loadLibrary(). To get the mess below working in the second
            // case, we call System.load() by ourselves. This makes the library
            // listed in the hidden ClassLoader.nativeLibraries field.
            System.load(fullpath);
         }
      }
      
      @SuppressWarnings("CallToThreadDumpStack")
      public synchronized void removeLibraries ()
      {
         for (int idx = files.size() - 1; idx >= 0; idx--)
         {
            String fullpath = files.get(idx);
            
            if (_os == OS_WINDOWS)
            {
               // In Windows, we can not remove the DLL file until we unload
               // it from the process. Nobody cares that the DLL files are
               // usually read into memory and the process does not need them
               // on the disk.
               try
               {
                  ClassLoader cl = Indigo.class.getClassLoader();
                  Field f = ClassLoader.class.getDeclaredField("nativeLibraries");
                  f.setAccessible(true);
                  List libs = (List)f.get(cl);
                  for (Iterator i = libs.iterator(); i.hasNext();)
                  {
                     Object lib = i.next();
                     f = lib.getClass().getDeclaredField("name");
                     f.setAccessible(true);
                     String name = (String)f.get(lib);
                     if (name.equals(fullpath))
                     {
                        Method m = lib.getClass().getDeclaredMethod("finalize", new Class[0]);
                        m.setAccessible(true);
                        // Here comes the trick: we call the finalizer twice,
                        // first time to undo our own System.load() above, and
                        // the second time to undo
                        // Native.loadLibrary/System.load() done by the caller.
                        // Each finalize() call decrements the "reference
                        // counter" of the process for the DLL file. After the
                        // counter is zero, the deletion of the file becomes 
                        // possible.
                        m.invoke(lib, new Object[0]);
                        m.invoke(lib, new Object[0]);
                     }
                  }
               }
               catch (Exception e)
               {
                  e.printStackTrace();
               }
            }
            (new File(fullpath)).delete();
            (new File(directories.get(idx))).delete();
         }
      }
   }
   
   static LibraryRemover _library_remover = new Indigo.LibraryRemover();
   
   public static String extractFromJar (Class cls, String path, String filename)
   {
      InputStream stream = cls.getResourceAsStream(path + "/" + filename);

      if (stream == null)
         return null;

      String tmpdir_path;
    
      try
      {
         File tmpfile = File.createTempFile("indigo", null);
         tmpdir_path = tmpfile.getAbsolutePath() + ".d";
         tmpfile.delete();
      }
      catch (IOException e)
      {
         return null;
      }

      final File tmpdir = new File(tmpdir_path);
      if (!tmpdir.mkdir())
         return null;

      final File dllfile = new File(tmpdir.getAbsolutePath() + File.separator + filename);

      try
      {
         FileOutputStream outstream = new FileOutputStream(dllfile);
         byte buf[]= new byte[4096];
         int len;

         while ((len = stream.read(buf)) > 0)
            outstream.write(buf, 0, len);
         
         outstream.close();
         stream.close();
      }
      catch (IOException e)
      {
         return null;
      }

      String p;
      
      try
      {
         p = dllfile.getCanonicalPath();
      }
      catch (IOException e)
      {
         return null;
      }
      
      final String fullpath = p;
      
      // To remove the temporary file and the directory on program's exit.
      _library_remover.addLibrary(tmpdir_path, fullpath);
      
      return fullpath;
   }

   private static String getPathToBinary (String path, String filename)
   {
      if (path == null)
      {
         String res = extractFromJar(Indigo.class, "/com/ggasoftware/indigo/" + _dllpath, filename);
         if (res != null)
            return res;
         path = "lib";
      }
      path = path + File.separator + _dllpath + File.separator + filename;
      try
      {
         return (new File(path)).getCanonicalPath();
      }
      catch (IOException e)
      {
         return path;
      }
   }

   private synchronized static void loadIndigo (String path)
   {
      if (_lib != null)
         return;

      if (_os == OS_LINUX || _os == OS_SOLARIS)
         _lib = (IndigoLib)Native.loadLibrary(getPathToBinary(path, "libindigo.so"), IndigoLib.class);
      else if (_os == OS_MACOS)
         _lib = (IndigoLib)Native.loadLibrary(getPathToBinary(path, "libindigo.dylib"), IndigoLib.class);
      else // _os == OS_WINDOWS
      {
         if((new File(getPathToBinary(path, "msvcr100.dll"))).exists())
            System.load(getPathToBinary(path, "msvcr100.dll"));
         System.load(getPathToBinary(path, "zlib.dll"));
         _lib = (IndigoLib)Native.loadLibrary(getPathToBinary(path, "indigo.dll"), IndigoLib.class);
      }
   }

   public Indigo (String path)
   {
      _path = path;
      loadIndigo(path);

      _sid = _lib.indigoAllocSessionId();
   }

   public Indigo ()
   {
      this(null);
   }

   public String getUserSpecifiedPath ()
   {
      return _path;
   }

   static public String getPlatformDependentPath ()
   {
      return _dllpath;
   }

   public long getSid ()
   {
      return _sid;
   }


   @Override
   @SuppressWarnings("FinalizeDeclaration")
   protected void finalize () throws Throwable
   {
      _lib.indigoReleaseSessionId(_sid);
      super.finalize();
   }

   private String _path;
   private long _sid;

   public static final int OS_WINDOWS = 1;
   public static final int OS_MACOS = 2;
   public static final int OS_LINUX = 3;
   public static final int OS_SOLARIS = 4;

   private static int _os = 0;
   private static String _dllpath = "";
   private static IndigoLib _lib = null;

   public static IndigoLib getLibrary ()
   {
      return _lib;
   }

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
      path += "/";

      if (_os == OS_MACOS)
      {
         String version = System.getProperty("os.version");
         
         if (version.startsWith("10.5"))
            path += "10.5";
         else if (version.startsWith("10.6") || version.startsWith("10.7"))
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
