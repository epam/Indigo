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

#ifndef __graph_constrained_bmatching_finder_h__
#define __graph_constrained_bmatching_finder_h__

#include "base_cpp/array.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/reusable_obj_array.h"
#include "base_cpp/tlscont.h"
#include "base_cpp/exception.h"

#include "graph/skew_symmetric_network.h"

namespace indigo {

class Graph;

/* A b-matching M of G=(V,E) is defined as a subset of E with
 * condition f(v) <= c(v), where c(v) is the node capacity and
 * f(v) is the number of incident to node v edges from M.
 *
 * B-matching M is call maximum if |M| >= |M'| for 
 * all b-matchings M'
 * 
 * The algorithm provides possibility to find B-matching with
 * specified cardinality.
 * In addition, it allows to find b-matching 
 * with constraints for capacities for vertices sets with hierarchy.
 *
 * Maximum edges multiplicities (when edge can be used multiple 
 * times) can be specified too. By default they are set to 1.
 *
 * Algorithm is based on finding maximum integer skew-symmetric 
 * flow in skew-symmetric network.
 */
class GraphConstrainedBMatchingFinder
{
public:
   /* Parameter per_node_set_id[i] defines node set id from 0 to N, 
    * but if per_node_set_id[i] = -1 then node doesn't belong to any 
    * constrained set. Vertices with same node set is are in the same 
    * constrained set
    * Initially all arc capacities are set to zero.
    */
   GraphConstrainedBMatchingFinder (const Graph &g, 
      const ObjArray< Array<int> > &nodes_per_set,
      const Array<int> *per_set_set_id);

   void setNodeCapacity    (int node, int capacity, int set_id);
   int  getNodeCapacity    (int node, int set_id) const;

   void setNodeSetCapacity (int set_id, int capacity);
   int  getNodeSetCapacity (int set_id) const;

   void setMaxEdgeMultiplicity (int edge, int capacity);
   int  getMaxEdgeMultiplicity (int edge) const;

   /* This method tries to find b-matching with specified cardinality.
    * If return value if true then matching was found, but if
    * return value is false then maximum b-matching was found
    * although it is has cardinality lower than specified.
    */
   bool findMatching (int cardinality);

   int getEdgeMultiplicity (int edge) const;
   int getNodeIncidentEdgesCount (int node) const;

   DECL_ERROR;
private:
   struct ConstraintSet
   { 
      int node;
      int in_arc; 
   };

   void _createSets     (int n, int root, const Array<int> *per_set_set_id);
   void _createSet      (int idx, int root, const Array<int> *per_set_set_id);
   void _createVertices ();
   void _createEdges    ();
   void _connectVerticesWithSets (const ObjArray< Array<int> > &nodes_per_set);

   const Graph &_g;

   TL_CP_DECL(SkewSymmetricNetwork, _network);
   // Edges mapping between graph and network
   TL_CP_DECL(Array<int>, _edges_graph_to_net);
   TL_CP_DECL(Array<int>, _vertices_graph_to_net);
   TL_CP_DECL(ReusableObjArray< Array<int> >, _vertices_capacity_arc_per_set);
   TL_CP_DECL(Array<ConstraintSet>, _constraint_sets);
   TL_CP_DECL(Array<int>, _edge_matching_multiplicity);
   TL_CP_DECL(Array<int>, _node_incident_edges_count);

   int _source_edge;
};

}

#endif // __graph_constrained_bmatching_h__

