/****************************************************************************
 * Copyright (C) 2010 GGA Software Services LLC
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
#include "molecule/molecule_fingerprint.h"

TL_DEF_EXT(Indigo, indigo_self);

DLLEXPORT Indigo & indigoGetInstance ()
{
   TL_GET(Indigo, indigo_self);
   return indigo_self;
}

CEXPORT const char * indigoVersion ()
{
   return "1.0-beta2";
}

Indigo::Indigo ()
{
   error_handler = 0;
   error_handler_context = 0;
   _next_id = 1001;

   ignore_stereochemistry_errors = false;
   treat_x_as_pseudoatom = false;
   deconvolution_aromatization = true;
   molfile_saving_mode = 0;
   filename_encoding = ENCODING_ASCII;
   fp_params.any_qwords = 15;
   fp_params.sim_qwords = 8;
   fp_params.tau_qwords = 10;
   fp_params.ord_qwords = 25;
}

void Indigo::removeAllObjects ()
{
   OsLocker lock(_objects_lock);
   int i;

   for (i = _objects.begin(); i != _objects.end(); i = _objects.next(i))
      delete _objects.value(i);

   _objects.clear();
}

Indigo::~Indigo ()
{
   removeAllObjects();
}

CEXPORT qword indigoAllocSessionId ()
{
   return TL_ALLOC_SESSION_ID();
}

CEXPORT void indigoSetSessionId (qword id)
{
   TL_SET_SESSION_ID(id);
}

CEXPORT void indigoReleaseSessionId (qword id)
{
   TL_SET_SESSION_ID(id);
   TL_GET(Indigo, indigo_self);
   indigo_self.removeAllObjects();
   TL_RELEASE_SESSION_ID(id);
}

CEXPORT const char * indigoGetLastError (void)
{
   TL_GET2(Indigo, self, indigo_self);

   return self.error_message.ptr();
}

CEXPORT void indigoSetErrorHandler (void (*handler)
                 (const char *message, void *context), void *context)
{
   TL_GET2(Indigo, self, indigo_self);

   self.error_handler = handler;
   self.error_handler_context = context;
}

CEXPORT int indigoFree (int handle)
{
   INDIGO_BEGIN
   {
      self.removeObject(handle);
      return 1;
   }
   INDIGO_END(-1);
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
   TL_GET2(Indigo, self, indigo_self);

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
