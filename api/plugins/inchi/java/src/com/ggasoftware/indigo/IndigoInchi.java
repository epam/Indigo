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
import com.sun.jna.Native;
import java.io.File;
import java.io.IOException;

public class IndigoInchi
{
   public IndigoInchi (Indigo indigo)
   {
      loadLibrary(indigo.getUserSpecifiedPath());
      _indigo = indigo;
   }

   public void resetOptions ()
   {
      _indigo.setSessionID();
      Indigo.checkResult(this, _lib.indigoInchiResetOptions());
   }
	
   public IndigoObject loadMolecule (String inchi)
   {
      _indigo.setSessionID();
      return new IndigoObject(_indigo, Indigo.checkResult(this, _lib.indigoInchiLoadMolecule(inchi)));
   }
   
   public String getInchi (IndigoObject molecule)
   {
      _indigo.setSessionID();
      return Indigo.checkResultString(this, _lib.indigoInchiGetInchi(molecule.self));
   }
   
   public String getInchiKey (String inchi)
   {
      _indigo.setSessionID();
      return Indigo.checkResultString(this, _lib.indigoInchiGetInchiKey(inchi));
   }
   
   public String getWarning ()
   {
      _indigo.setSessionID();
      return Indigo.checkResultString(this, _lib.indigoInchiGetWarning());
   }
      
   public String getLog ()
   {
      _indigo.setSessionID();
      return Indigo.checkResultString(this, _lib.indigoInchiGetLog());
   }
      
   public String getAuxInfo ()
   {
      _indigo.setSessionID();
      return Indigo.checkResultString(this, _lib.indigoInchiGetAuxInfo());
   }
         
   private static String getPathToBinary (String path, String filename)
   {
      String dllpath = Indigo.getPlatformDependentPath();

      if (path == null)
      {
         String res = Indigo.extractFromJar(IndigoInchi.class, "/com/ggasoftware/indigo/" + dllpath, filename);
         if (res != null)
            return res;
         path = "lib";
      }
      path = path + File.separator + dllpath + File.separator + filename;
      try
      {
         return (new File(path)).getCanonicalPath();
      }
      catch (IOException e)
      {
         return path;
      }
   }
   private synchronized static void loadLibrary (String path)
   {
      if (_lib != null)
         return;

      int os = Indigo.getOs();

      if (os == Indigo.OS_LINUX || os == Indigo.OS_SOLARIS)
         _lib = (IndigoInchiLib)Native.loadLibrary(getPathToBinary(path, "libindigo-inchi.so"), IndigoInchiLib.class);
      else if (os == Indigo.OS_MACOS)
         _lib = (IndigoInchiLib)Native.loadLibrary(getPathToBinary(path, "libindigo-inchi.dylib"), IndigoInchiLib.class);
      else // os == OS_WINDOWS
         _lib = (IndigoInchiLib)Native.loadLibrary(getPathToBinary(path, "indigo-inchi.dll"), IndigoInchiLib.class);
   }

   Indigo _indigo;
   static IndigoInchiLib _lib;
}
