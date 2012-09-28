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

#ifndef __cycle_enumerator_h__
#define __cycle_enumerator_h__

#include "base_cpp/array.h"

namespace indigo {

class Graph;
class SpanningTree;
class Filter;

class CycleEnumerator
{
public:
   explicit CycleEnumerator (Graph &graph);
           ~CycleEnumerator ();

   int   min_length;
   int   max_length;
   void *context;

   Filter *vfilter;

   bool (*cb_check_vertex)(Graph &graph, int v_idx, void *context);
   bool (*cb_handle_cycle)(Graph &graph, const Array<int> &vertices, const Array<int> &edges, void *context);

   bool process ();

protected:
   bool _pathFinder (const SpanningTree &spt, int ext_v1, int ext_v2, int ext_e);
   Graph &_graph;
private:
   CycleEnumerator (const CycleEnumerator &); // no implicit copy
};

}
#endif
