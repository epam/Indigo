using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Security;

namespace com.epam.indigo
{
    public unsafe class IndigoInchiLib
    {
        static IndigoInchiLib()
        {
            var assemblyFolder = Path.GetDirectoryName(new Uri(typeof(IndigoInchiLib).Assembly.CodeBase).LocalPath);

            if (System.Environment.OSVersion.Platform == System.PlatformID.Win32NT)
            {
                if (System.Environment.Is64BitProcess)
                {
                    IndigoNativeLibraryLoader.LoadLibrary("lib/Win/x64/indigo-inchi.dll", assemblyFolder);
                }
                else
                {
                    IndigoNativeLibraryLoader.LoadLibrary("lib/Win/x86/indigo-inchi.dll", assemblyFolder);
                }
            }
            else if (System.Environment.OSVersion.Platform == System.PlatformID.Win32NT)
            {
                if (IndigoNativeLibraryLoader.isMac())
                {
                    IndigoNativeLibraryLoader.LoadLibrary("lib/Mac/10.7/indigo-inchi.dylib", assemblyFolder);
                }
                else
                {
                    if (Environment.Is64BitProcess)
                    {
                        IndigoNativeLibraryLoader.LoadLibrary("lib/Linux/x64/indigo-inchi.so", assemblyFolder);
                    }
                    else
                    {
                        IndigoNativeLibraryLoader.LoadLibrary("lib/Linux/x86/indigo-inchi.so", assemblyFolder);
                    }
                }
            }
            else
            {
                throw new PlatformNotSupportedException();
            }
        }

        [DllImport("indigo-inchi"), SuppressUnmanagedCodeSecurity]
        public static extern sbyte* indigoInchiVersion();

        [DllImport("indigo-inchi"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoInchiResetOptions();

        [DllImport("indigo-inchi"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoInchiLoadMolecule(String inchi_string);

        [DllImport("indigo-inchi"), SuppressUnmanagedCodeSecurity]
        public static extern sbyte* indigoInchiGetInchi(int molecule);

        [DllImport("indigo-inchi"), SuppressUnmanagedCodeSecurity]
        public static extern sbyte* indigoInchiGetInchiKey(String inchi_string);

        [DllImport("indigo-inchi"), SuppressUnmanagedCodeSecurity]
        public static extern sbyte* indigoInchiGetWarning();

        [DllImport("indigo-inchi"), SuppressUnmanagedCodeSecurity]
        public static extern sbyte* indigoInchiGetLog();

        [DllImport("indigo-inchi"), SuppressUnmanagedCodeSecurity]
        public static extern sbyte* indigoInchiGetAuxInfo();
    }
}