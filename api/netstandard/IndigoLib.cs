using System.Runtime.InteropServices;

namespace com.epam.indigo
{
    public unsafe class IndigoLib
    {
        [DllImport("indigo.dll")]
        public static extern sbyte* indigoVersion();

        [DllImport("indigo.dll")]
        public static extern long indigoAllocSessionId();

        [DllImport("indigo.dll")]
        public static extern void indigoSetSessionId(long id);

        [DllImport("indigo.dll")]
        public static extern void indigoReleaseSessionId(long id);

        [DllImport("indigo.dll")]
        public static extern sbyte* indigoGetLastError();
    }
}
