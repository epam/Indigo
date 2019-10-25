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
                // If there is no ibrary on filesystem
                return _extractFromAssembly(path, filename);
            }
            byte[] outputFilePathBytes = File.ReadAllBytes(outputFilePath);
            byte [] resourceBytes = getBinaryResource(string.Format("{0}.{1}", path, filename));
            if (!Compare(resourceBytes, outputFilePathBytes)) {
                // If library on filesystem differs from current (like old version)
                File.WriteAllBytes(outputFilePath, resourceBytes);
            }
            // If current version of library is already on filesystem
            return outputFilePath;
        }

        string _getPathToAssembly()
        {
            return Path.GetDirectoryName(Assembly.GetAssembly(typeof(IndigoNativeLibraryLoader)).Location);
        }

        private unsafe bool Compare(byte[] a, byte[] b)
        {
            if (a.Length != b.Length) return false;
            int len = a.Length;
            unsafe
            {
                fixed (byte* ap = a, bp = b)
                {
                    long* alp = (long*)ap, blp = (long*)bp;
                    for (; len >= 8; len -= 8)
                    {
                        if (*alp != *blp) return false;
                        alp++;
                        blp++;
                    }
                    byte* ap2 = (byte*)alp, bp2 = (byte*)blp;
                    for (; len > 0; len--)
                    {
                        if (*ap2 != *bp2) return false;
                        ap2++;
                        bp2++;
                    }
                }
            }
            return true;
        }

        private byte[] getBinaryResource(string resource)
        {
            Stream fs = Assembly.GetExecutingAssembly().GetManifestResourceStream(resource);
            if (fs == null)
                throw new IndigoException("Internal error: there is no resource " + resource);
            byte[] ba = new byte[fs.Length];
            fs.Read(ba, 0, ba.Length);
            fs.Close();
            if (ba == null)
                throw new IndigoException("Internal error: there is no resource " + resource);
            return ba;
        }

        string _extractFromAssembly(string inputPath, string filename)
        {
            byte[] ba = getBinaryResource(string.Format("{0}.{1}", inputPath, filename));

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
