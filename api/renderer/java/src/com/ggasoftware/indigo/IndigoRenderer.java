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

public class IndigoRenderer
{
   public IndigoRenderer (Indigo indigo)
   {
      loadLibrary(indigo.getUserSpecifiedPath());
      _indigo = indigo;
   }

   public void render (IndigoObject obj, IndigoObject output)
   {
      _indigo.setSessionID();
      Indigo.checkResult(this, _lib.indigoRender(obj.self, output.self));
   }

   public void renderToFile (IndigoObject obj, String filename)
   {
      _indigo.setSessionID();
      Indigo.checkResult(this, _lib.indigoRenderToFile(obj.self, filename));
   }

   public byte[] renderToBuffer (IndigoObject obj)
   {
      _indigo.setSessionID();
      IndigoObject buf = _indigo.writeBuffer();
      Indigo.checkResult(this, _lib.indigoRender(obj.self, buf.self));

      return buf.toBuffer();
   }

   public void renderGridToFile (IndigoObject objects, int[] refAtoms, int ncolumns, String filename)
   {
      _indigo.setSessionID();
      if (refAtoms != null && objects.count() != refAtoms.length)
         throw new IndigoException(this, "refAtoms size does not match the number of objects");
      Indigo.checkResult(this, _lib.indigoRenderGridToFile(objects.self, refAtoms, ncolumns, filename));
   }

   public byte[] renderGridToBuffer (IndigoObject objects, int[] refAtoms, int ncolumns)
   {
      _indigo.setSessionID();
      if (objects.count() != refAtoms.length)
         throw new IndigoException(this, "refAtoms size does not match the number of objects");
      IndigoObject buf = _indigo.writeBuffer();
      Indigo.checkResult(this, _lib.indigoRenderGrid(objects.self, refAtoms, ncolumns, buf.self));
      return buf.toBuffer();
   }

   private static String getPathToBinary (String path, String filename)
   {
      String dllpath = Indigo.getPlatformDependentPath();

      if (path == null)
      {
         String res = Indigo.extractFromJar(IndigoRenderer.class, "/com/ggasoftware/indigo/" + dllpath, filename);
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
         _lib = (IndigoRendererLib)Native.loadLibrary(getPathToBinary(path, "libindigo-renderer.so"), IndigoRendererLib.class);
      else if (os == Indigo.OS_MACOS)
         _lib = (IndigoRendererLib)Native.loadLibrary(getPathToBinary(path, "libindigo-renderer.dylib"), IndigoRendererLib.class);
      else // os == OS_WINDOWS
         _lib = (IndigoRendererLib)Native.loadLibrary(getPathToBinary(path, "indigo-renderer.dll"), IndigoRendererLib.class);
   }

   Indigo _indigo;
   static IndigoRendererLib _lib;
}
