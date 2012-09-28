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

#ifndef __graph_vertex_equivalence__
#define __graph_vertex_equivalence__

namespace indigo {

class Graph;
class Output;
class Scanner;

// Find equivalence classes for vertices.
// Used to check if vertices are equivalent during SSS
class GraphVertexEquivalence
{
public:

   virtual ~GraphVertexEquivalence () {}
   
   virtual void construct (const Graph &g) {}

   virtual void save (Output &output) {}
   virtual void load (Scanner &input, const Graph &g) {}

   virtual void prepareForQueries () {}

   virtual int  getVertexEquivalenceClassId (int vertex_idx) { return -1; }
   virtual void fixVertex   (int vertex_idx) {}
   virtual void unfixVertex (int vertex_idx) {}
   virtual bool useHeuristicFurther () { return false; }

   virtual bool isVertexInTransversal (int vertex_idx) { return true; }

   // This method shouldn't be here...
   virtual void setNeighbourhoodRadius (int radius) {}
};

}

#endif // __graph_vertex_equivalence__

