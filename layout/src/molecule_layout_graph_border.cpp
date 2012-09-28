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

#include "layout/molecule_layout_graph.h"

using namespace indigo;

// Return:
//   0 - no intersection
//   1 - intersection
//  -1 - unknown. Ray is too near to the one of the points
static int _isRayIntersectWithCheck (float a, float b, const Vec2f &p, const Vec2f &v1, const Vec2f &v2, bool check_precision) 
{
   // Ray x=at+p.x, y=bt+p.y, t>=0 and segment [V1,V2];
   float a11, a12, a21, a22, b1, b2;
   float delta, delta1, delta2, t, s, a0, b0, pr;
   const float eps = 0.0001f;

   a11 = a;
   a12 = v1.x - v2.x;
   b1 = v1.x - p.x;
   a21 = b;
   a22 = v1.y - v2.y;
   b2 = v1.y - p.y;
   delta = a11 * a22 - a12 * a21;
   delta2 = a11 * b2 - a21 * b1;
   delta1 = b1 * a22 - b2 * a12;

   if (fabs(delta) < eps)
   {
      if (fabs(b1 * a21 - b2 * a11) > eps) 
         return 0;

      if (fabs(a11) > eps) 
      {
         a0 = b1 / a11;
         b0 = (b1 - a12) / a11;
         if (b0 < a0) 
         {
            pr = a0;
            a0 = b0;
            b0 = pr;
         }
      } else
      {
         a0 = b2 / a21;
         b0 = (b2 - a22) / a21;
         if (b0 < a0) 
         {
            pr = a0;
            a0 = b0;
            b0 = pr;
         }
      }
      if (check_precision)
         if (fabs(a0) < eps && fabs(b0) <= eps)
            return -1;

      if (a0 <= -eps && b0 <= -eps)
         return 0;
      return 1;
   }
   
   t = delta1 / delta;
   s = delta2 / delta;

   if (check_precision)
      if (fabs(s) < eps || fabs(s - 1) < eps)
         return -1;

   if (t < -eps || s < -eps || s > 1 + eps)
      return 0;
   return 1;
}

static bool _isRayIntersect (float a, float b, const Vec2f &p, const Vec2f &v1, const Vec2f &v2) 
{
   return _isRayIntersectWithCheck(a, b, p, v1, v2, false) == 1;
}

// Check if point is outside biconnected component
// By calculating number of intersections of ray
bool MoleculeLayoutGraph::_isPointOutside (const Vec2f &p) const
{
   int i, count = 0;
   float a, b;
   const float eps = 0.01f;

   bool success = false;

   while (!success)
   {
      success = true;

      a = (float)rand();
      b = (float)rand();
      a = 2.f * (a / RAND_MAX - 0.5f);
      b = 2.f * (b / RAND_MAX - 0.5f);

      if (fabs(a) < eps || fabs(b) < eps)
      {
         success = false;
         continue;
      }

      if (_outline.get() == 0)
      {
         for (i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
         {
            const Vec2f &pos = getPos(i);

            if (_layout_vertices[i].type == ELEMENT_BOUNDARY && fabs((pos.x - p.x) / a - (pos.y - p.y) / b) < 0.1f)
            {
               count++;

               if (count > 100)
                  return false;

               success = false;
               break;
            }
         }
      } else
      {
         const Array<Vec2f> &outline = *_outline.get();

         for (i = 0; i < outline.size(); i++)
         {
            if (fabs((outline[i].x - p.x) / a - (outline[i].y - p.y) / b) < 0.1f)
            {
               count++;

               if (count > 100)
                  return false;

               success = false;
               break;
            }
         }
      }
   }

   // Calculate
   count = 0;

   if (_outline.get() == 0)
   {
      for (i = edgeBegin(); i < edgeEnd(); i = edgeNext(i)) 
      {
         if (_layout_edges[i].type == ELEMENT_BOUNDARY)
         {
            const Edge &edge = getEdge(i);

            if (_isRayIntersect(a, b, p, getPos(edge.beg), getPos(edge.end)))
               count++;
         }
         
      }
   } else
   {
      const Array<Vec2f> &outline = *_outline.get();

      for (i = 0; i < outline.size(); i++)
         if (_isRayIntersect(a, b, p, outline[i], outline[(i + 1) % outline.size()]))
            count++;
   }

   if (count & 1)
      return false;
   return true;
}

// Check if point is outside cycle
// By calculating number of intersections of ray
bool MoleculeLayoutGraph::_isPointOutsideCycle (const Cycle &cycle, const Vec2f &p) const
{
   int i, count = 0;
   float a, b;
   Vec2f v1, v2;
   const float eps = 0.01f;

   bool success = false;

   while (!success)
   {
      success = true;

      a = (float)rand();
      b = (float)rand();
      a = 2.f * (a / RAND_MAX - 0.5f);
      b = 2.f * (b / RAND_MAX - 0.5f);

      if (fabs(a) < eps || fabs(b) < eps)
      {
         success = false;
         continue;
      }

      for (i = 0; i < cycle.vertexCount(); i++)
      {
         const Vec2f &pos = getPos(cycle.getVertex(i));

         if (fabs((pos.x - p.x) / a - (pos.y - p.y) / b) < EPSILON) 
         {
            count++;

            if (count > 50)
               return false;

            success = false;
            break;
         }
      }
   }

   // Calculate
   count = 0;

   for (i = 0; i < cycle.vertexCount(); i++)
      if (_isRayIntersect(a, b, p, getPos(cycle.getVertex(i)),
                                   getPos(cycle.getVertex((i + 1) % cycle.vertexCount()))))
         count++;

   if (count & 1)
      return false;
   return true;
}

// The same but with mapping
bool MoleculeLayoutGraph::_isPointOutsideCycleEx (const Cycle &cycle, const Vec2f &p, const Array<int> &mapping) const
{
   // TODO: check that point 'p' is equal to the one of cycle points (sometimes it happens)
   float a, b;

   int tries = 0;
   while (tries < 50)
   {
      tries++;

      // Choose random direction
      a = (float)rand();
      b = (float)rand();
      a = 2.f * (a / RAND_MAX - 0.5f);
      b = 2.f * (b / RAND_MAX - 0.5f);

      // Calculate number of intersection with boundary
      int count = 0;

      for (int i = 0; i < cycle.vertexCount(); i++)
      {
         int ret = _isRayIntersectWithCheck(a, b, p, getPos(mapping[cycle.getVertex(i)]),
                                      getPos(mapping[cycle.getVertex((i + 1) % cycle.vertexCount())]), true);
         if (ret == -1)
         {
            // Ray is too near to the point. Choose another one point
            count = -1;
            break;
         }
         if (ret == 1)
            count++;
      }

      if (count == -1)
         // Try again
         continue;

      // If number of intersections is even then point is outside
      if (count & 1)
         return false;
      return true;
   }

   // Return any value hoping it will never happen
   return false;
}

// Extract component border
void MoleculeLayoutGraph::_getBorder (Cycle &border) const
{
   QS_DEF(Array<int>, vertices);
   QS_DEF(Array<int>, edges);
   int i, n = 0;

   for (i = edgeBegin(); i < edgeEnd(); i = edgeNext(i))
      if  (_layout_edges[i].type == ELEMENT_BOUNDARY)
         n++;

   if (n == 0)
      return;

   vertices.clear();
   edges.clear();

   for (i = edgeBegin(); i < edgeEnd(); i = edgeNext(i))
      if  (_layout_edges[i].type == ELEMENT_BOUNDARY)
         break;

   Edge edge = getEdge(i);

   vertices.push(edge.beg);
   edges.push(i);

   while (edge.end != vertices[0])
   {
      const Vertex &vert = getVertex(edge.end);
      bool found = false;

      for (int i = vert.neiBegin(); !found && i < vert.neiEnd(); i = vert.neiNext(i))
      {
         int nei_v = vert.neiVertex(i);
         int nei_e = vert.neiEdge(i);

         if (getEdgeType(nei_e) == ELEMENT_BOUNDARY && nei_v != edge.beg)
         {
            edge.beg = edge.end;
            edge.end = nei_v;

            vertices.push(edge.beg);
            edges.push(nei_e);

            found = true;
         }
      }

      if (!found || vertices.size() > n)
         throw Error("corrupted border");
   }

   border.copy(vertices, edges);
   border.canonize();
}

// Split border in two parts by two vertices
void MoleculeLayoutGraph::_splitBorder (int v1, int v2, Array<int> &part1v, Array<int> &part1e, Array<int> &part2v, Array<int> &part2e) const
{
   Cycle border;

   _getBorder(border);

   int idx1 = border.findVertex(v1);
   int idx2 = border.findVertex(v2);
   int i;

   if (idx1 == -1 || idx2 == -1)
      throw Error("border division by non-boundary vertex");

   if (idx1 > idx2)
      __swap(idx1, idx2, i);

   part1v.clear();
   part1e.clear();
   part2v.clear();
   part2e.clear();

   for (i = idx1; i < idx2 + 1; i++)
   {
      part1v.push(border.getVertex(i));
      part1e.push(border.getEdge(i));
   }
   
   part1e.pop(); // edge count is less

   for (i = idx2; i < border.vertexCount(); i++)
   {
      part2v.push(border.getVertex(i));
      part2e.push(border.getEdge(i));
   }

   for (i = 0; i < idx1 + 1; i++)
   {
      part2v.push(border.getVertex(i));
      part2e.push(border.getEdge(i));
   }

   part2e.pop(); // edge count is less
}

// Cycle enumerator callback
// Check if cycle is boundary and mark vertices and edges as boundary/internal
bool MoleculeLayoutGraph::_border_cb (Graph &graph, const Array<int> &vertices, const Array<int> &edges, void *context)
{
   MoleculeLayoutGraph &self = *(MoleculeLayoutGraph *)context;
   Cycle cycle(vertices, edges);

   //cycle.canonize();

   int i;
   QS_DEF(Array<int>, types);

   types.clear_resize(self.vertexEnd());

   for (i = self.vertexBegin(); i < self.vertexEnd(); i = self.vertexNext(i))
      types[i] = ELEMENT_INTERNAL;

   for (i = 0; i < cycle.vertexCount(); i++)
      types[cycle.getVertex(i)] = ELEMENT_BOUNDARY;

   // Check vertices not in cycle are inside it
   for (i = self.vertexBegin(); i < self.vertexEnd(); i = self.vertexNext(i))
      if (types[i] == ELEMENT_INTERNAL)
         if (self._isPointOutsideCycle(cycle, self.getPos(i)))
            return true; //continue

   // Check edge centers are inside cycle
   types.clear_resize(self.edgeEnd());

   for (i = self.edgeBegin(); i < self.edgeEnd(); i = self.edgeNext(i))
      types[i] = ELEMENT_INTERNAL;

   for (i = 0; i < cycle.vertexCount(); i++) 
      types[cycle.getEdge(i)] = ELEMENT_BOUNDARY;

   for (i = self.edgeBegin(); i < self.edgeEnd(); i = self.edgeNext(i))
      if (types[i] == ELEMENT_INTERNAL)
      {
         Vec2f p;
         const Edge &edge = self.getEdge(i);
         
         p.lineCombin2(self.getPos(edge.beg), 0.5f, self.getPos(edge.end), 0.5f);

         if (self._isPointOutsideCycle(cycle, p))
            return true; //continue
      }

   // Mark edges and bonds
   for (i = self.vertexBegin(); i < self.vertexEnd(); i = self.vertexNext(i))
      self._layout_vertices[i].type = ELEMENT_INTERNAL;

   for (i = self.edgeBegin(); i < self.edgeEnd(); i = self.edgeNext(i))
      self._layout_edges[i].type = ELEMENT_INTERNAL;

   for (i = 0; i < cycle.vertexCount(); i++) 
   {
      self._layout_vertices[cycle.getVertex(i)].type = ELEMENT_BOUNDARY;
      self._layout_edges[cycle.getEdge(i)].type = ELEMENT_BOUNDARY;
   }

   return false;
}

