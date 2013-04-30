using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.Serialization;

namespace com.ggasoftware.indigo
{
   public class BingoException : Exception
   {
      public BingoException ()
         : base()
      {
      }

      public BingoException (string message)
         : base(message)
      {
      }

      protected BingoException (SerializationInfo info, StreamingContext context)
         : base(info, context)
      {
      }

      public BingoException (string message, Exception innerException)
         : base(message, innerException)
      {
      }
   }
}
