using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Resources;
using System.IO;
using System.Reflection;
using System.Reflection.Emit;

namespace com.epam.indigo
{
    public class IndigoLibraryLoader
    {        
        class WindowsLoader
        {
            [DllImport("kernel32")]
            public static extern IntPtr LoadLibrary(string lpFileName);
            [DllImport("kernel32.dll")]
            public static extern int FreeLibrary(IntPtr module);
            [DllImport("kernel32.dll")]
            public static extern IntPtr GetProcAddress(IntPtr hModule, string procedureName);
            [DllImport("kernel32.dll")]
            public static extern int GetLastError();
        }

        class LinuxLoader
        {
            [DllImport("libdl.so.2")]
            public static extern IntPtr dlopen([MarshalAs(UnmanagedType.LPTStr)] string filename, int flags);
            [DllImport("libdl.so.2")]
            public static extern int dlclose(IntPtr handle);
            [DllImport("libdl.so.2")]
            public static extern IntPtr dlsym(IntPtr libraryPointer, string procedureName);
            [DllImport("libdl.so.2")]
            public static extern string dlerror();
        }


        class MacLoader
        {
            [DllImport("libdl.dylib")]
            public static extern IntPtr dlopen(string filename, int flags);
            [DllImport("libdl.dylib")]
            public static extern int dlclose(IntPtr handle);
            [DllImport("libdl.dylib")]
            public static extern IntPtr dlsym(IntPtr libraryPointer, string procedureName);
            [DllImport("libdl.dylib")]
            public static extern string dlerror();
        }

        public static IntPtr LoadLibrary(string filename)
        {
            switch (Environment.OSVersion.Platform)
            {
                case PlatformID.Win32NT:
                    return WindowsLoader.LoadLibrary(filename);
                case PlatformID.Unix:
                    if (isMac())
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
                    if (isMac())
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
                    if (isMac())
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
                    if (isMac())
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
            utsname uts = new utsname();
            uname(out uts);
            return uts.sysname.ToString();
        }

        static public bool isMac()
        {
            return (detectUnixKernel() == "Darwin");
        }


        string _getPathToBinary(String path, String filename, String resource_name, Assembly assembly)
        {
            if (path == null)
                return _extractFromAssembly(filename, resource_name, assembly);
            return Path.Combine(path, filename);
        }

        string _getTemporaryDirectory(Assembly resource_assembly)
        {
            String dir;
            dir = Path.Combine(Path.GetTempPath(), "GGA_indigo");
            dir = Path.Combine(dir, resource_assembly.GetName().Name);
            dir = Path.Combine(dir, resource_assembly.GetName().Version.ToString());
            return dir;
        }

        string _extractFromAssembly(String filename, String resource_name, Assembly resource_assembly)
        {
            ResourceManager manager = new ResourceManager(resource_name, resource_assembly);

            object file_data = manager.GetObject(filename);
            if (file_data == null)
                throw new IndigoException("Internal error: there is no resource " + filename);

            String tmpdir_path = _getTemporaryDirectory(resource_assembly);
            String version = resource_assembly.GetName().Version.ToString();
            // Make per-version-unique dependent dll name
            String path = Path.Combine(tmpdir_path, filename);
            String dir = Path.GetDirectoryName(path);
            String name = Path.GetFileName(path);

            String new_dll_name;
            new_dll_name = version + "_" + name;

            // This temporary file is used to avoid inter-process
            // race condition when concurrently stating many processes
            // on the same machine for the first time.
            String tmp_filename = Path.GetTempFileName();
            String new_full_path = Path.Combine(dir, new_dll_name);
            FileInfo file = new FileInfo(new_full_path);
            file.Directory.Create();
            // Check if file already exists
            if (!file.Exists || file.Length == 0) {
                File.WriteAllBytes(tmp_filename, (byte[]) file_data);
                // file is ready to be moved.. lets check again
                if (!file.Exists || file.Length == 0) {
                    File.Move(tmp_filename, file.FullName);
                } else {
                    File.Delete(tmp_filename);
                }
            }
            return file.FullName;
        }

        public void loadLibrary(String path, String dll_name, string resource_name, bool make_unique_dll_name)
        {
            lock (_sync_object)
            {
                String subprefix = null;

                switch (Environment.OSVersion.Platform)
                {
                    case PlatformID.Win32NT:
                        subprefix = (IntPtr.Size == 8) ? "Win/x64/" : "Win/x86/";
                        break;
                    case PlatformID.Unix:
                        if (isMac())
                        {
                            subprefix = "Mac/10.7/";
                        }
                        else
                        {
                            subprefix = (IntPtr.Size == 8) ? "Linux/x64/" : "Linux/x86/";
                        }
                        break;
                    default:
                        throw new PlatformNotSupportedException(String.Format(
                            "Unsupported platform: {0}",
                            Environment.OSVersion.Platform
                        )
                        );
                }

                data.file_name = _getPathToBinary(path, subprefix + dll_name,
                resource_name, Assembly.GetCallingAssembly(), make_unique_dll_name);

                data.file_name = data.file_name.Replace('/', '\\');

                data.handle = LibraryLoader.LoadLibrary(data.file_name);

                if (data.handle == IntPtr.Zero)
                    throw new Exception("Cannot load library " + dll_name +
                        " from the temporary file " + data.file_name.Replace('\\', '/') + ": " + LibraryLoader.GetLastError()
                    );

                _loaded_dlls.Add(dll_name, data);

                _dll_handles.Add(data);
            }
        }

        Object _sync_object = new Object();
    }
}
