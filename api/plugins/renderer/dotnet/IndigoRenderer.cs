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
      private IndigoRendererLib _renderer_lib;

      public IndigoRenderer (Indigo indigo)
      {
         String dllpath = indigo.getDllPath();

         IndigoDllLoader dll_loader = IndigoDllLoader.Instance;
         dll_loader.loadLibrary(dllpath, "indigo-renderer.dll", 
            "com.ggasoftware.indigo.Properties.Resources", false);
         _renderer_lib = dll_loader.getInterface<IndigoRendererLib>("indigo-renderer.dll");

         _indigo = indigo;
      }

      public void renderToFile (IndigoObject obj, string filename)
      {
         _indigo.setSessionID();
         _renderer_lib.indigoRenderToFile(obj.self, filename);
      }

      public byte[] renderToBuffer (IndigoObject obj)
      {
         _indigo.setSessionID();
         IndigoObject bufh = _indigo.writeBuffer();
         _renderer_lib.indigoRender(obj.self, bufh.self);
         byte* buf;
         int bufsize;
         _indigo._indigo_lib.indigoToBuffer(bufh.self, &buf, &bufsize);

         byte[] res = new byte[bufsize];
         for (int i = 0; i < bufsize; ++i)
            res[i] = buf[i];
         return res;
      }

      public void renderToHDC (IndigoObject obj, IntPtr hdc, bool printing)
      {
         _indigo.setSessionID();
         int hdch = _renderer_lib.indigoRenderWriteHDC((void*)hdc, printing ? 1 : 0);
         _renderer_lib.indigoRender(obj.self, hdch);
      }

      public Bitmap renderToBitmap (IndigoObject obj)
      {
         _indigo.setSessionID();

         _indigo._indigo_lib.indigoSetOption("render-output-format", "png");
         byte[] res = renderToBuffer(obj);

         MemoryStream stream = new MemoryStream();
         stream.Write(res, 0, res.Length);
         stream.Seek(0, SeekOrigin.Begin);

         return (Bitmap)Image.FromStream(stream);
      }

      public Metafile renderToMetafile (IndigoObject obj)
      {
         _indigo.setSessionID();

         _indigo._indigo_lib.indigoSetOption("render-output-format", "emf");
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

         _renderer_lib.indigoRenderGrid(items.self, refatoms, ncolumns, bufh.self);
         return bufh.toBuffer();
      }

      public void renderGridToFile (IndigoObject items, int[] refatoms, int ncolumns, string filename)
      {
         if (refatoms != null)
            if (refatoms.Length != items.count())
               throw new IndigoException("renderGridToFile(): refatoms[] size must be equal to the number of objects");

         _renderer_lib.indigoRenderGridToFile(items.self, refatoms, ncolumns, filename);
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
   }
}
