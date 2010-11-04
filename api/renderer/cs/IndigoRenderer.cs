using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;

namespace com.scitouch.indigo
{
   public unsafe class IndigoRenderer
   {
      private Indigo _indigo;

      public IndigoRenderer (Indigo indigo)
      {
         String dllpath = indigo.getDllPath();

         Indigo.LoadLibrary(dllpath + "indigo-renderer.dll");
         _indigo = indigo;
      }

      public void renderToFile (IndigoObject obj, string filename)
      {
         _indigo.setSessionID();
         indigoRenderToFile(obj.self, filename);
      }

      public byte[] renderToBuffer (IndigoObject obj)
      {
         _indigo.setSessionID();
         int bufh = Indigo.indigoWriteBuffer();
         indigoRender(obj.self, bufh);
         byte* buf;
         int bufsize;
         Indigo.indigoToBuffer(bufh, &buf, &bufsize);

         byte[] res = new byte[bufsize];
         for (int i = 0; i < bufsize; ++i)
            res[i] = buf[i];
         Indigo.indigoFree(bufh);
         return res;
      }

      public void renderToHDC (IndigoObject obj, IntPtr hdc, bool printing)
      {
         _indigo.setSessionID();
         int hdch = indigoRenderWriteHDC((void*)hdc, printing ? 1 : 0);
         indigoRender(obj.self, hdch);
      }

      public Bitmap renderToBitmap (IndigoObject obj)
      {
         _indigo.setSessionID();

         Indigo.indigoSetOption("render-output-format", "png");
         byte[] res = renderToBuffer(obj);

         MemoryStream stream = new MemoryStream();
         stream.Write(res, 0, res.Length);
         stream.Seek(0, SeekOrigin.Begin);

         return (Bitmap)Image.FromStream(stream);
      }

      public Metafile renderToMetafile (IndigoObject obj)
      {
         _indigo.setSessionID();

         Indigo.indigoSetOption("render-output-format", "emf");
         byte[] res = renderToBuffer(obj);

         MemoryStream ms = new MemoryStream(res);
         Metafile mf = new Metafile(ms);

         return mf;
      }

      public static void SaveMetafile (Metafile mf, Stream stream)
      {
         IntPtr henh = mf.GetHenhmetafile();
         int size = GetEnhMetaFileBits(henh, 0, null);

         byte[] buffer = new byte[size];

         if (GetEnhMetaFileBits(henh, size, buffer) <= 0)
            throw new SystemException("GetEnhMetaFileBits");

         stream.Write(buffer, 0, buffer.Length);
         stream.Flush();
      }

      public static void SaveMetafile (Metafile mf, String path)
      {
         FileStream fs = new FileStream(path, FileMode.OpenOrCreate);
         SaveMetafile(mf, fs);
         fs.Close();
      }

      [DllImport("gdi32")]
      private static extern int GetEnhMetaFileBits (IntPtr hemf, int cbBuffer, byte[] lpbBuffer);

      [DllImport("indigo-renderer.dll")]
      public static extern int indigoRenderWriteHDC (void* hdc, int printingHdc);
      [DllImport("indigo-renderer.dll")]
      public static extern int indigoRender (int item, int output);
      [DllImport("indigo-renderer.dll")]
      public static extern int indigoRenderToFile (int item, string filename);
   }
}
