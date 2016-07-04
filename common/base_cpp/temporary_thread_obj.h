/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
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

#ifndef __temporary_thread_obj__
#define __temporary_thread_obj__

#include <unordered_map>
#include <chrono>
#include <thread>
#include <memory>
#include <mutex>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

//
// Various Indigo API methods returns pointers to the temporary buffers
// This leads to an issues when multiple threads gets the same pointers
// To avoid this we returns a temporary object that belongs to the current
// thread for a short time (10 sec).
//
template <typename T>
class TemporaryThreadObjManager
{
public:
   // This method return a temporary object for this thread that lives at least 10 seconds.
   // Method is synchronized
   T& getObject ();

private:
   typedef std::chrono::steady_clock steady_clock;
   struct Container
   {
      Container();
      Container(Container&& that);

      steady_clock::time_point timestamp;
      std::unique_ptr<T> obj;
   };

   typedef std::unordered_map<std::thread::id, Container> Map;
   Map _objects;

   std::mutex access_mutex;
};

#include "temporary_thread_obj.hpp"

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif // __tlscont_h__
