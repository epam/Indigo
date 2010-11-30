/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
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

namespace indigo {

class Graph;

class Filter
{
public:
   enum
   {     
      EQ = 1,
      NEQ = 2,
      LESS = 3,
      MORE = 4
   };

   DLLEXPORT Filter ();
   DLLEXPORT Filter (const int *filter, int type, int value);

   DLLEXPORT void init (const int *filter, int type, int value);

   DLLEXPORT void initAll (int size);
   DLLEXPORT void initNone (int size);

   DLLEXPORT void hide (int idx);
   DLLEXPORT void unhide (int idx);

   DLLEXPORT bool valid (int idx) const;

   DLLEXPORT void collectGraphVertices (const Graph &graph, Array<int> &indices) const;
   DLLEXPORT void collectGraphEdges (const Graph &graph, Array<int> &indices) const;
   DLLEXPORT int  count (const Graph &graph) const;

   DEF_ERROR("filter");

protected:
   const int *_filter;

   Array<int> _own;

   int  _value;
   int  _type;

private:
   Filter (const Filter &); // no implicit copy
};

}

#endif
