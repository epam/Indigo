using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Security;

namespace com.epam.indigo
{
    public unsafe class IndigoInchiLib
    {
        [DllImport("indigo-inchi"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoInchiVersion();

        [DllImport("indigo-inchi"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoInchiResetOptions();

        [DllImport("indigo-inchi"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoInchiLoadMolecule([MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))] string inchi_string);

        [DllImport("indigo-inchi"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoInchiGetInchi(int molecule);

        [DllImport("indigo-inchi"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoInchiGetInchiKey([MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))] string inchi_string);

        [DllImport("indigo-inchi"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoInchiGetWarning();

        [DllImport("indigo-inchi"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoInchiGetLog();

        [DllImport("indigo-inchi"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoInchiGetAuxInfo();

        [DllImport("indigo-inchi"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoInchiInit(long sessionId);

        [DllImport("indigo-inchi"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoInchiDispose(long sessionId);
    }
}