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

#ifndef __smart_output_h__
#define __smart_output_h__

#include "base_cpp/array.h"
#include "base_cpp/tlscont.h"
#include "base_cpp/exception.h"
#include "base_cpp/output.h"
#include "base_cpp/reusable_obj_array.h"

namespace indigo {

class SmartTableOutput : public Output
{
public:
   SmartTableOutput (Output &output, bool use_smart_printing);
   virtual ~SmartTableOutput ();

   virtual void write (const void *data, int size);
   virtual void seek  (int offset, int from);
   virtual int  tell  ();
   virtual void flush ();

   void setLineFormat (const char *line_format);
   void printHLine ();
   
   enum { HLINE_CHAR = '\a' } ;

   DECL_ERROR;

private:
   void _updateColumnWidths (int index, Array<int> &widths);
   void _printLineSmart     (int index, const Array<int> &widths);

   TL_CP_DECL(ReusableObjArray< Array<char> >, _lines);
   TL_CP_DECL(ReusableObjArray< Array<char> >, _line_formats);
   TL_CP_DECL(Array<int>, _line_format_index);

   Array<char> *_active_line;
   bool _use_smart_printing;
   Output &_output;
};

}

#endif // __smart_output_h__
