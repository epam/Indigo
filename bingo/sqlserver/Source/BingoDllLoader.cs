using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.Resources;
using System.IO;
using System.Reflection;
using System.Reflection.Emit;
using System.Diagnostics;

namespace indigo
{
    class LibraryLoader
    {

        class WindowsLoader
        {
            [DllImport("kernel32.dll")]
            public static extern IntPtr LoadLibrary(string lpFileName);
            [DllImport("kernel32.dll")]
            public static extern int FreeLibrary(IntPtr module);
            [DllImport("kernel32.dll")]
            public static extern IntPtr GetProcAddress(IntPtr hModule, string procedureName);
            [DllImport("kernel32.dll")]
            public static extern int GetLastError();
        }

        public static IntPtr LoadLibrary(string filename)
        {
            return WindowsLoader.LoadLibrary(filename);
        }

        public static int FreeLibrary(IntPtr handle)
        {
            return WindowsLoader.FreeLibrary(handle);
        }

        public static IntPtr GetProcAddress(IntPtr library, string procedureName)
        {
            return WindowsLoader.GetProcAddress(library, procedureName);
        }

        public static string GetLastError()
        {
            return WindowsLoader.GetLastError().ToString();
        }
    }


    // Singleton DLL loader
    public class BingoDllLoader
    {
        private static volatile BingoDllLoader _instance;
        private static object _global_sync_root = new Object();
        private static volatile int _instance_id = 0;

        public static BingoDllLoader Instance
        {
            get
            {
                if (_instance == null)
                {
                    lock (_global_sync_root)
                    {
                        if (_instance == null)
                            _instance = new BingoDllLoader();
                    }
                }

                return _instance;
            }
        }

        // Returns Id of the instance. When DllLoader is being finalized this value gets increased.
        public static int InstanceId
        {
            get
            {
                return _instance_id;
            }
        }

        class WrappedInterface
        {
            // Dictionary with delegates for calling unmanaged functions
            public Dictionary<string, Delegate> delegates = new Dictionary<string, Delegate>();
            // Interface instance with wrappers for calling unmanaged functions
            public object instance = null;
        }

        class DllData
        {
            public IntPtr handle;
            public string file_name;
            public string lib_path;

            public Dictionary<Type, WrappedInterface> interfaces = new Dictionary<Type, WrappedInterface>();
        }

        // Mapping from the DLL name to the handle.
        Dictionary<string, DllData> _loaded_dlls = new Dictionary<string, DllData>();
        // DLL handles in the loading order
        List<DllData> _dll_handles = new List<DllData>();
        // Local synchronization object
        Object _sync_object = new Object();

        public void loadLibrary(String path, String dll_name, string resource_name, bool make_unique_dll_name)
        {
            lock (_sync_object)
            {
                DllData data = null;
                if (_loaded_dlls.TryGetValue(dll_name, out data))
                {
                    // Library has already been loaded
                    if (data.lib_path != path)
                        throw new Exception(String.Format("Library {0} has already been loaded by different path {1}", dll_name, data.lib_path));
                    return;
                }

                data = new DllData();
                data.lib_path = path;
                data.file_name = _getPathToBinary(path, dll_name, resource_name, Assembly.GetCallingAssembly(), make_unique_dll_name);

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

        ~BingoDllLoader()
        {
            lock (_global_sync_root)
            {
                _instance = null;
                _instance_id++;

                // Unload all loaded libraries in the reverse order
                _dll_handles.Reverse();
                foreach (DllData dll in _dll_handles)
                    LibraryLoader.FreeLibrary(dll.handle);
            }
        }

        public bool isValid()
        {
            return (_instance != null);
        }

        string _getPathToBinary(String path, String filename, String resource_name,
           Assembly resource_assembly, bool make_unique_dll_name)
        {
            if (path == null)
                return _extractFromAssembly(filename, resource_name, resource_assembly, make_unique_dll_name);
            return Path.Combine(path, filename);
        }

        String _getTemporaryDirectory(Assembly resource_assembly)
        {
            String dir;
            dir = Path.Combine(Path.GetTempPath(), "EPAM_bingo");
            dir = Path.Combine(dir, resource_assembly.GetName().Name);
            dir = Path.Combine(dir, resource_assembly.GetName().Version.ToString());
            return dir;
        }

        String _extractFromAssembly(String filename, String resource_name,
           Assembly resource_assembly, bool make_unique_dll_name)
        {
            ResourceManager manager = new ResourceManager(resource_name, resource_assembly);

            String output = String.Join(" ", resource_assembly.GetManifestResourceNames());

            Object file_data = manager.GetObject(filename);
            if (file_data == null)
                throw new Exception("Internal error: there is no resource " + filename + " in resource " + resource_name + ". All resources: " + output);

            String tmpdir_path = _getTemporaryDirectory(resource_assembly);
            String version = resource_assembly.GetName().Version.ToString();
            // Make per-version-unique dependent dll name
            String path = Path.Combine(tmpdir_path, filename);
            String dir = Path.GetDirectoryName(path);
            String name = Path.GetFileName(path);

            String new_dll_name;
            if (make_unique_dll_name)
                new_dll_name = version + "_" + name;
            else
                new_dll_name = name;

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

        // Returns implementation of a given interface for wrapping function the specified DLL
        public IT getInterface<IT>(string dll_name) where IT : class
        {
            lock (_sync_object)
            {
                Type itype = typeof(IT);
                // Check if such interface was already loaded
                WrappedInterface interf = null;
                if (_loaded_dlls.ContainsKey(dll_name))
                {
                    if (!_loaded_dlls[dll_name].interfaces.TryGetValue(itype, out interf))
                    {
                        interf = createInterface<IT>(dll_name);
                        _loaded_dlls[dll_name].interfaces.Add(itype, interf);
                    }
                }
                else
                {
                    interf = createInterface<IT>(dll_name);
                    _loaded_dlls[dll_name].interfaces.Add(itype, interf);
                }

                return (IT)interf.instance;
            }
        }

        string getDelegateField(MethodInfo m)
        {
            return m.Name + "_ptr";
        }

        Type createDelegateType(string delegate_type_name, ModuleBuilder mb, Type ret_type, Type[] arg_types)
        {
            // Create delegate
            TypeBuilder delegate_type = mb.DefineType(delegate_type_name,
               TypeAttributes.Class | TypeAttributes.Public | TypeAttributes.Sealed |
               TypeAttributes.AnsiClass | TypeAttributes.AutoClass,
               typeof(System.MulticastDelegate));

            ConstructorBuilder constructorBuilder =
               delegate_type.DefineConstructor(MethodAttributes.RTSpecialName |
               MethodAttributes.HideBySig | MethodAttributes.Public,
               CallingConventions.Standard,
               new Type[] { typeof(object), typeof(System.IntPtr) });
            constructorBuilder.SetImplementationFlags(MethodImplAttributes.Runtime | MethodImplAttributes.Managed);
            MethodBuilder methodBuilder = delegate_type.DefineMethod("Invoke",
               MethodAttributes.Public | MethodAttributes.HideBySig |
               MethodAttributes.NewSlot | MethodAttributes.Virtual,
               ret_type, arg_types);
            methodBuilder.SetImplementationFlags(MethodImplAttributes.Runtime | MethodImplAttributes.Managed);

            // Add [UnmanagedFunctionPointer(CallingConvention.Cdecl)] attribute for the delegate
            ConstructorInfo func_pointer_constructor =
               typeof(UnmanagedFunctionPointerAttribute).GetConstructor(new Type[] { typeof(CallingConvention) });
            CustomAttributeBuilder ca_builder =
               new CustomAttributeBuilder(func_pointer_constructor, new object[] { CallingConvention.Cdecl });
            delegate_type.SetCustomAttribute(ca_builder);

            return delegate_type.CreateType();
        }

        private class TypeListComparer : IEqualityComparer<List<Type>>
        {
            public bool Equals(List<Type> x, List<Type> y)
            {
                if (x.Count != y.Count)
                    return false;
                for (int i = 0; i < x.Count; i++)
                    if (x[i] != y[i])
                        return false;
                return true;
            }
            public int GetHashCode(List<Type> obj)
            {
                int hash = 0;
                foreach (Type t in obj)
                    hash ^= t.GetHashCode();
                return hash;
            }
        }


        // Creates implementation of a given interface for wrapping function the specified DLL
        WrappedInterface createInterface<IT>(string dll_name) where IT : class
        {
            WrappedInterface result = new WrappedInterface();

            Type itype = typeof(IT);
            AppDomain cd = System.Threading.Thread.GetDomain();
            AssemblyName an = new AssemblyName();
            an.Name = itype.Name + "_" + dll_name.Replace('.', '_');
            AssemblyBuilder ab = cd.DefineDynamicAssembly(an, AssemblyBuilderAccess.Run);
            ModuleBuilder mb = ab.DefineDynamicModule(an.Name, false);
            TypeBuilder tb = mb.DefineType(an.Name, TypeAttributes.Class |
               TypeAttributes.Public);
            tb.AddInterfaceImplementation(itype);

            IntPtr dll_handle = _loaded_dlls[dll_name].handle;

            Dictionary<List<Type>, Type> signature_to_name =
               new Dictionary<List<Type>, Type>(new TypeListComparer());

            // Set delegate references
            foreach (MethodInfo m in itype.GetMethods())
            {
                ParameterInfo[] parameters = m.GetParameters();
                Type[] arg_types = new Type[parameters.Length];
                for (int i = 0; i < parameters.Length; i++)
                    arg_types[i] = parameters[i].ParameterType;

                Type delegate_ret_type = m.ReturnType;
                if (delegate_ret_type == typeof(String))
                    delegate_ret_type = typeof(sbyte*);

                List<Type> signature = new List<Type>();
                signature.Add(delegate_ret_type);
                signature.AddRange(arg_types);

                Type call_delegate = null;
                if (!signature_to_name.TryGetValue(signature, out call_delegate))
                {
                    // Check if type was already created
                    string delegate_type_name = String.Format("delegate_{0}", signature_to_name.Count);
                    call_delegate = createDelegateType(delegate_type_name, mb, delegate_ret_type, arg_types);
                    signature_to_name.Add(signature, call_delegate);
                }

                string delegate_field_name = m.Name + "_ptr";
                FieldBuilder delegate_field =
                   tb.DefineField(delegate_field_name, typeof(Delegate), FieldAttributes.Private);

                IntPtr proc = LibraryLoader.GetProcAddress(dll_handle, m.Name);
                if (proc == IntPtr.Zero)
                    throw new Exception(String.Format("Cannot find procedure {0} in the library {1}", m.Name, dll_name));
                Delegate proc_delegate = Marshal.GetDelegateForFunctionPointer(proc, call_delegate);
                result.delegates.Add(delegate_field_name, proc_delegate);

                MethodBuilder meth = tb.DefineMethod(m.Name,
                   MethodAttributes.Public | MethodAttributes.Virtual, m.ReturnType, arg_types);

                ILGenerator il = meth.GetILGenerator();
                il.Emit(OpCodes.Ldarg_0);
                il.Emit(OpCodes.Ldfld, delegate_field);
                for (int i = 1; i < arg_types.Length + 1; i++)
                    il.Emit(OpCodes.Ldarg, i);
                MethodInfo infoMethod = proc_delegate.GetType().GetMethod("Invoke", arg_types);
                il.EmitCall(OpCodes.Callvirt, infoMethod, null);
                // Automatically convert sbyte* to String
                if (m.ReturnType == typeof(String))
                {
                    Type str_type = typeof(String);
                    ConstructorInfo ci = str_type.GetConstructor(new Type[] { typeof(sbyte*) });
                    il.Emit(OpCodes.Newobj, ci);
                }
                il.Emit(OpCodes.Ret);

                tb.DefineMethodOverride(meth, m);
            }

            // ab.Save(an.Name + ".dll");

            Type impl_class = tb.CreateType();
            IT impl = (IT)Activator.CreateInstance(impl_class);
            // Set references to the delegates
            foreach (string field_name in result.delegates.Keys)
            {
                impl_class.GetField(field_name, BindingFlags.Instance | BindingFlags.NonPublic)
                   .SetValue(impl, result.delegates[field_name]);
            }

            result.instance = impl;
            return result;
        }
    }
}
