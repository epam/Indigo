using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security;
using System.Text;

namespace com.epam.indigo
{
    public unsafe class IndigoInchiLib
    {
        static IndigoInchiLib()
        {
            if (System.Environment.OSVersion.Platform == System.PlatformID.Win32NT)
            {
                if (System.Environment.Is64BitProcess)
                {
                    IndigoNativeLibraryLoader.LoadLibrary("lib/Win/x64/indigo-inchi.dll", true);
                }
                else
                {
                    IndigoNativeLibraryLoader.LoadLibrary("lib/Win/x86/indigo-inchi.dll", true);
                }
            }
            else if (System.Environment.OSVersion.Platform == System.PlatformID.Win32NT)
            {
                if (IndigoNativeLibraryLoader.isMac())
                {
                    IndigoNativeLibraryLoader.LoadLibrary("lib/Mac/10.7/indigo-inchi.dylib", true);
                }
                else
                {
                    if (Environment.Is64BitProcess)
                    {
                        IndigoNativeLibraryLoader.LoadLibrary("lib/Linux/x64/indigo-inchi.dylib", true);
                    }
                    else
                    {
                        IndigoNativeLibraryLoader.LoadLibrary("lib/Linux/x86/indigo-inchi.dylibo", true);
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