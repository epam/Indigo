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

#ifndef __path_enumerator_h__
#define __path_enumerator_h__

namespace indigo {

#include "base_cpp/array.h"

class Graph;

class PathEnumerator
{
public:
   explicit PathEnumerator (Graph &graph, int begin, int end);
           ~PathEnumerator ();

   int   max_length;
   void *context;

   bool (*cb_check_vertex)(Graph &graph, int v_idx, void *context);
   bool (*cb_check_edge)(Graph &graph, int e_idx, void *context);
   bool (*cb_handle_path)(Graph &graph, const Array<int> &vertices, const Array<int> &edges, void *context);

   void process ();

protected:
   bool _pathFinder ();

   Graph &_graph;
   int _begin;
   int _end;

private:
   PathEnumerator (const PathEnumerator &); // no implicit copy
};

}

#endif
