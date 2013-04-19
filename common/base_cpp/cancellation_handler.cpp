/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
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

#include "base_cpp/cancellation_handler.h"

#include "base_c/nano.h"
#include "base_cpp/output.h"
#include "base_cpp/tlscont.h"

namespace indigo
{
//
// TimeoutCancellationHandler
//

TimeoutCancellationHandler::TimeoutCancellationHandler (int mseconds)
{
   reset(mseconds);
}

TimeoutCancellationHandler::~TimeoutCancellationHandler ()
{
}

bool TimeoutCancellationHandler::isCancelled ()
{
   qword dif_time = nanoClock() - _currentTime;
   if (_mseconds > 0 && nanoHowManySeconds(dif_time) * 1000 > _mseconds)
   {
      ArrayOutput mes_out(_message);
      mes_out.printf("The operation timed out: %d ms", _mseconds);
      mes_out.writeChar(0);
      return true;
   }
   return false;
}

const char* TimeoutCancellationHandler::cancelledRequestMessage ()
{
   return _message.ptr();
}

void TimeoutCancellationHandler::reset (int mseconds)
{
   _mseconds = mseconds;
   _currentTime = nanoClock();
}

//
// Global thread-local cancellation handler
//

class CancellationHandlerWrapper
{
public:
   CancellationHandlerWrapper () : handler(0) {}

   CancellationHandler* handler;
};

static _SessionLocalContainer<CancellationHandlerWrapper> cancellation_handler;

CancellationHandler* getCancellationHandler ()
{
   CancellationHandlerWrapper &wrapper = cancellation_handler.getLocalCopy();
   return wrapper.handler;
}

CancellationHandler* setCancellationHandler (CancellationHandler* handler)
{
   CancellationHandlerWrapper &wrapper = cancellation_handler.getLocalCopy();
   CancellationHandler* prev = wrapper.handler;
   wrapper.handler = handler;
   return prev;
}

AutoCancellationHandler::AutoCancellationHandler(CancellationHandler& hand) {
   _prev = setCancellationHandler(&hand);
}

AutoCancellationHandler::~AutoCancellationHandler() {
   setCancellationHandler(_prev);
}

}
