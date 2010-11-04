using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.Serialization;

namespace com.scitouch.indigo
{
   public class IndigoException : Exception
   {
      public IndigoException ()
         : base()
      {
      }

      public IndigoException (string message)
         : base(message)
      {
      }

      protected IndigoException (SerializationInfo info, StreamingContext context)
         : base(info, context)
      {
      }

      public IndigoException (string message, Exception innerException)
         : base(message, innerException)
      {
      }
   }
}
