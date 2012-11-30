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

#ifndef __biconnected_decomposer_h__
#define __biconnected_decomposer_h__

#include "base_cpp/tlscont.h"
#include "base_cpp/ptr_array.h"
#include "graph/graph.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class DLLEXPORT BiconnectedDecomposer
{
public:
   explicit BiconnectedDecomposer (const Graph &graph);
   virtual ~BiconnectedDecomposer ();

   // returns the amount of biconnected components
   int decompose ();

   int componentsCount ();

   bool isArticulationPoint (int idx) const;
   void getComponent (int idx, Filter &filter) const;
   const Array<int> & getIncomingComponents (int idx) const;
   int getIncomingCount (int idx) const;
   void getVertexComponents (int idx, Array<int> &components) const;

   DECL_ERROR;

protected:
   void _biconnect (int v, int u);

   bool _pushToStack (Array<int> &dfs_stack, int v);
   void _processIfNotPushed (Array<int> &dfs_stack, int w);

   const Graph &_graph;
   TL_CP_DECL(PtrArray<Array<int> >, _components);
   TL_CP_DECL(Array<int>, _dfs_order);
   TL_CP_DECL(Array<int>, _lowest_order);
   TL_CP_DECL(PtrArray<Array<int> >, _component_lists);
   TL_CP_DECL(Array<Array<int> *>, _component_ids);
   TL_CP_DECL(Array<Edge>, _edges_stack);
   int _cur_order;
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
