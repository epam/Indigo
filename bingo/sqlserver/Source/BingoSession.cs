using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

namespace indigo
{
   class BingoSession : IDisposable
   {
      private ulong _session_id, _previous_session_id;

      public BingoSession()
      {
         _previous_session_id = BingoCore.lib.bingoGetSessionID();
         _session_id = BingoCore.lib.bingoAllocateSessionID();
         setSession();
      }

      public void setSession()
      {
         BingoCore.lib.bingoSetSessionID(_session_id);
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

      private void _releaseSesssion()
      {
         BingoCore.lib.bingoReleaseSessionID(_session_id);
         // Restore session ID because of possible nested BingoSessions
         BingoCore.lib.bingoSetSessionID(_previous_session_id);
      }
   }
}
