using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Security;

namespace com.epam.indigo
{
    public unsafe class IndigoInchiLib
    {
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