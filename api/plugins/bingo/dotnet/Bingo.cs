using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.IO;

namespace com.epam.indigo
{
    /// <summary>
    /// Bingo instance corresponds to a single chemical database
    /// </summary>
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
    public unsafe class Bingo : IDisposable
    {
        private Indigo _indigo;
        private BingoLib _lib;
        private IndigoDllLoader _dllLoader;
        private int _id;

        private Bingo(Indigo indigo, int id, BingoLib lib)
        {
            _indigo = indigo;
            _lib = lib;
            _id = id;
            _dllLoader = IndigoDllLoader.Instance;
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
            if (_id >= 0 && _dllLoader != null && _dllLoader.isValid() && _indigo != null && _lib != null)
            {
                _indigo.setSessionID();
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

        internal static string checkResult(Indigo indigo, string result)
        {
            if (result == null)
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
                    dll_loader.loadLibrary(dllpath, libraryName, "com.epam.indigo.Properties.ResourcesWin", false);
                    break;
                case PlatformID.Unix:
                    if (IndigoDllLoader.isMac())
                    {
                        libraryName = "libbingo.dylib";
                        dll_loader.loadLibrary(dllpath, libraryName, "com.epam.indigo.Properties.ResourcesMac", false);
                    }
                    else
                    {
                        libraryName = "libbingo.so";
                        dll_loader.loadLibrary(dllpath, libraryName, "com.epam.indigo.Properties.ResourcesLinux", false);
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
            indigo.setSessionID();
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
        /// <param name="options">Additional options separated with a semicolon. See the Bingo documentation for more details.</param>
        /// <returns>Bingo database instance</returns>
        public static Bingo loadDatabaseFile(Indigo indigo, string location, string options)
        {
            if (options == null)
            {
                options = "";
            }
            BingoLib lib = Bingo.getLib(indigo);
            indigo.setSessionID();
            int databaseID = Bingo.checkResult(indigo, lib.bingoLoadDatabaseFile(location, options));
            return new Bingo(indigo, databaseID, lib);
        }

        /// <summary>
        /// Loads a chemical storage of a specifed type from a specified location
        /// </summary>
        /// <param name="indigo">Indigo instance</param>
        /// <param name="location">Directory with the files location</param>
        /// <returns>Bingo database instance</returns>
        public static Bingo loadDatabaseFile(Indigo indigo, string location)
        {
            return loadDatabaseFile(indigo, location, null);
        }

        /// <summary>
        /// Insert a structure into the database and returns id of this structure
        /// </summary>
        /// <param name="record">Indigo object with a chemical structure (molecule or reaction)</param>
        /// <returns>record id</returns>
        public int insert(IndigoObject record)
        {
           _indigo.setSessionID();
           return Bingo.checkResult(_indigo, _lib.bingoInsertRecordObj(_id, record.self));
        }

        /// <summary>
        /// Inserts a structure under a specified id
        /// </summary>
        /// <param name="record">Indigo object with a chemical structure (molecule or reaction)</param>
        /// <param name="id">record id</param>
        /// <returns> inserted record id</returns>
        public int insert(IndigoObject record, int id)
        {
           _indigo.setSessionID();
           return Bingo.checkResult(_indigo, _lib.bingoInsertRecordObjWithId(_id, record.self, id));
        }

        /// <summary>
        /// Insert a structure into the database and returns id of this structure
        /// </summary>
        /// <param name="record">Indigo object with a chemical structure (molecule or reaction)</param>
        /// <param name="ext_fp">Indigo object with a external similarity fingerprint</param>
        /// <returns>record id</returns>
        public int insertWithExtFP(IndigoObject record, IndigoObject ext_fp)
        {
           _indigo.setSessionID();
           return Bingo.checkResult(_indigo, _lib.bingoInsertRecordObjWithExtFP(_id, record.self, ext_fp.self));
        }

        /// <summary>
        /// Inserts a structure under a specified id
        /// </summary>
        /// <param name="record">Indigo object with a chemical structure (molecule or reaction)</param>
        /// <param name="ext_fp">Indigo object with a external similarity fingerprint</param>
        /// <param name="id">record id</param>
        /// <returns> inserted record id</returns>
        public int insertWithExtFP(IndigoObject record, IndigoObject ext_fp, int id)
        {
           _indigo.setSessionID();
           return Bingo.checkResult(_indigo, _lib.bingoInsertRecordObjWithIdAndExtFP(_id, record.self, id, ext_fp.self));
        }

        /// <summary>
        /// Delete a record by id
        /// </summary>
        /// <param name="id">Record id</param>
        public void delete(int id)
        {
           _indigo.setSessionID();
           Bingo.checkResult(_indigo, _lib.bingoDeleteRecord(_id, id));
        }

        /// <summary>
        /// Execute substructure search operation
        /// </summary>
        /// <param name="query">Indigo query object (molecule or reaction)</param>
        /// <param name="options">Search options</param>
        /// <returns>Bingo search object instance</returns>
        public BingoObject searchSub(IndigoObject query, string options)
        {
            if (options == null)
            {
                options = "";
            }
            _indigo.setSessionID();
            return new BingoObject(Bingo.checkResult(_indigo, _lib.bingoSearchSub(_id, query.self, options)), _indigo, _lib);
        }

        /// <summary>
        /// Execute substructure search operation
        /// </summary>
        /// <param name="query">Indigo query object (molecule or reaction)</param>
        /// <returns>Bingo search object instance</returns>
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
        /// <returns>Bingo search object instance</returns>
        public BingoObject searchSim(IndigoObject query, float min, float max, string metric)
        {
            if (metric == null)
            {
                metric = "tanimoto";
            }
            _indigo.setSessionID();
            return new BingoObject(Bingo.checkResult(_indigo, _lib.bingoSearchSim(_id, query.self, min, max, metric)), _indigo, _lib);
        }

        /// <summary>
        /// Execute similarity search operation
        /// </summary>
        /// <param name="query">indigo object (molecule or reaction)</param>
        /// <param name="min">Minimum similarity value</param>
        /// <param name="max">Maximum similairy value</param>
        /// <returns>Bingo search object instance</returns>
        public BingoObject searchSim(IndigoObject query, float min, float max)
        {
            return searchSim(query, min, max, null);
        }

        /// <summary>
        /// Execute similarity search operation
        /// </summary>
        /// <param name="query">indigo object (molecule or reaction)</param>
        /// <param name="min">Minimum similarity value</param>
        /// <param name="max">Maximum similairy value</param>
        /// <param name="ext_fp">Indigo object with a external similarity fingerprint</param>
        /// <param name="metric">Default value is "tanimoto"</param>
        /// <returns>Bingo search object instance</returns>
        public BingoObject searchSim(IndigoObject query, float min, float max, IndigoObject ext_fp, string metric)
        {
            if (metric == null)
            {
                metric = "tanimoto";
            }
            _indigo.setSessionID();
            return new BingoObject(Bingo.checkResult(_indigo, _lib.bingoSearchSimWithExtFP(_id, query.self, min, max, ext_fp.self, metric)), _indigo, _lib);
        }

        /// <summary>
        /// Execute enumerate id operation
        /// </summary>
        /// <returns>Bingo enumerate id object instance</returns>
        public BingoObject enumerateId()
        {
            _indigo.setSessionID();
            return new BingoObject(Bingo.checkResult(_indigo, _lib.bingoEnumerateId(_id)), _indigo, _lib);
        }

        /// <summary>
        /// Perform exact search operation
        /// </summary>
        /// <param name="query">indigo object (molecule or reaction)</param>
        /// <returns>Bingo search object instance</returns>
        public BingoObject searchExact(IndigoObject query)
        {
           return searchExact(query, null);
        }

        /// <summary>
        /// Perform exact search operation
        /// </summary>
        /// <param name="query">indigo object (molecule or reaction)</param>
        /// <param name="options">exact search options</param>
        /// <returns>Bingo search object instance</returns>
        public BingoObject searchExact(IndigoObject query, string options)
        {
           if (options == null)
           {
              options = "";
           }
           _indigo.setSessionID();
           return new BingoObject(Bingo.checkResult(_indigo, _lib.bingoSearchExact(_id, query.self, options)), _indigo, _lib);
        }

        /// <summary>
        /// Perform search by molecular formula
        /// </summary>
        /// <param name="query">string with formula to search. For example, 'C22 H23 N3 O2'.</param>
        /// <param name="options">search options</param>
        /// <returns>Bingo search object instance</returns>
        public BingoObject searchMolFormula(string query, string options)
        {
           if (options == null)
           {
              options = "";
           }
           _indigo.setSessionID();
           return new BingoObject(Bingo.checkResult(_indigo, _lib.bingoSearchMolFormula(_id, query, options)), _indigo, _lib);
        }

        /// <summary>
        /// Perform search by molecular formula
        /// </summary>
        /// <param name="query">string with formula to search. For example, 'C22 H23 N3 O2'.</param>
        /// <returns>Bingo search object instance</returns>
        public BingoObject searchMolFormula(string query)
        {
           return searchMolFormula(query, null);
        }

        /// <summary>
        /// Post-process index optimization
        /// </summary>
        public void optimize ()
        {
           _indigo.setSessionID();
           Bingo.checkResult(_indigo, _lib.bingoOptimize(_id));
        }

        /// <summary>
        /// Returns an IndigoObject for the record with the specified id
        /// </summary>
        /// <param name="id">record id</param>
        /// <returns>Indigo object</returns>
        public IndigoObject getRecordById(int id)
        {
           _indigo.setSessionID();
           return new IndigoObject(_indigo, Bingo.checkResult(_indigo, _lib.bingoGetRecordObj(_id, id))); ;
        }

        /// <summary>
        /// Returns Bingo version
        /// </summary>
        /// <returns>Bingo version</returns>
        public string version()
        {
            _indigo.setSessionID();
            return Bingo.checkResult(_indigo, _lib.bingoVersion());
        }
    }
}
