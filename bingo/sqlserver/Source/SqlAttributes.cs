using System;
using Microsoft.SqlServer.Server;

namespace indigo.SqlAttributes
{
   public enum AccessLevelKind { None, Reader, Operator };

   public class BingoSqlFunctionAttribute : Attribute
   {
      public AccessLevelKind access_level { get; set; }
      public bool substitute_schema { get; set; }

      public BingoSqlFunctionAttribute ()
      {
         access_level = AccessLevelKind.Operator;
         substitute_schema = true;
      }
   }

   public class BingoSqlFunctionForReaderAttribute : BingoSqlFunctionAttribute
   {
      public BingoSqlFunctionForReaderAttribute()
      {
          access_level = AccessLevelKind.Reader;
      }
   }
}
