using System;
using System.Runtime.InteropServices;
using System.Security;

namespace com.epam.indigo
{
    public unsafe class IndigoRendererLib
    {
        static IndigoRendererLib()
        {
            if (System.Environment.OSVersion.Platform == System.PlatformID.Win32NT)
            {
                if (System.Environment.Is64BitProcess)
                {
                    IndigoNativeLibraryLoader.LoadLibrary("lib/Win/x64/indigo-renderer.dll", true);
                }
                else
                {
                    IndigoNativeLibraryLoader.LoadLibrary("lib/Win/x86/indigo-renderer.dll", true);
                }
            }
            else if (System.Environment.OSVersion.Platform == System.PlatformID.Win32NT)
            {
                if (IndigoNativeLibraryLoader.isMac())
                {
                    IndigoNativeLibraryLoader.LoadLibrary("lib/Mac/10.7/indigo-renderer.dylib", true);
                }
                else
                {
                    if (Environment.Is64BitProcess)
                    {
                        IndigoNativeLibraryLoader.LoadLibrary("lib/Linux/x64/indigo-renderer.dylib", true);
                    }
                    else
                    {
                        IndigoNativeLibraryLoader.LoadLibrary("lib/Linux/x86/indigo-renderer.dylibo", true);
                    }
                }
            }
            else
            {
                throw new PlatformNotSupportedException();
            }
        }

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
    }
}
