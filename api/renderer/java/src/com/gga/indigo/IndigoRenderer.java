/****************************************************************************
 * Copyright (C) 2010 GGA Software Services LLC
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

package com.gga.indigo;
import com.gga.indigo.*;
import java.io.*;

public class IndigoRenderer
{
   public IndigoRenderer (Indigo indigo)
   {
      String full_dll_path = indigo.getFullDllPath();
      int os = Indigo.getOs();
      
      if (os == Indigo.OS_LINUX)
         System.load(full_dll_path + File.separator + "libindigo-renderer-jni.so");
         //System.loadLibrary("indigo-renderer-jni");
      else if (os == Indigo.OS_MACOS)
         System.load(full_dll_path + File.separator + "libindigo-renderer-jni.dylib");
      else // os == Indigo.OS_WINDOWS
         System.load(full_dll_path + File.separator + "indigo-renderer-jni.dll");

      _sid = indigo.getSid();
      _indigo = indigo;
   }

   public void render (IndigoObject obj, IndigoObject output)
   {
      indigoRender(obj.self, output.self);
   }

   public void renderToFile (IndigoObject obj, String filename)
   {
      indigoRenderToFile(obj.self, filename);
   }

   public byte[] renderToBuffer (IndigoObject obj)
   {
      int b = _indigo.indigoWriteBuffer();
      indigoRender(obj.self, b);
      byte[] result = _indigo.indigoToBuffer(b);
      _indigo.indigoFree(b);
      return result;
   }

   public native int indigoRender (int handle, int output);
   public native int indigoRenderToFile (int handle, String filename);

   private long _sid;
   private Indigo _indigo;
}
