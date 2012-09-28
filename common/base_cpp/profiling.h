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

#ifndef __profiling_h__
#define __profiling_h__

#include "base_c/nano.h"
#include "base_cpp/os_sync_wrapper.h"
#include "base_cpp/array.h"
#include "base_cpp/obj_array.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

#define _PROF_GET_NAME_INDEX(var_name, name) \
   static int var_name##_name_index;                             \
   if (var_name##_name_index == 0)                               \
   {                                                             \
      indigo::OsLocker locker(indigo::_profiling_global_lock);   \
      if (var_name##_name_index == 0) {                          \
         indigo::ProfilingSystem &inst = indigo::ProfilingSystem::getInstance(); \
         var_name##_name_index = inst.getNameIndex(name);        \
      }                                                          \
   }
   

#define profTimerStart(var_name, name) \
   _PROF_GET_NAME_INDEX(var_name, name)   \
   indigo::_ProfilingTimer var_name##_timer(var_name##_name_index)

#define profTimerStop(var_name) \
   var_name##_timer.stop()

#define profTimerGetTime(var_name) \
   var_name##_timer.getTime()

#define profIncTimer(name, dt) \
   {                               \
      _PROF_GET_NAME_INDEX(var_name, name)   \
      ProfilingSystem &inst = ProfilingSystem::getInstance(); \
      inst.addTimer(var_name##_name_index, dt); \
   }

#define profIncCounter(name, count) \
   {                                       \
      _PROF_GET_NAME_INDEX(var_name, name)   \
      ProfilingSystem &inst = ProfilingSystem::getInstance(); \
      inst.addCounter(var_name##_name_index, count); \
   }

#define profTimersReset()        ProfilingSystem::getInstance().reset(false)
#define profTimersResetSession() ProfilingSystem::getInstance().reset(true)

#define profGetStatistics(output, all) ProfilingSystem::getInstance().getStatistics(output, all)

namespace indigo {
class Output;

class DLLEXPORT ProfilingSystem
{
public:
   static ProfilingSystem& getInstance ();

   static int getNameIndex (const char *name, bool add_if_not_exists = true);

   void addTimer      (int name_index, qword dt);
   void addCounter    (int name_index, int value);
   void reset         (bool all);
   void getStatistics (Output &output, bool get_all);

   bool  hasLabel            (const char *name);
   float getLabelExecTime    (const char *name, bool total = false);
   qword getLabelValue       (const char *name, bool total = false);
   qword getLabelCallCount   (const char *name, bool total = false);

   DECL_ERROR;
private:
   struct Record
   {
      enum { TYPE_TIMER, TYPE_COUNTER };

      struct Data {
         qword count, value, max_value;
         double square_sum;

         Data();

         void reset ();

         void add (qword value);
      };

      Data current, total;

      int type;

      void reset (bool all);
   };

   static int _recordsCmp (int idx1, int idx2, void *context);

   void _printTimerData (const Record::Data &data, Output &output);
   void _printCounterData (const Record::Data &data, Output &output);

   bool _hasLabelIndex (int name_index);
   void _ensureRecordExistanceLocked (int name_index);

   ObjArray<Record> _records;
   Array<int> _sorted_records;
   OsLock _lock;

   static ObjArray< Array<char> > _names;
};

// This class shouldn't be used explicitly
class DLLEXPORT _ProfilingTimer
{
public:
   _ProfilingTimer   (int name_index);
   ~_ProfilingTimer  ();

   qword stop    ();
   qword getTime () const;

private:
   int _name_index;
   qword _start_time, _dt;
};

extern DLLEXPORT OsLock _profiling_global_lock;

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif // __profiling_h__

