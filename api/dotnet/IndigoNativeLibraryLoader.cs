using System;
using System.Runtime.InteropServices;
using System.IO;
using System.Reflection;
using System.Collections.Generic;

namespace com.epam.indigo
{
    class LibraryLoader
    {
        class WindowsLoader
        {
            [DllImport("kernel32")]
            public static extern IntPtr LoadLibrary(string lpFileName);
            [DllImport("kernel32")]
            public static extern int FreeLibrary(IntPtr module);
            [DllImport("kernel32")]
            public static extern IntPtr GetProcAddress(IntPtr hModule, string procedureName);
            [DllImport("kernel32")]
            public static extern int GetLastError();
        }

        class LinuxLoader
        {
            [DllImport("dl")]
            public static extern IntPtr dlopen([MarshalAs(UnmanagedType.LPTStr)] string filename, int flags);
            [DllImport("dl")]
            public static extern int dlclose(IntPtr handle);
            [DllImport("dl")]
            public static extern IntPtr dlsym(IntPtr libraryPointer, string procedureName);
            [DllImport("dl")]
            public static extern string dlerror();
        }


        class MacLoader
        {
            [DllImport("dl")]
            public static extern IntPtr dlopen(string filename, int flags);
            [DllImport("dl")]
            public static extern int dlclose(IntPtr handle);
            [DllImport("dl")]
            public static extern IntPtr dlsym(IntPtr libraryPointer, string procedureName);
            [DllImport("dl")]
            public static extern string dlerror();
        }


        public static IntPtr LoadLibrary(string filename)
        {
            switch (Environment.OSVersion.Platform)
            {
                case PlatformID.Win32NT:
                    return WindowsLoader.LoadLibrary(filename);
                case PlatformID.Unix:
                    if (IndigoNativeLibraryLoader.isMac())
                    {
                        return MacLoader.dlopen(filename.Replace("\\", "/"), 0x8 | 0x1); // RTLD_GLOBAL | RTLD_NOW
                    }
                    else
                    {
                        return LinuxLoader.dlopen(filename.Replace("\\", "/"), 0x00100 | 0x00002); // RTLD_GLOBAL | RTLD_NOW
                    }
            }
            return IntPtr.Zero;
        }

        public static int FreeLibrary(IntPtr handle)
        {
            switch (Environment.OSVersion.Platform)
            {
                case PlatformID.Win32NT:
                    return WindowsLoader.FreeLibrary(handle);
                case PlatformID.Unix:
                    if (IndigoNativeLibraryLoader.isMac())
                    {
                        return MacLoader.dlclose(handle);
                    }
                    else
                    {
                        return LinuxLoader.dlclose(handle);
                    }
            }
            return 0;
        }

        public static IntPtr GetProcAddress(IntPtr library, string procedureName)
        {
            switch (Environment.OSVersion.Platform)
            {
                case PlatformID.Win32NT:
                    return WindowsLoader.GetProcAddress(library, procedureName);
                case PlatformID.Unix:
                    if (IndigoNativeLibraryLoader.isMac())
                    {
                        return MacLoader.dlsym(library, procedureName);
                    }
                    else
                    {
                        return LinuxLoader.dlsym(library, procedureName);
                    }
            }
            return IntPtr.Zero;
        }

        public static string GetLastError()
        {
            switch (Environment.OSVersion.Platform)
            {
                case PlatformID.Win32NT:
                    return WindowsLoader.GetLastError().ToString();
                case PlatformID.Unix:
                    if (IndigoNativeLibraryLoader.isMac())
                    {
                        return MacLoader.dlerror();
                    }
                    else
                    {
                        return LinuxLoader.dlerror();
                    }
            }
            return null;
        }
    }

    // Singleton Native Library loader
    public class IndigoNativeLibraryLoader
    {
        private static volatile IndigoNativeLibraryLoader _instance;
        private static object _global_sync_root = new Object();
        private static Dictionary<string, IntPtr> _loadedLibraries = new Dictionary<string, IntPtr>();

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
                // updateSystemPath(data);
                if (load)
                {
                    switch (Environment.OSVersion.Platform)
                    {
                        case PlatformID.Win32NT:
                            updateSystemPath(data);
                            break;
                        case PlatformID.Unix:
                            if (!_loadedLibraries.ContainsKey(data.file_name))
                            {
                                var handle = LibraryLoader.LoadLibrary(data.file_name);
                                if (handle == null)
                                    throw new IndigoException(string.Format("LoadLibrary error: null handle for library {0}", data.file_name));
                                _loadedLibraries[data.file_name] = handle;
                            }        
                            break;
                        throw new PlatformNotSupportedException(string.Format("Unsupported platform: {0}", Environment.OSVersion.Platform));
                    }
                }
            }
        }

        private static void updateSystemPath(DllData data)
        {
            string newEnvPath = Directory.GetParent(data.file_name).ToString();
            string envPathSep;
            string pathVariableName;
            switch (Environment.OSVersion.Platform)
            {
                case PlatformID.Win32NT:
                    envPathSep = ";";
                    pathVariableName = "PATH";
                    break;
                case PlatformID.Unix:
                    envPathSep = ":";
                    if (isMac())
                    {
                        pathVariableName = "DYLD_LIBRARY_PATH";
                    }
                    else
                    {
                        pathVariableName = "LD_LIBRARY_PATH";
                    }
                    break;
                default:
                    throw new PlatformNotSupportedException(string.Format("Unsupported platform: {0}", Environment.OSVersion.Platform));
            }
            var pathEnv = Environment.GetEnvironmentVariable(pathVariableName);
            if (pathEnv == null || !pathEnv.Contains(newEnvPath))
            {
                pathEnv = Directory.GetParent(data.file_name) + envPathSep + pathEnv;
                Environment.SetEnvironmentVariable(pathVariableName, pathEnv, EnvironmentVariableTarget.Process);
                Console.WriteLine(Environment.GetEnvironmentVariable(pathVariableName));
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
            return _extractFromAssembly(path, filename);
        }

        string _getTemporaryDirectory(Assembly resource_assembly)
        {
            string dir;
            dir = Path.Combine(Path.GetTempPath(), "EPAM_indigo");
            dir = Path.Combine(dir, resource_assembly.GetName().Name);
            dir = Path.Combine(dir, resource_assembly.GetName().Version.ToString());
            return dir;
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

            string tmpdir_path = _getTemporaryDirectory(Assembly.GetCallingAssembly());
            // Make per-version-unique dependent dll name
            string outputPath = Path.Combine(tmpdir_path, inputPath);
            outputPath = Path.Combine(outputPath, filename);
            string dir = Path.GetDirectoryName(outputPath);
            string name = Path.GetFileName(outputPath);

            string new_dll_name = name;

            // This temporary file is used to avoid inter-process
            // race condition when concurrently stating many processes
            // on the same machine for the first time.
            string tmp_filename = Path.GetTempFileName();
            string new_full_path = Path.Combine(dir, new_dll_name);
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
