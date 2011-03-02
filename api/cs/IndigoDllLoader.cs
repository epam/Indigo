using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.Resources;
using System.IO;
using System.Reflection;

namespace com.ggasoftware.indigo
{
   // Singleton DLL loader
   public class IndigoDllLoader
   {
      private static volatile IndigoDllLoader _instance;
      private static object _global_sync_root = new Object();
      private static volatile int _instance_id = 0;

      public static IndigoDllLoader Instance
      {
         get
         {
            if (_instance == null)
            {
               lock (_global_sync_root)
               {
                  if (_instance == null)
                     _instance = new IndigoDllLoader();
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

      struct DllInfo
      {
         public string name;
         public IntPtr handle;
         public int unload_count;
         public string file_name;
         public string lib_path;
      }

      // Mapping from the dll name to the handle
      List<DllInfo> _loaded_dlls = new List<DllInfo>();
      // Path for temporary directory
      String _temporary_dir = null;

      Object _sync_object = new Object();

      public void loadLibrary (String path, String dll_name, int unload_count, string resource_name)
      {
         lock (_sync_object)
         {
            foreach (DllInfo info in _loaded_dlls)
               if (info.name == dll_name)
               {
                  // Library has already been loaded
                  if (info.lib_path != path)
                     throw new IndigoException(
                        String.Format("Library {0} has already been loaded by different path {1}",
                        dll_name, info.lib_path));
                  return;
               }

            String subprefix = (IntPtr.Size == 8) ? "Win/x64/" : "Win/x86/";

            DllInfo dll_info = new DllInfo();
            dll_info.name = dll_name;
            dll_info.lib_path = path;
            dll_info.file_name = _getPathToBinary(path, subprefix + dll_name,
               resource_name, Assembly.GetCallingAssembly());
            dll_info.handle = LoadLibrary(dll_info.file_name);
            dll_info.unload_count = unload_count;
            if (dll_info.handle == IntPtr.Zero)
               throw new IndigoException("Cannot load library " + dll_name);
            _loaded_dlls.Add(dll_info);
         }
      }

      [DllImport("kernel32")]
      static extern IntPtr LoadLibrary(string lpFileName);
      [DllImport("kernel32.dll")]
      static extern int FreeLibrary(IntPtr module);

      ~IndigoDllLoader ()
      {
         lock (_global_sync_root)
         {
            _instance = null;
            _instance_id++;

            // Unload all loaded libraries in the reverse order
            _loaded_dlls.Reverse();
            foreach (DllInfo dll in _loaded_dlls)
            {
               // Libraries that were loaded with DllImport must be unloaded twice
               for (int i = 0; i < dll.unload_count; i++)
                  FreeLibrary(dll.handle);
            }
            _loaded_dlls.Clear();

            // Remove temporary directory. Exception will be thrown if some dll are still loaded
            if (_temporary_dir != null)
            {
               try
               {
                  Directory.Delete(_temporary_dir, true);
               }
               catch (IOException)
               {
               }
               _temporary_dir = null;
            }
         }
      }

      string _getPathToBinary (String path, String filename, String resource_name, Assembly resource_assembly)
      {
         if (path == null)
         {
            String result = _extractFromAssembly(filename, resource_name, resource_assembly);
            if (result != null)
               return result;
            path = "lib";
         }
         return Path.Combine(path, filename);
      }

      String _getTemporaryDirectory ()
      {
         if (_temporary_dir == null)
            _temporary_dir = Path.Combine(Path.GetTempPath(), "indigo_" + Path.GetRandomFileName());
         return _temporary_dir;
      }

      String _extractFromAssembly (String filename, String resource_name, Assembly resource_assembly)
      {
         try
         {
            ResourceManager manager = new ResourceManager(resource_name, resource_assembly);
            Object file_data = manager.GetObject(filename);

            String tmpdir_path = _getTemporaryDirectory();
            FileInfo file = new FileInfo(Path.Combine(tmpdir_path, filename));
            file.Directory.Create();
            File.WriteAllBytes(file.FullName, (byte[])file_data);
            return file.FullName;
         }
         catch
         {
            return null;
         }
      }
   }
}
