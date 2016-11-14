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

#include "indigo_internal.h"

#include "base_cpp/output.h"
#include "base_cpp/profiling.h"
#include "base_cpp/temporary_thread_obj.h"
#include "molecule/molecule_fingerprint.h"
#include "reaction/rxnfile_saver.h"
#include "molecule/molfile_saver.h"

_SessionLocalContainer<Indigo> indigo_self;

DLLEXPORT Indigo & indigoGetInstance ()
{
   return indigo_self.getLocalCopy();
}

CEXPORT const char * indigoVersion ()
{
   return INDIGO_VERSION;
}

void Indigo::init ()
{
   error_handler = 0;
   error_handler_context = 0;
   _next_id = 1001;

   stereochemistry_options.reset();
   ignore_noncritical_query_features = false;
   treat_x_as_pseudoatom = false;
   skip_3d_chirality = false;
   deconvolution_aromatization = true;
   deco_save_ap_bond_orders = false;
   deco_ignore_errors = true;
   molfile_saving_mode = 0;
   molfile_saving_no_chiral = false;
   filename_encoding = ENCODING_ASCII;
   fp_params.any_qwords = 15;
   fp_params.sim_qwords = 8;
   fp_params.tau_qwords = 10;
   fp_params.ord_qwords = 25;
   fp_params.ext = true;

   embedding_edges_uniqueness = false;
   find_unique_embeddings = true;
   max_embeddings = 10000;

   layout_max_iterations = 0;

   molfile_saving_skip_date = false;

   molfile_saving_add_stereo_desc = false;

   molfile_saving_add_implicit_h = true;

   smiles_saving_write_name = false;
   smiles_saving_smarts_mode = false;

   aam_cancellation_timeout = 0;
   cancellation_timeout = 0;

   preserve_ordering_in_serialize = false;

   unique_dearomatization = false;

   arom_options = AromaticityOptions();

   // Update global index
   static ThreadSafeStaticObj<OsLock> lock;
   {
      OsLocker locker(lock.ref());
      static int global_id;

      _indigo_id = global_id++;
   }
}


Indigo::Indigo ()
{
   init();
}

void Indigo::removeAllObjects ()
{
   OsLocker lock(_objects_lock);
   int i;

   for (i = _objects.begin(); i != _objects.end(); i = _objects.next(i))
      delete _objects.value(i);

   _objects.clear();
}

void Indigo::updateCancellationHandler ()
{
    if (cancellation_timeout > 0) {
        resetCancellationHandler(new TimeoutCancellationHandler(cancellation_timeout));
    } else {
        resetCancellationHandler(nullptr);
    }
}

void Indigo::initMolfileSaver (MolfileSaver &saver)
{
   saver.mode = molfile_saving_mode;
   saver.skip_date = molfile_saving_skip_date;
   saver.no_chiral = molfile_saving_no_chiral;
   saver.add_stereo_desc = molfile_saving_add_stereo_desc;
   saver.add_implicit_h = molfile_saving_add_implicit_h;
}

void Indigo::initRxnfileSaver (RxnfileSaver &saver)
{
   saver.molfile_saving_mode = molfile_saving_mode;
   saver.skip_date = molfile_saving_skip_date;
   saver.add_stereo_desc = molfile_saving_add_stereo_desc;
   saver.add_implicit_h = molfile_saving_add_implicit_h;
}


Indigo::~Indigo ()
{
   removeAllObjects();
}

int Indigo::getId () const
{
   return _indigo_id;
}


CEXPORT qword indigoAllocSessionId ()
{
   qword id = TL_ALLOC_SESSION_ID();
   Indigo &indigo = indigo_self.getLocalCopy(id);
   indigo.init();

   return id;
}

CEXPORT void indigoSetSessionId (qword id)
{
   TL_SET_SESSION_ID(id);
}

CEXPORT void indigoReleaseSessionId (qword id)
{
   TL_SET_SESSION_ID(id);
   indigoGetInstance().removeAllObjects();
   TL_RELEASE_SESSION_ID(id);
}

CEXPORT const char * indigoGetLastError (void)
{
   Indigo &self = indigoGetInstance();
   return self.error_message.ptr();
}

CEXPORT void indigoSetErrorHandler (INDIGO_ERROR_HANDLER handler, void *context)
{
   Indigo &self = indigoGetInstance();
   self.error_handler = handler;
   self.error_handler_context = context;
}

CEXPORT int indigoFree (int handle)
{
   try
   {
      Indigo &self = indigoGetInstance();
      self.removeObject(handle);
   }
   catch (Exception &)
   {
   }
   return 1;
}

CEXPORT int indigoFreeAllObjects ()
{
   indigoGetInstance().removeAllObjects();
   return 1;
}

CEXPORT int indigoCountReferences (void)
{
   INDIGO_BEGIN
   {
      return self.countObjects();
   }
   INDIGO_END(-1);
}

CEXPORT void indigoSetErrorMessage (const char *message)
{
   Indigo &self = indigoGetInstance();
   self.error_message.readString(message, true);
}

int Indigo::addObject (IndigoObject *obj)
{
   OsLocker lock(_objects_lock);
   int id = _next_id++;
   _objects.insert(id, obj);
   return id;
}

void Indigo::removeObject (int id)
{
   OsLocker lock(_objects_lock);
   if (_objects.at2(id) == 0)
      return;
   delete _objects.at(id);
   _objects.remove(id);
}

IndigoObject & Indigo::getObject (int handle)
{
   OsLocker lock(_objects_lock);

   try
   {
      return *_objects.at(handle);
   }
   catch (RedBlackMap<int, IndigoObject *>::Error &e)
   {
      throw IndigoError("can not access object #%d: %s", handle, e.message());
   }
}

int Indigo::countObjects ()
{
   OsLocker lock(_objects_lock);

   return _objects.size();
}

static TemporaryThreadObjManager<Indigo::TmpData> _indigo_temporary_obj_manager;
Indigo::TmpData& Indigo::getThreadTmpData ()
{
   return _indigo_temporary_obj_manager.getObject();
}


//
// IndigoError
//

IndigoError::IndigoError (const char *format, ...) :
Exception()
{
   va_list args;

   va_start(args, format);
   _init("core", format, args);
   va_end(args);
}
   
IndigoError::IndigoError (const IndigoError &other) : Exception()
{
   other._cloneTo(this);
}

//
// IndigoPluginContext
//
IndigoPluginContext::IndigoPluginContext ()
{
   indigo_id = -1;
}

void IndigoPluginContext::validate ()
{
   Indigo &indigo = indigoGetInstance();
   if (indigo.getId() != indigo_id)
   {
      init();
      indigo_id = indigo.getId();
   }
}


//
// Options registrator
//

// Options registrator is placed in the main module to avoid being ignored by a 
// linker as an unused object
// http://stackoverflow.com/questions/1229430/how-do-i-prevent-my-unused-global-variables-being-compiled-out
static _IndigoBasicOptionsHandlersSetter _indigo_basic_options_handlers_setter;

// 
// Debug methods
//

#ifdef _WIN32
#include <Windows.h>
#endif

CEXPORT void indigoDbgBreakpoint (void)
{
#ifdef _WIN32
   if (!IsDebuggerPresent())
   {
      char msg[200];
      sprintf(msg, "Wait for a debugger?\nPID=%d", GetCurrentProcessId());
      int ret = MessageBox(NULL, msg, "Debugging (indigoDbgBreakpoint)", MB_OKCANCEL);
      if (ret == IDOK)
      {
         while (!IsDebuggerPresent())
            Sleep(100);
      }
   }
#else
#endif
}

CEXPORT const char * indigoDbgProfiling (int whole_session)
{
   INDIGO_BEGIN
   {
      auto &tmp = self.getThreadTmpData();
      ArrayOutput out(tmp.string);
      profGetStatistics(out, whole_session != 0);

      tmp.string.push(0);
      return tmp.string.ptr();
   }
   INDIGO_END(0);
}

CEXPORT int indigoDbgResetProfiling (int whole_session)
{
   INDIGO_BEGIN
   {
      if (whole_session)
         profTimersResetSession();
      else
         profTimersReset();

      return 1;
   }
   INDIGO_END(-1);
}

CEXPORT qword indigoDbgProfilingGetCounter (const char *name, int whole_session)
{
   INDIGO_BEGIN
   {
      return indigo::ProfilingSystem::getInstance().getLabelCallCount(name, whole_session != 0);
   }
   INDIGO_END(-1);
}

