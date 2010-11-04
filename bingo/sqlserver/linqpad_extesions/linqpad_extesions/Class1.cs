using System;
using System.Data.Common;
using System.Data.Linq;
using System.Data.Linq.Mapping;
using System.Data.SqlClient;
using System.Globalization;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Linq.Expressions;
using System.Collections.Generic;
using System.Data;
using System.IO;
using System.Runtime.CompilerServices;

namespace indigo
{
   public static class Utils
   {
      public static void DumpSDF(this IQueryable query, string fileName, string molfile_field)
      {
         if (File.Exists(fileName))
            File.Delete(fileName);

         using (var output = new FileStream(fileName, FileMode.CreateNew))
         {
            using (var writer = new StreamWriter(output))
            {
               FieldInfo[] fields = null;
               Type type = null;

               foreach (var r in query)
               {
                  if (type == null)
                  {
                     type = r.GetType();
                     fields = type.GetFields();

                     // Find molfile field
                     bool found = false;
                     for (int i = 0; i < fields.Length; i++)
                        if (fields[i].Name == molfile_field)
                        {
                           FieldInfo tmp = fields[0];
                           fields[0] = fields[i];
                           fields[i] = tmp;
                           found = true;
                           break;
                        }
                     if (!found)
                        throw new Exception("Cannot find " + molfile_field  + " field");

                  }

                  bool first = true;
                  foreach (var p in fields)
                  {
                     if (first)
                        writer.Write(p.GetValue(r));
                     else
                     {
                        writer.WriteLine(">  <" + p.Name + ">");
                        writer.WriteLine(p.GetValue(r) + "\n");
                     }
                     first = false;
                  }

                  writer.WriteLine("$$$$");
               }
            }
         }
      }
   }
}
