using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

namespace indigo
{
   class BingoSession : IDisposable
   {
      private ulong _session_id = 0;
      private bool _session_valid = false;

      public BingoSession()
         : this(true)
      {
      }

      public BingoSession(bool create_session)
      {
         if (create_session)
         {
            _createSesssion();
            setSession();
         }
      }

      public void setSession()
      {
         if (_session_valid)
            bingoSetSessionID(_session_id);
      }

      public void acquire(BingoSession session)
      {
         _releaseSesssion();
         _session_valid = session._session_valid;
         _session_id = session._session_id;
         session._session_valid = false;
      }

      public void Dispose()
      {
         _releaseSesssion();
         GC.SuppressFinalize(this);
      }

      ~BingoSession()
      {
         Dispose();
      }

      private void _createSesssion()
      {
         if (!_session_valid)
         {
            _session_id = bingoAllocateSessionID();
            _session_valid = true;
            //BingoLog.logMessage("Bingo-core session {0} allocated", _session_id);
         }
      }

      private void _releaseSesssion()
      {
         if (_session_valid)
         {
            bingoReleaseSessionID(_session_id);
            _session_valid = false;
            //BingoLog.logMessage("Bingo-core session {0} released", _session_id);
         }
      }

      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      [return: MarshalAs(UnmanagedType.U8)]
      private static extern ulong bingoAllocateSessionID();

      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      private static extern void bingoReleaseSessionID([MarshalAs(UnmanagedType.U8)] ulong id);

      [DllImport("bingo-core-c.dll", CharSet = CharSet.Auto)]
      private static extern void bingoSetSessionID([MarshalAs(UnmanagedType.U8)] ulong id);
   }
}
