using System;
using System.Text;
using System.Runtime.InteropServices;

namespace com.epam.indigo
{
    [Flags]
    public enum ReactingCenter
    {
        NOT_CENTER = -1,
        UNMARKED = 0,
        CENTER = 1,
        UNCHANGED = 2,
        MADE_OR_BROKEN = 4,
        ORDER_CHANGED = 8
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
    public unsafe class Indigo : IDisposable
    {
        public const int ABS = 1;
        public const int OR = 2;
        public const int AND = 3;
        public const int EITHER = 4;
        public const int UP = 5;
        public const int DOWN = 6;
        public const int CIS = 7;
        public const int TRANS = 8;
        public const int CHAIN = 9;
        public const int RING = 10;
        public const int ALLENE = 11;
        public const int SINGLET = 101;
        public const int DOUBLET = 102;
        public const int TRIPLET = 103;

        public const int RC_NOT_CENTER = -1;
        public const int RC_UNMARKED = 0;
        public const int RC_CENTER = 1;
        public const int RC_UNCHANGED = 2;
        public const int RC_MADE_OR_BROKEN = 4;
        public const int RC_ORDER_CHANGED = 8;

        public const int SG_TYPE_GEN = 0;
        public const int SG_TYPE_DAT = 1;
        public const int SG_TYPE_SUP = 2;
        public const int SG_TYPE_SRU = 3;
        public const int SG_TYPE_MUL = 4;
        public const int SG_TYPE_MON = 5;
        public const int SG_TYPE_MER = 6;
        public const int SG_TYPE_COP = 7;
        public const int SG_TYPE_CRO = 8;
        public const int SG_TYPE_MOD = 9;
        public const int SG_TYPE_GRA = 10;
        public const int SG_TYPE_COM = 11;
        public const int SG_TYPE_MIX = 12;
        public const int SG_TYPE_FOR = 13;
        public const int SG_TYPE_ANY = 14;

        public const uint MAX_SIZE = 1000000000;
      
        private long _sid = -1;
        private IndigoLib _indigoLib;

        ~Indigo()
        {
            Dispose();
        }

        public void Dispose()
        {
            if (_sid >= 0)
            {
                IndigoLib.indigoReleaseSessionId(_sid);
                _sid = -1;
            }
        }

        private static int strLen(sbyte* input)
        {
            int res = 0;
            do
            {
                if (input[res] == 0)
                {
                    break;
                }
                res++;
            } while (res < MAX_SIZE);
            return res;
        }

        private static string _sbyteToStringUTF8(sbyte* input) 
        {
            return new string(input, 0, strLen(input), Encoding.UTF8);
        }

        public float checkResult(float result)
        {
            if (result < 0)
            {
                throw new IndigoException(_sbyteToStringUTF8(IndigoLib.indigoGetLastError()));
            }

            return result;
        }

        public double checkResult(double result)
        {
            if (result < 0)
            {
                throw new IndigoException(_sbyteToStringUTF8(IndigoLib.indigoGetLastError()));
            }

            return result;
        }

        public int checkResult(int result)
        {
            if (result < 0)
            {
                throw new IndigoException(_sbyteToStringUTF8(IndigoLib.indigoGetLastError()));
            }

            return result;
        }

        public string checkResult(sbyte* result)
        {
            if (result == null)
            {
                throw new IndigoException(_sbyteToStringUTF8(IndigoLib.indigoGetLastError()));
            }

            return _sbyteToStringUTF8(result);
        }

        public void setSessionID()
        {
            IndigoLib.indigoSetSessionId(_sid);
        }

        public Indigo()
        {
            switch (Environment.OSVersion.Platform)
            {
                case PlatformID.Win32NT:
                    IndigoLibraryLoader.loadLibrary("com.epam.indigo.Properties.ResourcesWin", false);
                    break;
                case PlatformID.Unix:
                    if (IndigoLibraryLoader.isMac())
                    {
                        IndigoLibraryLoader.loadLibrary("com.epam.indigo.Properties.ResourcesMac", false);
                    }
                    else
                    {
                        IndigoLibraryLoader.loadLibrary("com.epam.indigo.Properties.ResourcesLinux", false);
                    }
                    break;
                default:
                    throw new PlatformNotSupportedException(String.Format("Unsupported platform: {0}", Environment.OSVersion.Platform));
            }
            _sid = IndigoLib.indigoAllocSessionId();
            IndigoLib.indigoSetSessionId(_sid);
        }

        public string version()
        {
            return checkResult(IndigoLib.indigoVersion());
        }
    }
}
