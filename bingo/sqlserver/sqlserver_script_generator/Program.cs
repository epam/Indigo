using System;
using System.Collections.Generic;
using System.Reflection;
using System.Linq;
using System.IO;
using Microsoft.SqlServer.Server;
using indigo.SqlAttributes;

namespace indigo
{
   class SqlServerScriptGenerator
   {
      static IEnumerable<MethodInfo> GetMethodsWithSqlAttributes(Assembly assembly)
      {
         List<MethodInfo> methods = new List<MethodInfo>();
         foreach (Type type in assembly.GetTypes())
         {
            foreach (MethodInfo method in type.GetMethods())
            {
               object[] attributes = method.GetCustomAttributes(typeof(BingoSqlFunctionAttribute), true);
               if (attributes.Length > 0)
                  methods.Add(method);
            }
         }
         methods.Sort((i1, i2) => i1.Name.CompareTo(i2.Name));
         return methods;
      }

      static Dictionary<string, string> types_substitution = new Dictionary<string, string>
         {
            { "Int32", "int" },
            { "SqlInt32", "int" },
            { "Int64", "bigint" },
            { "SqlInt64", "bigint" },
            { "SqlBoolean", "bit" },
            { "SqlByte", "tinyint" },
            { "String", "nvarchar(max)" },
            { "SqlString", "nvarchar(max)" },
            { "Double", "float" },
            { "SqlBinary", "varbinary(max)" },
            { "SqlDouble", "float" },
            { "SqlSingle", "real" },
            { "Single", "real" },
         };

      static string SqlType(string ctype, MethodInfo m, bool is_proxy)
      {
         if (ctype == "IEnumerable")
         {
            object[] attributes = m.GetCustomAttributes(typeof(SqlFunctionAttribute), true);
            SqlFunctionAttribute sql_attr = (SqlFunctionAttribute)(attributes[0]);
            if (!is_proxy)
               return String.Format("TABLE ({0})", sql_attr.TableDefinition);
            else
               return "TABLE";
         }
         if (types_substitution.ContainsKey(ctype))
            return types_substitution[ctype];
         System.Console.WriteLine("Please, add type {0} to the type substitution table", ctype);
         throw new Exception("Type " + ctype + " wasn't found in the substitution table");
      }

      static void PrintParametersAndReturn(StreamWriter stream, MethodInfo m, bool is_proxy)
      {
         // Write aruments and return value
         bool if_first = true;
         if (m.ReturnType != typeof(void))
            stream.Write("  (\n");
         foreach (ParameterInfo p in m.GetParameters())
         {
            if (is_proxy && p.Name == "bingo_schema")
               continue;

            if (!if_first)
            {
               stream.Write(",");
               stream.Write("\n");
            }
            else
            {
               if_first = false;
            }
            stream.Write("    @{0} {1}",
               p.Name, SqlType(p.ParameterType.Name, m, is_proxy));
         }
         if (!if_first)
            stream.Write("\n");
         if (m.ReturnType != typeof(void))
            stream.Write("  )\n");

         if (m.ReturnType != typeof(void))
            stream.Write("  RETURNS {0}\n", SqlType(m.ReturnType.Name, m, is_proxy));

         stream.Write("AS\n");
      }

      static void PrintFunctionName(StreamWriter stream, MethodInfo m, bool is_hidden)
      {
         bool is_function = (m.ReturnType != typeof(void));
         stream.Write("CREATE ");
         if (is_function)
            stream.Write("FUNCTION ");
         else
            stream.Write("PROCEDURE ");

         stream.Write("[$(bingo)].{0}{1} \n", is_hidden ? "_" : "", m.Name);
      }

      static void PrintDrop(StreamWriter stream, MethodInfo m, bool proxy)
      {
         bool is_function = (m.ReturnType != typeof(void));
         stream.Write("DROP ");
         if (is_function)
            stream.Write("FUNCTION ");
         else
            stream.Write("PROCEDURE ");

         stream.Write("[$(bingo)].{0}{1} \n", proxy ? "_" : "", m.Name);
         stream.Write("GO\n\n");
      }

      static void PrintProxyFunctionBody(StreamWriter stream, MethodInfo m,
         BingoSqlFunctionAttribute attr)
      {
         if (m.ReturnType.Name != "IEnumerable")
            stream.Write("BEGIN\n");

         if (m.ReturnType != typeof(void))
            stream.Write("  RETURN ");
         else
            stream.Write("  EXEC ");

         if (m.ReturnType.Name == "IEnumerable")
            stream.Write("(SELECT * FROM ");

         stream.Write("[$(bingo)]._{0} ", m.Name);
         if (m.ReturnType != typeof(void))
            stream.Write("(");
         bool if_first = true;
         foreach (ParameterInfo p in m.GetParameters())
         {
            if (!if_first)
               stream.Write(", ");
            else
               if_first = false;
            if (attr.substitute_schema && p.Name == "bingo_schema")
               stream.Write("'$(bingo)'");
            else
               stream.Write("@{0}", p.Name);
         }
         if (m.ReturnType != typeof(void))
            stream.Write(")");
         if (m.ReturnType.Name == "IEnumerable")
            stream.Write(")");

         stream.Write("\n");
         if (m.ReturnType.Name != "IEnumerable")
            stream.Write("END\n");
      }

      static void SignFunction(StreamWriter stream, MethodInfo m, bool proxy)
      {
         stream.Write("ADD SIGNATURE TO [$(bingo)].{0}{1} BY CERTIFICATE $(bingo)_certificate\n",
            proxy ? "_" : "", m.Name);
         stream.Write("  WITH PASSWORD = '$(bingo_pass)'\n");
         stream.Write("GO\n");
      }

      private static void AppendCreateMethods(IEnumerable<MethodInfo> methods, StreamWriter create_file)
      {
         foreach (MethodInfo m in methods)
         {
            object[] attributes = m.GetCustomAttributes(typeof(BingoSqlFunctionAttribute), true);
            BingoSqlFunctionAttribute attr = (BingoSqlFunctionAttribute)(attributes[0]);

            create_file.Write("--\n-- {0}\n--\n", m.Name);

            bool need_proxy_function = false;
            if (attr.substitute_schema)
               need_proxy_function |= (m.GetParameters().Count(p => p.Name == "bingo_schema") > 0);
            PrintFunctionName(create_file, m, need_proxy_function);
            PrintParametersAndReturn(create_file, m, false);

            create_file.Write("  EXTERNAL NAME [$(bingo)_assembly].[{0}].{1}\n",
               m.ReflectedType.FullName, m.Name);

            create_file.WriteLine("GO");
            SignFunction(create_file, m, need_proxy_function);
            create_file.WriteLine("");

            if (need_proxy_function)
            {
               // Create proxy function
               PrintFunctionName(create_file, m, false);
               PrintParametersAndReturn(create_file, m, true);
               PrintProxyFunctionBody(create_file, m, attr);
               create_file.WriteLine("GO");

               if (m.ReturnType.Name != "IEnumerable")
                  // Inline table functions cannot be signed
                  SignFunction(create_file, m, false);

               create_file.WriteLine("");
            }

            // Grant access to the created function
            if (attr.access_level != AccessLevelKind.None)
            {
               string permissions = "execute";
               if (m.ReturnType.Name == "IEnumerable")
                  permissions = "select";

               create_file.Write("grant {0} on [$(bingo)].{1} to $(bingo)_", permissions, m.Name);

               if (attr.access_level == AccessLevelKind.Reader)
                  create_file.Write("reader");
               else
                  create_file.Write("operator");

               create_file.WriteLine("\nGO\n", m.Name);
            }
         }
      }

      private static void AppendDropMethods(IEnumerable<MethodInfo> methods, StreamWriter drop_file)
      {
         foreach (MethodInfo m in methods)
         {
            object[] attributes = m.GetCustomAttributes(typeof(BingoSqlFunctionAttribute), true);
            BingoSqlFunctionAttribute attr = (BingoSqlFunctionAttribute)(attributes[0]);

            bool need_proxy_function = false;
            if (attr.substitute_schema)
               need_proxy_function |= (m.GetParameters().Count(p => p.Name == "bingo_schema") > 0);

            PrintDrop(drop_file, m, false);
            if (need_proxy_function)
               PrintDrop(drop_file, m, true);
         }
      }


      static void Main(string[] args)
      {
         string path = ".";
         if (args.Length == 0)
            System.Console.WriteLine("Usage: path to directory with scripts. Using current directory");
         else
            path = args[0];

         Assembly a = indigo.SqlAttributes.AccessLevelKind.None.GetType().Assembly;

         FileInfo create_script_file_info = new FileInfo(path + "\\bingo_create_methods.sql");
         FileInfo drop_script_file_info = new FileInfo(path + "\\bingo_drop_methods.sql");
         StreamWriter drop_file = drop_script_file_info.CreateText();
         StreamWriter create_file = create_script_file_info.CreateText();

         create_file.WriteLine("--\n-- This file was generated automatically --\n--\n");
         drop_file.WriteLine("--\n-- This file was generated automatically --\n--\n");

         create_file.WriteLine("use $(database)\ngo\n");
         drop_file.WriteLine("use $(database)\ngo\n");

         AppendCreateMethods(GetMethodsWithSqlAttributes(a), create_file);
         AppendDropMethods(GetMethodsWithSqlAttributes(a), drop_file);

         drop_file.Close();
         create_file.Close();
      }

   }
}
