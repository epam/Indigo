using System;
using System.Runtime.InteropServices;
using System.IO;
using System.Reflection;

namespace com.epam.indigo
{
    // Singleton Native Library loader
    public class IndigoNativeLibraryLoader
    {
        private static volatile IndigoNativeLibraryLoader _instance;
        private static object _global_sync_root = new Object();

        public static IndigoNativeLibraryLoader Instance
        {
            get
            {
                if (_instance == null)
                {
                    lock (_global_sync_root)
                    {
                        if (_instance == null)
                            _instance = new IndigoNativeLibraryLoader();
                    }
                }

                return _instance;
            }
        }

        class DllData
        {
            public string file_name;
            public string lib_path;
        }

        // Mapping from the DLL name to the handle.
        // DLL handles in the loading order
        // Local synchronization object
        Object _sync_object = new Object();

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
        struct utsname
        {
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
            public string sysname;

            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
            public string nodename;

            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
            public string release;

            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
            public string version;

            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
            public string machine;

            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 1024)]
            public string extraJustInCase;

        }

        [DllImport("libc")]
        private static extern void uname(out utsname uname_struct);

        private static string detectUnixKernel()
        {
            utsname uts;
            uname(out uts);
            return uts.sysname.ToString();
        }

        static public bool isMac()
        {
            return (detectUnixKernel() == "Darwin");
        }

        public void loadLibrary(string path, string filename, bool load = false)
        {
            lock (_sync_object)
            {
                var data = new DllData();
                data.lib_path = path;
                data.file_name = _getPathToBinary(path, filename);
            }
        }

        ~IndigoNativeLibraryLoader()
        {
            lock (_global_sync_root)
            {
                _instance = null;
            }
        }

        public bool isValid()
        {
            return (_instance != null);
        }

        string _getPathToBinary(string path, string filename)
        {
            string outputFilePath = Path.Combine(_getPathToAssembly(), filename);
            if (!File.Exists(outputFilePath)) 
            {
                return _extractFromAssembly(path, filename);
            }
            return outputFilePath;
        }

        string _getPathToAssembly()
        {
            return Path.GetDirectoryName(Assembly.GetAssembly(typeof(IndigoNativeLibraryLoader)).Location);
        }

        string _extractFromAssembly(string inputPath, string filename)
        {
            string resource = string.Format("{0}.{1}", inputPath, filename);
            Stream fs = Assembly.GetExecutingAssembly().GetManifestResourceStream(resource);
            if (fs == null)
                throw new IndigoException("Internal error: there is no resource " + resource);
            byte[] ba = new byte[fs.Length];
            fs.Read(ba, 0, ba.Length);
            fs.Close();
            if (ba == null)
                throw new IndigoException("Internal error: there is no resource " + resource);

            string outputPath = Path.Combine(_getPathToAssembly(), filename);
            string dir = Path.GetDirectoryName(outputPath);
            string name = Path.GetFileName(outputPath);

            // This temporary file is used to avoid inter-process
            // race condition when concurrently stating many processes
            // on the same machine for the first time.
            string tmp_filename = Path.GetTempFileName();
            string new_full_path = Path.Combine(dir, name);
            FileInfo file = new FileInfo(new_full_path);
            if (!file.Directory.Exists)
                file.Directory.Create();
            // Check if file already exists
            if (!file.Exists || file.Length == 0) {
                File.WriteAllBytes(tmp_filename, ba);
                // file is ready to be moved.. lets check again
                if (!file.Exists || file.Length == 0) {
                    File.Move(tmp_filename, file.FullName);
                } else {
                    File.Delete(tmp_filename);
                }
            }
            return file.FullName;
        }
    }
}
