/****************************************************************************
 * Copyright (C) 2010-2011 GGA Software Services LLC
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

#ifndef __indigo_savers__
#define __indigo_savers__

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

#include "indigo_internal.h"

class DLLEXPORT IndigoSaver : public IndigoObject
{
public:
   IndigoSaver (Output &output);
   ~IndigoSaver ();

   void acquireOutput (Output *output);
   void close ();

   void appendObject (IndigoObject &object);

   static IndigoSaver* create (Output &output, const char *type);

protected:
   virtual void _appendHeader () {};
   virtual void _appendFooter () {};
   virtual void _append (IndigoObject &object) = 0;

   Output &_output;

private:
   bool _closed;
   Output *_own_output;
};

class IndigoSdfSaver : public IndigoSaver
{
public:
   IndigoSdfSaver (Output &output) : IndigoSaver(output) {}
   virtual const char * debugInfo ();
   static void append (Output &output, IndigoObject &object);
   static void appendMolfile (Output &output, IndigoObject &object);

protected:
   virtual void _append (IndigoObject &object);
};

class IndigoSmilesSaver : public IndigoSaver
{
public:
   IndigoSmilesSaver (Output &output) : IndigoSaver(output) {}
   virtual const char * debugInfo ();

   static void generateSmiles (IndigoObject &obj, Array<char> &out_buffer);

   static void append (Output &output, IndigoObject &object);

protected:
   virtual void _append (IndigoObject &object);
};

class IndigoCmlSaver : public IndigoSaver
{
public:
   IndigoCmlSaver (Output &output) : IndigoSaver(output) {}
   virtual const char * debugInfo ();
   static void append (Output &output, IndigoObject &object);
   static void appendHeader (Output &output);
   static void appendFooter (Output &output);

protected:
   virtual void _append (IndigoObject &object);
   virtual void _appendHeader ();
   virtual void _appendFooter ();
};

class IndigoRdfSaver : public IndigoSaver
{
public:
   IndigoRdfSaver (Output &output) : IndigoSaver(output) {}
   virtual const char * debugInfo ();
   static void append (Output &output, IndigoObject &object);
   static void appendRXN (Output &output, IndigoObject &object);
   static void appendHeader (Output &output);

protected:
   virtual void _append (IndigoObject &object);
   virtual void _appendHeader ();
};

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif // __indigo_savers__
