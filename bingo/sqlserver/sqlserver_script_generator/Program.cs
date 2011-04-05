using System;
using System.Text;
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
            { "Void", "" },
            { "Int32", "int" },
            { "SqlInt32", "int" },
            { "Int64", "bigint" },
            { "SqlInt64", "bigint" },
            { "SqlBoolean", "bit" },
            { "SqlByte", "tinyint" },
            { "SqlString", "nvarchar(max)" },
            { "SqlBinary", "varbinary(max)" },
            { "SqlDouble", "float" },
            { "SqlSingle", "real" },
            { "Single", "real" },
         };

      static string SqlType (string ctype, MethodInfo m)
      {
         if (ctype == "IEnumerable")
         {
            object[] attributes = m.GetCustomAttributes(typeof(SqlFunctionAttribute), true);
            SqlFunctionAttribute sql_attr = (SqlFunctionAttribute)(attributes[0]);
            return String.Format("TABLE ({0})", sql_attr.TableDefinition);
         }

         if (types_substitution.ContainsKey(ctype))
            return types_substitution[ctype];
         String msg = String.Format("Please, add type {0} to the type substitution table for {1} method",
            ctype, m.Name);
         System.Console.WriteLine(msg);
         throw new Exception(msg);
      }

      class IndigoSQLFunctionInfo
      {
         public string ret_type, name;
         public List<KeyValuePair<string, string>> arguments = new List<KeyValuePair<string,string>>();
         public AccessLevelKind access_level;
         public string code;

         public void writeCode (StreamWriter stream)
         {
            stream.Write("CREATE ");
            if (ret_type != "")
               stream.Write("FUNCTION ");
            else
               stream.Write("PROCEDURE ");
            stream.Write("{0} \n", name);

            if (ret_type != "")
               stream.Write("  (\n");
            bool if_first = true;
            foreach (KeyValuePair<string, string> p in arguments)
            {
               if (!if_first)
               {
                  stream.Write(",");
                  stream.Write("\n");
               }
               stream.Write("    @{0} {1}", p.Key, p.Value);
               if_first = false;
            }
            if (!if_first)
               stream.Write("\n");
            if (ret_type != "")
            {
               stream.Write("  )\n");
               stream.Write("  RETURNS {0}\n", ret_type);
            }
            stream.Write("AS\n");
            stream.Write("{0}\n", code);
            stream.Write("GO\n");
         }

         public void writeSign (StreamWriter stream)
         {
            stream.Write("ADD SIGNATURE TO {0} BY CERTIFICATE $(bingo)_certificate\n", name);
            stream.Write("  WITH PASSWORD = '$(bingo_pass)'\n");
            stream.Write("GO\n\n");
         }

         public void writeDrop (StreamWriter stream)
         {
            stream.Write("DROP ");
            if (ret_type != "")
               stream.Write("FUNCTION ");
            else
               stream.Write("PROCEDURE ");
            stream.Write("{0} \n", name);
            stream.Write("GO\n\n");
         }

         public void writeGrant (StreamWriter stream)
         {
            if (access_level != AccessLevelKind.None)
            {
               string permissions = "execute";
               if (ret_type.StartsWith("TABLE"))
                  permissions = "select";
               string role;
               if (access_level == AccessLevelKind.Reader)
                  role = "reader";
               else
                  role = "operator";
               stream.Write("grant {0} on {1} to $(bingo)_{2}", permissions, name, role);
               stream.WriteLine("\nGO\n");
            }
         }

         public void writeCreate (StreamWriter stream, bool wrapper_func)
         {
            writeCode(stream);
            if (!wrapper_func)
               writeSign(stream);
            if (wrapper_func)
               writeGrant(stream);
         }

      }

      class IndigoSQLFunction
      {
         public string comment;
         public IndigoSQLFunctionInfo main_function = new IndigoSQLFunctionInfo();
         public List<IndigoSQLFunctionInfo> wrappers = new List<IndigoSQLFunctionInfo>();

         public void writeCreate (StreamWriter stream)
         {
            stream.Write("--\n-- {0}\n--\n", comment);
            main_function.writeCreate(stream, false);
            foreach (IndigoSQLFunctionInfo m in wrappers)
               m.writeCreate(stream, true);
         }

         public void writeDrop (StreamWriter stream)
         {
            foreach (IndigoSQLFunctionInfo m in wrappers)
               m.writeDrop(stream);
            main_function.writeDrop(stream);
         }
      }

      static IndigoSQLFunctionInfo CreateWrapper (IndigoSQLFunctionInfo parent, bool binary, MethodInfo m)
      {
         StringBuilder code_builder = new StringBuilder();

         IndigoSQLFunctionInfo w = new IndigoSQLFunctionInfo();
         w.name = String.Format("[$(bingo)].{0}{1}", m.Name, binary ? "B" : "");
         if (parent.ret_type.StartsWith("TABLE"))
            w.ret_type = "TABLE";
         else
         {
            w.ret_type = parent.ret_type;
            code_builder.Append("BEGIN\n");
         }
         w.access_level = parent.access_level;

         object[] attributes = m.GetCustomAttributes(typeof(BingoSqlFunctionAttribute), true);
         BingoSqlFunctionAttribute attr = (BingoSqlFunctionAttribute)(attributes[0]);

         if (parent.ret_type != "")
            code_builder.Append("  RETURN ");
         else
            code_builder.Append("  EXEC ");

         if (parent.ret_type.StartsWith("TABLE"))
            code_builder.Append("(SELECT * FROM ");

         code_builder.AppendFormat("{0} ", parent.name);
         if (parent.ret_type != "")
            code_builder.Append("(");

         bool is_first = true;
         bool str_bin_found = false;
         foreach (KeyValuePair<string, string> p in parent.arguments)
         {
            if (!is_first)
               code_builder.Append(", ");

            if (attr.substitute_bingo && p.Key == "bingo_schema")
               code_builder.Append("'$(bingo)'");
            else if (attr.substitute_bingo && p.Key == "bingo_db")
               code_builder.Append("'$(database)'");
            else if (attr.str_bin == p.Key && !binary)
            {
               code_builder.AppendFormat("cast(@{0} as VARBINARY(max))", p.Key);
               w.arguments.Add(new KeyValuePair<string, string>(p.Key, "varchar(max)"));
               str_bin_found = true;
            }
            else
            {
               code_builder.AppendFormat("@{0}", p.Key);
               w.arguments.Add(p);
            }
            is_first = false;
         }

         if (!binary && attr.str_bin != null && !str_bin_found)
         {
            String msg = String.Format("Cannot find {0} in {1}", attr.str_bin, m.Name);
            System.Console.WriteLine(msg);
            throw new Exception(msg);
         }

         if (parent.ret_type != "")
            code_builder.Append(")");
         if (parent.ret_type.StartsWith("TABLE"))
            code_builder.Append(")");

         code_builder.Append("\n");
         if (!parent.ret_type.StartsWith("TABLE"))
            code_builder.Append("END");

         w.code = code_builder.ToString();
         return w;
      }

      static List<IndigoSQLFunction> GenerateFunctions (IEnumerable<MethodInfo> methods)
      {
         List<IndigoSQLFunction> functions = new List<IndigoSQLFunction>();

         foreach (MethodInfo m in methods)
         {
            IndigoSQLFunction f = new IndigoSQLFunction();
            f.comment = m.Name;

            object[] attributes = m.GetCustomAttributes(typeof(BingoSqlFunctionAttribute), true);
            BingoSqlFunctionAttribute attr = (BingoSqlFunctionAttribute)(attributes[0]);

            IndigoSQLFunctionInfo mf = f.main_function;
            mf.name = String.Format("[$(bingo)].z_{0}", m.Name);
            mf.ret_type = SqlType(m.ReturnType.Name, m);
            mf.access_level = attr.access_level;
            foreach (ParameterInfo p in m.GetParameters())
               mf.arguments.Add(new KeyValuePair<string,string>(p.Name, SqlType(p.ParameterType.Name, m)));

            mf.code = String.Format("  EXTERNAL NAME [$(bingo)_assembly].[{0}].{1}",
               m.ReflectedType.FullName, m.Name);

            // Create wrappers
            f.wrappers.Add(CreateWrapper(mf, false, m)); 
            if (attr.str_bin != null)
               f.wrappers.Add(CreateWrapper(mf, true, m)); 

            functions.Add(f);
         }

         return functions;
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

         foreach (IndigoSQLFunction m in GenerateFunctions(GetMethodsWithSqlAttributes(a)))
         {
            m.writeCreate(create_file);
            m.writeDrop(drop_file);
         }

         drop_file.Close();
         create_file.Close();
      }

   }
}
