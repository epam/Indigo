using System;
using System.Runtime.InteropServices;
using System.Text;

namespace com.epam.indigo
{
    internal class UTF8Marshaller : ICustomMarshaler
    {
        private static readonly Lazy<UTF8Marshaller> _marshaller = new Lazy<UTF8Marshaller>(() => new UTF8Marshaller());

        public void CleanUpManagedData(object managedObj)
        {
        }

        public void CleanUpNativeData(IntPtr pNativeData)
        {
            Marshal.FreeHGlobal(pNativeData);
        }

        public int GetNativeDataSize()
        {
            return -1;
        }

        public IntPtr MarshalManagedToNative(object managedObj)
        {
            if (managedObj == null) return IntPtr.Zero;

            var source = managedObj is string managedStr
                ? Encoding.UTF8.GetBytes(managedStr)
                : throw new MarshalDirectiveException("UTF8Marshaller must be used on a string.");

            var destination = Marshal.AllocHGlobal(source.Length + 1);
            Marshal.Copy(source, 0, destination, source.Length);
            Marshal.WriteByte(destination + source.Length, 0);
            return destination;
        }

        public object MarshalNativeToManaged(IntPtr str)
        {
            return GetString(str);
        }

        // Used implicitly
        public static ICustomMarshaler GetInstance(string _)
        {
            return _marshaller.Value;
        }

        public static unsafe string GetString(IntPtr str)
        {
            if (str == IntPtr.Zero) return null;

            var byteCount = 0;
            var numPtr = (byte*)(void*)str;
            while (*numPtr++ > 0) byteCount++;
            return Encoding.UTF8.GetString((byte*)(void*)str, byteCount);
        }
    }
}
