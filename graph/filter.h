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

#ifndef __filter_h__
#define __filter_h__

#include "base_cpp/array.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class Graph;

class DLLEXPORT Filter
{
public:
   enum
   {     
      EQ = 1,
      NEQ = 2,
      LESS = 3,
      MORE = 4
   };

   Filter ();
   Filter (const int *filter, int type, int value);

   void init (const int *filter, int type, int value);

   void initAll (int size);
   void initNone (int size);

   void hide (int idx);
   void unhide (int idx);

   bool valid (int idx) const;

   void collectGraphVertices (const Graph &graph, Array<int> &indices) const;
   void collectGraphEdges (const Graph &graph, Array<int> &indices) const;
   int  count (const Graph &graph) const;

   DECL_ERROR;

protected:
   const int *_filter;

   Array<int> _own;

   int  _value;
   int  _type;

private:
   Filter (const Filter &); // no implicit copy
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
