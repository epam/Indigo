using System.Runtime.InteropServices;
using System.Security;

namespace com.epam.indigo
{
    public unsafe class IndigoRendererLib
    {
        [DllImport("indigo-renderer"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoRenderWriteHDC(void* hdc, int printingHdc);

        [DllImport("indigo-renderer"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoRender(int item, int output);

        [DllImport("indigo-renderer"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoRenderToFile(int item, string filename);

        [DllImport("indigo-renderer"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoRenderGrid(int items, int[] refAtoms, int nColumns, int output);

        [DllImport("indigo-renderer"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoRenderGridToFile(int items, int[] refAtoms, int nColumns, string filename);

        [DllImport("indigo-renderer"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoRenderReset();

        [DllImport("indigo-renderer"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoRendererInit(long sessionId);

        [DllImport("indigo-renderer"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoRendererDispose(long sessionId);
    }
}
