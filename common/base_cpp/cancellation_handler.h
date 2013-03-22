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

#ifndef __cancellation_handler_h__
#define __cancellation_handler_h__

#include "base_c/defs.h"
#include "base_cpp/array.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo
{

class DLLEXPORT CancellationHandler
{
public:
   virtual bool isCancelled () = 0;
   virtual const char* cancelledRequestMessage () = 0;
};

class DLLEXPORT TimeoutCancellationHandler : public CancellationHandler
{
public:
   TimeoutCancellationHandler(int mseconds);
   virtual ~TimeoutCancellationHandler();

   virtual bool isCancelled();
   virtual const char* cancelledRequestMessage();

   void reset (int mseconds);

private:
   Array<char> _message;
   int _mseconds;
   qword _currentTime;
};

// Global thread-local cancellation handler
DLLEXPORT CancellationHandler* getCancellationHandler ();
// Returns previous cancellation handler
DLLEXPORT CancellationHandler* setCancellationHandler (CancellationHandler* handler);
/*
 * Automatic cancellation handler can be used to store cancel callback within one code block
 */
class DLLEXPORT AutoCancellationHandler {
public:
   AutoCancellationHandler(CancellationHandler& hand);
   virtual ~AutoCancellationHandler();
private:
   CancellationHandler* _prev;
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif /* __cancellation_handler_h__ */

/* END OF 'cancellation_handler.H' FILE */
