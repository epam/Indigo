/****************************************************************************
 * Copyright (C) 2009-2011 GGA Software Services LLC
 * 
 * This file is part of Indigo toolkit.
 * 
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#ifndef __cancellation_handler_h__
#define __cancellation_handler_h__

#include "base_c/defs.h"
#include "base_c/nano.h"

namespace indigo
{

class CancellationHandler
{
public:
   virtual bool isCancelled() = 0;
   virtual const char* cancelledRequestMessage() = 0;
};

class TimeoutCancellationHandler : public CancellationHandler
{
public:
   TimeoutCancellationHandler(int mseconds):_mseconds(mseconds)
   {
      _currentTime = nanoClock();
   }
   virtual ~TimeoutCancellationHandler(){}

   virtual bool isCancelled()
   {
      qword dif_time = nanoClock() - _currentTime;
      if( nanoHowManySeconds(dif_time) * 1000 > _mseconds)
      {
         ArrayOutput mes_out(_message);
         mes_out.printf("The operation timed out: %d ms", _mseconds);
         mes_out.writeChar(0);
         return true;
      }
      return false;
   }
   virtual const char* cancelledRequestMessage()
   {
      return _message.ptr();
   }
private:
   Array<char> _message;
   int _mseconds;
   qword _currentTime;
   TimeoutCancellationHandler(const TimeoutCancellationHandler&); //no implicit copy
};

}

#endif /* __cancellation_handler_h__ */

/* END OF 'cancellation_handler.H' FILE */
