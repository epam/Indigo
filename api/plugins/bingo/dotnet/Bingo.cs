using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.IO;

namespace com.ggasoftware.indigo
{
    /// <summary>
    /// Bingo instance corresponds to a single chemical database
    /// </summary>
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

        /// <summary>
        /// Destructor
        /// </summary>
        ~Bingo()
        {
            Dispose();
        }

        /// <summary>
        /// Dispose method that closes the database
        /// </summary>
        public void Dispose()
        {
            if (_id >= 0)
            {
                Bingo.checkResult(_indigo, _lib.bingoCloseDatabase(_id));
                _id = -1;
            }
        }

        /// <summary>
        /// Method to close a database
        /// </summary>
        public void close()
        {
            Dispose();
        }

        internal static int checkResult(Indigo indigo, int result)
        {
            if (result < 0)
            {
                throw new BingoException(new String(indigo._indigo_lib.indigoGetLastError()));
            }

            return result;
        }

        internal static float checkResult(Indigo indigo, float result)
        {
            if (result < 0.0)
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

        /// <summary>
        /// Creates a chemical storage of a specifed type in a specified location
        /// </summary>
        /// <param name="indigo">Indigo instance</param>
        /// <param name="location">Directory with the files location</param>
        /// <param name="type">"molecule" or "reaction"</param>
        /// <param name="options">Additional options separated with a semicolon. See the Bingo documentation for more details.</param>
        /// <returns>Bingo database instance</returns>
        public static Bingo createDatabaseFile(Indigo indigo, string location, string type, string options)
        {
            if (options == null)
            {
                options = "";
            }
            BingoLib lib = Bingo.getLib(indigo);
            int databaseID = Bingo.checkResult(indigo, lib.bingoCreateDatabaseFile(location, type, options));
            return new Bingo(indigo, databaseID, lib);
        }

        /// <summary>
        /// Creates a chemical storage of a specifed type in a specified location
        /// </summary>
        /// <param name="indigo">Indigo instance</param>
        /// <param name="location">Directory with the files location</param>
        /// <param name="type">"molecule" or "reaction"</param>
        /// <returns>Bingo database instance</returns>
        public static Bingo createDatabaseFile(Indigo indigo, string location, string type)
        {
            return createDatabaseFile(indigo, location, type, null);
        }

        /// <summary>
        /// Loads a chemical storage of a specifed type from a specified location
        /// </summary>
        /// <param name="indigo">Indigo instance</param>
        /// <param name="location">Directory with the files location</param>
        /// <param name="type">"molecule" or "reaction"</param>
        /// <param name="options">Additional options separated with a semicolon. See the Bingo documentation for more details.</param>
        /// <returns>Bingo database instance</returns>
        public static Bingo loadDatabaseFile(Indigo indigo, string location, string type, string options)
        {
            if (options == null)
            {
                options = "";
            }
            BingoLib lib = Bingo.getLib(indigo);
            int databaseID = Bingo.checkResult(indigo, lib.bingoLoadDatabaseFile(location, type, options));
            return new Bingo(indigo, databaseID, lib);
        }

        /// <summary>
        /// Loads a chemical storage of a specifed type from a specified location
        /// </summary>
        /// <param name="indigo">Indigo instance</param>
        /// <param name="location">Directory with the files location</param>
        /// <param name="type">"molecule" or "reaction"</param>
        /// <returns>Bingo database instance</returns>
        public static Bingo loadDatabaseFile(Indigo indigo, string location, string type)
        {
            return loadDatabaseFile(indigo, location, type, null);
        }

        /// <summary>
        /// Insert a structure into the database and returns id of this structure
        /// </summary>
        /// <param name="record">Indigo object with a chemical structure (molecule or reaction)</param>
        /// <returns>record id</returns>
        public int insert(IndigoObject record)
        {
            return Bingo.checkResult(_indigo, _lib.bingoInsertRecordObj(_id, record.self));
        }

        /// <summary>
        /// Inserts a structure under a specified id
        /// </summary>
        /// <param name="record">Indigo object with a chemical structure (molecule or reaction)</param>
        /// <param name="id">record id</param>
        public void insert(IndigoObject record, int id)
        {
            Bingo.checkResult(_indigo, _lib.bingoInsertRecordObjWithId(_id, record.self, id));
        }

        /// <summary>
        /// Delete a record by id
        /// </summary>
        /// <param name="id">Record id</param>
        public void delete(int id)
        {
            Bingo.checkResult(_indigo, _lib.bingoDeleteRecord(_id, id));
        }

        /// <summary>
        /// Execute substructure search operation
        /// </summary>
        /// <param name="query">Indigo query object (molecule or reaction)</param>
        /// <param name="options">Search options</param>
        /// <returns>Bingo search object instanse</returns>
        public BingoObject searchSub(IndigoObject query, string options)
        {
            if (options == null)
            {
                options = "";
            }
            return new BingoObject(Bingo.checkResult(_indigo, _lib.bingoSearchSub(_id, query.self, options)), _indigo, _lib);
        }

        /// <summary>
        /// Execute substructure search operation
        /// </summary>
        /// <param name="query">Indigo query object (molecule or reaction)</param>
        /// <returns>Bingo search object instanse</returns>
        public BingoObject searchSub(IndigoObject query)
        {
            return searchSub(query, null);
        }

        /// <summary>
        /// Execute similarity search operation
        /// </summary>
        /// <param name="query">indigo object (molecule or reaction)</param>
        /// <param name="min">Minimum similarity value</param>
        /// <param name="max">Maximum similairy value</param>
        /// <param name="metric">Default value is "tanimoto"</param>
        /// <returns>Bingo search object instanse</returns>
        public BingoObject searchSim(IndigoObject query, float min, float max, string metric)
        {
            if (metric == null)
            {
                metric = "tanimoto";
            }
            return new BingoObject(Bingo.checkResult(_indigo, _lib.bingoSearchSim(_id, query.self, min, max, metric)), _indigo, _lib);
        }

        /// <summary>
        /// Execute similarity search operation
        /// </summary>
        /// <param name="query">indigo object (molecule or reaction)</param>
        /// <param name="min">Minimum similarity value</param>
        /// <param name="max">Maximum similairy value</param>
        /// <returns>Bingo search object instanse</returns>
        public BingoObject searchSim(IndigoObject query, float min, float max)
        {
            return searchSim(query, min, max, null);
        }

        /// <summary>
        /// Post-process index optimization
        /// </summary>
        public void optimize ()
        {
            Bingo.checkResult(_indigo, _lib.bingoOptimize(_id));
        }
    }
}
