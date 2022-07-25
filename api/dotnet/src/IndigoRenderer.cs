using System;
using System.Runtime.InteropServices;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;

namespace com.epam.indigo
{
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
    public unsafe class IndigoRenderer : IDisposable
    {
        private Indigo _indigo;
        private IndigoRendererLib IndigoRendererLib;
        private bool initialized;

        public IndigoRenderer(Indigo indigo)
        {
            _indigo = indigo;
             _indigo.setSessionID();
             // Preloads native library to register renderer options
            _indigo.checkResult(IndigoRendererLib.indigoRendererInit(_indigo.getSID()));
            initialized = true;
        }

        ~IndigoRenderer()
        {
            Dispose();
        }

        public void Dispose()
        {
            if (initialized)
            {
                _indigo.setSessionID();
                _indigo.checkResult(IndigoRendererLib.indigoRendererDispose());
                initialized = false;
            }
        }

        public void renderToFile(IndigoObject obj, string filename)
        {
            _indigo.setSessionID();
            _indigo.checkResult(IndigoRendererLib.indigoRenderToFile(obj.self, filename));
        }

        public byte[] renderToBuffer(IndigoObject obj)
        {
            _indigo.setSessionID();
            using (IndigoObject bufh = _indigo.writeBuffer())
            {
                _indigo.checkResult(IndigoRendererLib.indigoRender(obj.self, bufh.self));
                byte* buf;
                int bufsize;
                _indigo.checkResult(IndigoLib.indigoToBuffer(bufh.self, &buf, &bufsize));

                byte[] res = new byte[bufsize];
                for (int i = 0; i < bufsize; ++i)
                    res[i] = buf[i];
                return res;
            }
        }

        public void renderToHDC(IndigoObject obj, IntPtr hdc, bool printing)
        {
            _indigo.setSessionID();
            int hdch = _indigo.checkResult(IndigoRendererLib.indigoRenderWriteHDC((void*)hdc, printing ? 1 : 0));
            _indigo.checkResult(IndigoRendererLib.indigoRender(obj.self, hdch));
        }

        public Bitmap renderToBitmap(IndigoObject obj)
        {
            _indigo.setSessionID();
            _indigo.checkResult(IndigoLib.indigoSetOption("render-output-format", "png"));
            byte[] res = renderToBuffer(obj);

            MemoryStream stream = new MemoryStream();
            stream.Write(res, 0, res.Length);
            stream.Seek(0, SeekOrigin.Begin);

            return (Bitmap)Image.FromStream(stream);
        }

        public Metafile renderToMetafile(IndigoObject obj)
        {
            _indigo.setSessionID();
            _indigo.checkResult(IndigoLib.indigoSetOption("render-output-format", "emf"));
            byte[] res = renderToBuffer(obj);

            MemoryStream ms = new MemoryStream(res);
            Metafile mf = new Metafile(ms);

            return mf;
        }

        public byte[] renderGridToBuffer(IndigoObject items, int[] refatoms, int ncolumns)
        {
            using (IndigoObject bufh = _indigo.writeBuffer())
            {
                if (refatoms != null)
                    if (refatoms.Length != items.count())
                        throw new IndigoException("renderGridToFile(): refatoms[] size must be equal to the number of objects");

                _indigo.setSessionID();
                _indigo.checkResult(IndigoRendererLib.indigoRenderGrid(items.self, refatoms, ncolumns, bufh.self));
                return bufh.toBuffer();
            }
        }

        public void renderGridToFile(IndigoObject items, int[] refatoms, int ncolumns, string filename)
        {
            if (refatoms != null)
                if (refatoms.Length != items.count())
                    throw new IndigoException("renderGridToFile(): refatoms[] size must be equal to the number of objects");

            _indigo.setSessionID();
            _indigo.checkResult(IndigoRendererLib.indigoRenderGridToFile(items.self, refatoms, ncolumns, filename));
        }

        public void reset()
        {
            _indigo.setSessionID();
            _indigo.checkResult(IndigoRendererLib.indigoRenderReset());
        }

        public static void SaveMetafile(Metafile mf, Stream stream)
        {
            IntPtr henh = mf.GetHenhmetafile();
            int size = GetEnhMetaFileBits(henh, 0, null);

            byte[] buffer = new byte[size];

            if (GetEnhMetaFileBits(henh, size, buffer) <= 0)
                throw new SystemException("GetEnhMetaFileBits");

            stream.Write(buffer, 0, buffer.Length);
            stream.Flush();
        }

        public static void SaveMetafile(Metafile mf, String path)
        {
            FileStream fs = new FileStream(path, FileMode.OpenOrCreate);
            SaveMetafile(mf, fs);
            fs.Close();
        }

        [DllImport("gdi32")]
        private static extern int GetEnhMetaFileBits(IntPtr hemf, int cbBuffer, byte[] lpbBuffer);
    }
}
