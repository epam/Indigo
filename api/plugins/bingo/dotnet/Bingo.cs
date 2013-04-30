using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.IO;

namespace com.ggasoftware.indigo
{
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
    public unsafe class Bingo : IDisposable
    {
        private Indigo _indigo;
        private BingoLib _lib;
        private int _id;

        private Bingo(Indigo indigo, int id, BingoLib lib)
        {
            _indigo = indigo;
            _lib = lib;
            _id = id;
        }

        ~Bingo()
        {
            Dispose();
        }

        public void Dispose()
        {
            if (_id >= 0)
            {
                Bingo.checkResult(_indigo, _lib.bingoCloseDatabase(_id));
                _id = -1;
            }
        }

        public void close()
        {
            Dispose();
        }

        public static int checkResult(Indigo indigo, int result)
        {
            if (result < 0)
            {
                throw new BingoException(new String(indigo._indigo_lib.indigoGetLastError()));
            }

            return result;
        }

        private static BingoLib getLib(Indigo indigo)
        {
            String dllpath = indigo.getDllPath();
            string libraryName;
            IndigoDllLoader dll_loader = IndigoDllLoader.Instance;
            switch (Environment.OSVersion.Platform)
            {
                case PlatformID.Win32NT:
                    libraryName = "bingo.dll";
                    dll_loader.loadLibrary(dllpath, libraryName, "com.ggasoftware.indigo.Properties.ResourcesWin", false);
                    break;
                case PlatformID.Unix:
                    if (IndigoDllLoader.isMac())
                    {
                        libraryName = "libbingo.dylib";
                        dll_loader.loadLibrary(dllpath, libraryName, "com.ggasoftware.indigo.Properties.ResourcesMac", false);
                    }
                    else
                    {
                        libraryName = "libbingo.so";
                        dll_loader.loadLibrary(dllpath, libraryName, "com.ggasoftware.indigo.Properties.ResourcesLinux", false);
                    }
                    break;
                default:
                    throw new PlatformNotSupportedException(String.Format("Unsupported platform: {0}", Environment.OSVersion.Platform));
            }

            return dll_loader.getInterface<BingoLib>(libraryName);
        }

        public static Bingo createDatabaseFile(Indigo indigo, string location, string type, string options = null)
        {
            if (options == null)
            {
                options = "";
            }
            BingoLib lib = Bingo.getLib(indigo);
            int databaseID = Bingo.checkResult(indigo, lib.bingoCreateDatabaseFile(location, type, options));
            return new Bingo(indigo, databaseID, lib);
        }

        public static Bingo loadDatabaseFile(Indigo indigo, string location, string type, string options = null)
        {
            if (options == null)
            {
                options = "";
            }
            BingoLib lib = Bingo.getLib(indigo);
            int databaseID = Bingo.checkResult(indigo, lib.bingoLoadDatabaseFile(location, type, options));
            return new Bingo(indigo, databaseID, lib);
        }

        public int insert(IndigoObject record)
        {
            return Bingo.checkResult(_indigo, _lib.bingoInsertRecordObj(_id, record.self));
        }

        public int insert(IndigoObject record, int index)
        {
            return Bingo.checkResult(_indigo, _lib.bingoInsertRecordObjWithId(_id, record.self, index));
        }

        public void delete(int index)
        {
            Bingo.checkResult(_indigo, _lib.bingoDeleteRecord(_id, index));
        }

        public BingoObject searchSub(IndigoObject query, string options)
        {
            if (options == null)
            {
                options = "";
            }
            return new BingoObject(Bingo.checkResult(_indigo, _lib.bingoSearchSub(_id, query.self, options)), _indigo, _lib);
        }

        public BingoObject searchSim(IndigoObject query, float min, float max, string metric)
        {
            if (metric == null)
            {
                metric = "tanimoto";
            }
            return new BingoObject(Bingo.checkResult(_indigo, _lib.bingoSearchSim(_id, query.self, min, max, metric)), _indigo, _lib);
        }
    }
}
