using System;
using System.IO;
using System.Runtime.InteropServices;

namespace com.epam.indigo
{
    public class IndigoNativeLibraryLoader
    {
        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
        private struct utsname
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
            uname(out utsname uts);
            return uts.sysname.ToString();
        }

        public static bool isMac()
        {
            return (detectUnixKernel() == "Darwin");
        }

        public static IntPtr LoadLibrary(string inputPath, bool addAssemblyFolder = false)
        {
            var actualPath = inputPath;
            if (addAssemblyFolder)
            {
                var assemblyPath = new Uri(typeof(IndigoNativeLibraryLoader).Assembly.CodeBase).LocalPath;
                var assemblyFolder = Path.GetDirectoryName(assemblyPath);
                actualPath = Path.Combine(assemblyFolder, inputPath);
            }
#if DEBUG
            Console.WriteLine(string.Format("IndigoNativeLibraryLoader.LoadLibrary({0})", actualPath));
#endif
            IntPtr result;
            string errorMessage = "";

            if (System.Environment.OSVersion.Platform == System.PlatformID.Win32NT)
            {
                result = Windows.LoadLibrary(actualPath);
                if (result == IntPtr.Zero)
                {
                    errorMessage = Windows.GetLastError().ToString();
                }
            }
            else if (System.Environment.OSVersion.Platform == System.PlatformID.Unix)
            {
                if (isMac())
                {
                    result = MacOS.dlopen(actualPath, RTLD_GLOBAL + RTLD_LAZY);
                    if (result == IntPtr.Zero)
                    {
                        errorMessage = Marshal.PtrToStringAnsi(MacOS.dlerror());
                    }
                }
                else
                {
                    result = Linux.dlopen(actualPath, RTLD_GLOBAL + RTLD_LAZY);
                    if (result == IntPtr.Zero)
                    {
                        errorMessage = Marshal.PtrToStringAnsi(Linux.dlerror());
                    }
                }
            }
            else
            {
                var fullErrorMessage = "Not supported OS, only Windows, Linux and macOS are supported";
                System.Console.WriteLine(fullErrorMessage);
                throw new SystemException(fullErrorMessage);
            }

            if (result == IntPtr.Zero)
            {
                var fullErrorMessage = string.Format("Could not load library {0}: error {1}", inputPath, errorMessage);
                System.Console.WriteLine(fullErrorMessage);
                throw new SystemException(fullErrorMessage);
            }

            return result;
        }

        private const int RTLD_LAZY = 1;
        private const int RTLD_GLOBAL = 8;

        private static class Windows
        {
            [DllImport("kernel32.dll")]
            internal static extern IntPtr LoadLibrary(string filename);

            [DllImport("kernel32.dll")]
            internal static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

            [DllImport("kernel32.dll")]
            internal static extern int GetLastError();
        }

        private static class Linux
        {
            [DllImport("libdl.so")]
            internal static extern IntPtr dlopen(string filename, int flags);

            [DllImport("libdl.so")]
            internal static extern IntPtr dlerror();

            [DllImport("libdl.so")]
            internal static extern IntPtr dlsym(IntPtr handle, string symbol);
        }

        private static class MacOS
        {
            [DllImport("libSystem.dylib")]
            internal static extern IntPtr dlopen(string filename, int flags);

            [DllImport("libSystem.dylib")]
            internal static extern IntPtr dlerror();

            [DllImport("libSystem.dylib")]
            internal static extern IntPtr dlsym(IntPtr handle, string symbol);
        }
    }
}
