/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 * 
 * This file is part of Indigo toolkit.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

package com.epam.indigo;
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

   public String version ()
   {
      _indigo.setSessionID();
      return Indigo.checkResultString(this, _lib.indigoInchiVersion());
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
         String res = Indigo.extractFromJar(IndigoInchi.class, "/" + dllpath, filename);
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
