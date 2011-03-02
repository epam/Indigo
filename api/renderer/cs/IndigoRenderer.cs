using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;

namespace com.ggasoftware.indigo
{
   public unsafe class IndigoRenderer
   {
      private Indigo _indigo;
      static private IndigoDllLoader dll_loader =
         new IndigoDllLoader("com.ggasoftware.indigo.Properties.Resources", typeof(IndigoRenderer).Assembly);

      public IndigoRenderer (Indigo indigo)
      {
         String dllpath = indigo.getDllPath();
         dll_loader.loadLibrary(dllpath, "indigo-renderer.dll", 2);

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
         IndigoObject bufh = _indigo.writeBuffer();
         indigoRender(obj.self, bufh.self);
         byte* buf;
         int bufsize;
         Indigo.indigoToBuffer(bufh.self, &buf, &bufsize);

         byte[] res = new byte[bufsize];
         for (int i = 0; i < bufsize; ++i)
            res[i] = buf[i];
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

      public byte[] renderGridToBuffer (IndigoObject items, int[] refatoms, int ncolumns)
      {
         IndigoObject bufh = _indigo.writeBuffer();

         if (refatoms != null)
            if (refatoms.Length != items.count())
               throw new IndigoException("renderGridToFile(): refatoms[] size must be equal to the number of objects");

         indigoRenderGrid(items.self, refatoms, ncolumns, bufh.self);
         return bufh.toBuffer();
      }

      public void renderGridToFile (IndigoObject items, int[] refatoms, int ncolumns, string filename)
      {
         if (refatoms != null)
            if (refatoms.Length != items.count())
               throw new IndigoException("renderGridToFile(): refatoms[] size must be equal to the number of objects");

         indigoRenderGridToFile(items.self, refatoms, ncolumns, filename);
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
      [DllImport("indigo-renderer.dll")]
      public static extern int indigoRenderGrid (int items, int[] refAtoms, int nColumns, int output);
      [DllImport("indigo-renderer.dll")]
      public static extern int indigoRenderGridToFile (int items, int[] refAtoms, int nColumns, string filename);
   }
}
