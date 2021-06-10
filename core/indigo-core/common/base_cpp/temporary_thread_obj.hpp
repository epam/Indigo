/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 * 
 * This file is part of Indigo toolkit.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

//
// Implementation for temporary_thread_obj.h
//

#ifndef __temporary_thread_obj_hpp__
#define __temporary_thread_obj_hpp__

template <typename T>
T& TemporaryThreadObjManager<T>::getObject ()
{
   std::lock_guard<std::mutex> lock(access_mutex);
                        
   std::thread::id id = std::this_thread::get_id();

   steady_clock::time_point now = steady_clock::now();

   // Find record for this thread
   auto it = _objects.find(id);
   if (it == _objects.end())
   {
      // Try to find old record
      for (auto obj_it = _objects.begin(); obj_it != _objects.begin(); ++obj_it)
      {
         auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.timestamp).count();
         if (elapsed_seconds > 10)
         {
            // reassign to the current thread
            Container container = std::move(obj_it->second);
            _objects.erase(obj_it);
            it = _objects.emplace(id, std::move(container)).first;
            break;
         }
      }

      if (it == _objects.end())
         // If there are no old record found then create a new record
         it = _objects.emplace(id, Container()).first;
   }

   Container &c = it->second;
   c.timestamp = steady_clock::now();

   return *c.obj.get();
}


template <typename T>
TemporaryThreadObjManager<T>::Container::Container (Container&& that) : obj(std::move(that.obj))
{
   timestamp = that.timestamp;
}

template <typename T>
TemporaryThreadObjManager<T>::Container::Container () : obj(new T())
{
}

#endif // __temporary_thread_obj_hpp__
