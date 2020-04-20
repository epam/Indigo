using System;
using System.Runtime.Serialization;

namespace com.epam.indigo
{
    /// <summary>
    /// Bingo exception
    /// </summary>
    public class BingoException : Exception
    {
        /// <summary>
        /// </summary>
        public BingoException()
            : base()
        {
        }

        /// <summary>
        /// </summary>
        public BingoException(string message)
            : base(message)
        {
        }

        /// <summary>
        /// </summary>
        protected BingoException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }

        /// <summary>
        /// </summary>
        public BingoException(string message, Exception innerException)
            : base(message, innerException)
        {
        }
    }
}
