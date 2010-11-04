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

#ifndef __ring_canonizer_h__
#define __ring_canonizer_h__

#include "base_c/defs.h"
#include "base_cpp/exception.h"

class Graph;
class Output;
template<typename T> class DLLEXPORT Array;

class RingCanonizer
{
public:
   RingCanonizer () :
   cb_vertex_rank(0),
   cb_edge_rank(0),
   cb_vertex_code(0),
   cb_edge_code(0)
   {
   }

   void perform (const Graph &ring, Output &output);

   int (*cb_vertex_rank) (const Graph &tree, int vertex_idx);
   int (*cb_edge_rank)   (const Graph &tree, int edge_idx);

   void (*cb_vertex_code) (const Graph &tree, int vertex_idx, Output &output);
   void (*cb_edge_code)   (const Graph &tree, int edge_idx, Output &output);

   DEF_ERROR("ring canonizer");
protected:
   bool _makeCode (const Graph &ring, int start_vertex, int start_edge,
                   Array<int> &codes, Array<int> &indices);
};

#endif
