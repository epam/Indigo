using System;
using System.Runtime.Serialization;

namespace com.epam.indigo
{
    public class IndigoException : Exception
    {
        public IndigoException()
           : base()
        {
        }

        public IndigoException(string message)
           : base(message)
        {
        }

        protected IndigoException(SerializationInfo info, StreamingContext context)
           : base(info, context)
        {
        }

        public IndigoException(string message, Exception innerException)
           : base(message, innerException)
        {
        }
    }
}
