/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
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

// Attach two atoms to the same side of chain
void MoleculeLayoutGraphSmart::_attachEars (int vert_idx, int drawn_idx, int *ears, const Vec2f &rest_pos)
{
   Vec2f v1, v2, v3, v4;
   float phi = 13*PI/24;
   const Vertex &vert = getVertex(vert_idx);
   
   _layout_vertices[ears[0]].type = ELEMENT_IGNORE;
   _layout_vertices[ears[1]].type = ELEMENT_IGNORE;
   _layout_edges[vert.neiEdge(vert.findNeiVertex(ears[0]))].type = ELEMENT_BOUNDARY;
   _layout_edges[vert.neiEdge(vert.findNeiVertex(ears[1]))].type = ELEMENT_BOUNDARY;
   
   v1 = getPos(vert_idx);
   v2 = getPos(drawn_idx);
   _calculatePos(phi, v1, rest_pos, v3);
   _calculatePos(phi + 2*PI/3, v1, rest_pos, v4);
   
   if (Vec2f::dist(v3, v2) < Vec2f::dist(v4, v2))
      v3 = v4;
   
   _layout_vertices[ears[0]].pos = v3;
   _calculatePos(PI/4, v1, v3, getPos(ears[1]));
}

// Attach set of trivial components
void MoleculeLayoutGraphSmart::_attachDandlingVertices (int vert_idx, Array<int> &adjacent_list)
{
   int n_pos = 0, not_drawn_idx = 0, drawn_idx = -1;
   Vec2f v1, v2;
   QS_DEF(Array<Vec2f>, positions);
   int parity = 0;
   bool two_ears = false; // mark the case with two atoms to be drawn on the same side of chain

   const Vertex &vert = getVertex(vert_idx);
    
   // Calculate number of drawn edges
   for (int i = vert.neiBegin(); i < vert.neiEnd(); i = vert.neiNext(i))
   {
      if (getVertexType(vert.neiVertex(i)) != ELEMENT_NOT_DRAWN &&
          getEdgeType(vert.neiEdge(i)) != ELEMENT_NOT_DRAWN)
      {
         n_pos++;
         // amount of drown neibourhoods
         drawn_idx = i;
      } else
         not_drawn_idx = i;
   }

   if (n_pos > 1 && adjacent_list.size() == 1)
   {
      // n_pos of drawn edges and one not drawn 
      _calculatePositionsOneNotDrawn(positions, n_pos, vert_idx, not_drawn_idx);
   } 
   else 
   {
      // Single drawn edge
      _calculatePositionsSingleDrawn(vert_idx, adjacent_list, n_pos, drawn_idx, two_ears, positions, parity);
   }
   
   int ears[2] = {-1, -1};
   
   if (two_ears)
   {
      for (int i = 0; i < adjacent_list.size(); i++)
      {
         if (getVertex(adjacent_list[i]).degree() != 1)
            continue;
         if (ears[0] == -1)
            ears[0] = adjacent_list[i];
         else
            ears[1] = adjacent_list[i];
      }
   }

   // Calculate energy
   if (parity == 0)
      _orderByEnergy(positions);

   // Assign coordinates
   if (two_ears)
   {
      for (int i = 0; i < adjacent_list.size(); i++)
      {
         int j = adjacent_list[i];
         if (getVertex(j).degree() != 1)
         {
            _layout_vertices[j].type = ELEMENT_BOUNDARY;
            _layout_edges[vert.neiEdge(vert.findNeiVertex(j))].type = ELEMENT_BOUNDARY;
            _layout_vertices[j].pos = positions[0];
            break;
         }
      }
      
      _attachEars(vert_idx, vert.neiVertex(drawn_idx), ears, positions[0]);
      
      return;
   }
   
   int j = 0;
   while (adjacent_list.size() > 0)
   {
      int i = adjacent_list.pop();

      _layout_vertices[i].pos = positions[j];
      _layout_vertices[i].type = ELEMENT_BOUNDARY;
      _layout_edges[vert.neiEdge(vert.findNeiVertex(i))].type = ELEMENT_BOUNDARY;
      j++;
   }
}

bool MoleculeLayoutGraphSmart::_drawEdgesWithoutIntersection (const Cycle &cycle, Array<int> & cycle_vertex_types)
{
   bool is_attached = true;
   Vec2f p;

   QS_DEF(Array<int>, border1v);
   QS_DEF(Array<int>, border1e);
   QS_DEF(Array<int>, border2v);
   QS_DEF(Array<int>, border2e);

   for (int i = 0; i < cycle.vertexCount(); i++)
   {
      if (_layout_edges[cycle.getEdge(i)].type == ELEMENT_NOT_DRAWN)
      {
         for (int j = edgeBegin(); j < edgeEnd(); j = edgeNext(j))
            if (_layout_edges[j].type > ELEMENT_NOT_DRAWN)
               if ((_calcIntersection(i, j) % 10) != 1)
               {
                  is_attached = false;
                  break;
               }

         /*if (!is_attached)
            continue;

         if  (cycle_vertex_types[i] == ELEMENT_INTERNAL ||
            cycle_vertex_types[(i + 1) % cycle.vertexCount()] == ELEMENT_INTERNAL)
         {
            _layout_edges[cycle.getEdge(i)].type = ELEMENT_INTERNAL;
         } 
         else
         {  // Both vertices are boundary.
            // Check if edge is boundary by its center
            p.lineCombin2(_layout_vertices[cycle.getVertex(i)].pos, 0.9f,
               _layout_vertices[cycle.getVertexC(i + 1)].pos, 0.1f);

            if  (_isPointOutside(p))
            {
               _splitBorder(cycle.getVertex(i), cycle.getVertexC(i + 1), border1v, border1e, border2v, border2e);

               _layout_edges[cycle.getEdge(i)].type = ELEMENT_BOUNDARY;

               // Ignore border1 and check if vertices are inside new bound
               // if no then restore Border1 and ignore Border2
               for (int j = 0; j < border1e.size(); j++)
               {
                  if (j > 0)
                     _layout_vertices[border1v[j]].type = ELEMENT_IGNORE;
                  _layout_edges[border1e[j]].type = ELEMENT_IGNORE;
               }

               p.lineCombin2(_layout_vertices[border1v[1]].pos, 0.9f, _layout_vertices[border1v[2]].pos, 0.1f);

               if (!_isPointOutside(p))
               {
                  for (int j = 0; j < border1e.size(); j++)
                  {
                     if (j > 0)
                        _layout_vertices[border1v[j]].type = ELEMENT_INTERNAL;
                     _layout_edges[border1e[j]].type = ELEMENT_INTERNAL;
                  }
               }
               else
               {
                  for (int j = 0; j < border1e.size(); j++)
                  {
                     if (j > 0)
                        _layout_vertices[border1v[j]].type = ELEMENT_BOUNDARY;
                     _layout_edges[border1e[j]].type = ELEMENT_BOUNDARY;
                  }
                  for (int j = 0; j < border2e.size(); j++)
                  {
                     if (j > 0)
                        _layout_vertices[border2v[j]].type = ELEMENT_INTERNAL;
                     _layout_edges[border2e[j]].type = ELEMENT_INTERNAL;
                  }
               }
            } 
            else
               _layout_edges[cycle.getEdge(i)].type = ELEMENT_INTERNAL;
         }*/
      }
   }

   return is_attached;
}

bool MoleculeLayoutGraphSmart::_checkBadTryBorderIntersection (Array<int> &chain_ext, MoleculeLayoutGraph &next_bc, Array<int> &mapping)
{
   for (int i = 0; i < chain_ext.size() - 1; i++)
      for (int j = next_bc.edgeBegin(); j < next_bc.edgeEnd(); j = next_bc.edgeNext(j))
      {
         if (_layout_edges[next_bc._layout_edges[j].ext_idx].type == ELEMENT_BOUNDARY)
         {
            const Vertex &vert = next_bc.getVertex(mapping[chain_ext[i]]);
            int edge1_idx = vert.neiEdge(vert.findNeiVertex(mapping[chain_ext[i + 1]]));
            int intersect = next_bc._calcIntersection(edge1_idx, j);

            const Edge &edge1 = next_bc.getEdge(edge1_idx);
            const Edge &edge2 = next_bc.getEdge(j);

            // Check if edges intersect
            if ((intersect % 10) != 1 || (intersect == 21 && edge1.beg != edge2.beg && edge1.beg != edge2.end
               && edge1.end != edge2.beg && edge1.end != edge2.end))
            {
               return false;
            }
         }
      }

   return true;
}

bool MoleculeLayoutGraphSmart::_checkBadTryChainOutside (Array<int> &chain_ext, MoleculeLayoutGraph &next_bc, Array<int> & mapping)
{
   // Check chain_ext is outside bound
   for (int i = 1; i < chain_ext.size() - 1; i++)
   {
      if (!_isPointOutside(next_bc._layout_vertices[mapping[chain_ext[i]]].pos))
         return false;
   }
   return true;
}

void MoleculeLayoutGraphSmart::_calculatePositionsOneNotDrawn (Array<Vec2f> &positions, int n_pos, int vert_idx, int not_drawn_idx)
{
   positions.clear_resize(n_pos);

   const Vertex &vert = getVertex(vert_idx);
   Vec2f v1, v2, p0;
   float phi;

   QS_DEF(Array<float>, angles); // polar angles of drawn edges
   QS_DEF(Array<int>, edges); // edge indices in CCW order

   angles.clear();
   edges.clear();

   // find angles
   for (int i = vert.neiBegin(); i < vert.neiEnd(); i = vert.neiNext(i))
   {
      if (i == not_drawn_idx)
         continue;

      edges.push(i);
      Vec2f &v1 = getPos(vert.neiVertex(i));
      Vec2f &v2 = getPos(vert_idx);
      p0.diff(v1, v2);
      if (p0.length() < EPSILON)
      {
         // Perturbate coordinate
         v1.y += 0.001;
         p0.diff(v1, v2);
      }
      angles.push(p0.tiltAngle2());
   }

   // sort
   for (int i = 0; i < n_pos; i++)
      for (int j = i + 1; j < n_pos; j++)
         if (angles[i] > angles[j])
         {
            angles.swap(i, j);
            edges.swap(i, j);
         }

         // place new edge between drawn
         v1 = getPos(vert_idx);

         for (int i = 0; i < n_pos - 1; i++)
         {
            v2 = getPos(vert.neiVertex(edges[i]));
            phi = (angles[i + 1] - angles[i]) / 2;
            _calculatePos(phi, v1, v2, positions[i]);      
         }

         v2 = getPos(vert.neiVertex(edges.top()));
         phi = (2 * PI + angles[0] - angles.top()) / 2;
         _calculatePos(phi, v1, v2, positions.top());
}

void MoleculeLayoutGraphSmart::_calculatePositionsSingleDrawn (int vert_idx, Array<int> &adjacent_list, int &n_pos, 
                                                          int drawn_idx, bool &two_ears, Array<Vec2f> &positions, int &parity)
{
   // Split 2pi to n_pos+1 parts
   // Place vertices like regular polygon
   // Drawn is first vertex, other in CCW order

   Vec2f v1, v2;
   float phi;
   const Vertex &vert = getVertex(vert_idx);

   if (adjacent_list.size() > 1)
   {
      if (n_pos == 1 && adjacent_list.size() == 3) // to avoid four bonds to be drawn like cross
      {
         n_pos = 5;
         int n_matter = 0, n_matter_2 = 0, n_single = 0, n_double_bond = 0;
         const Vertex &drawn_vert = getVertex(vert.neiVertex(drawn_idx));

         if (drawn_vert.degree() > 2)
            n_matter_2++;
         else if (drawn_vert.degree() == 1)
            n_single++;

         if (_molecule != 0)
         {
            int type = _molecule->getBondOrder(_molecule_edge_mapping[_layout_edges[vert.neiEdge(drawn_idx)].ext_idx]);

            if (type == BOND_DOUBLE)
               n_double_bond++;
         }

         for (int i = 0; i < adjacent_list.size(); i++)
         {
            int adj_degree = getVertex(adjacent_list[i]).degree();

            if (adj_degree == 1)
               n_single++;
            else
               n_matter++;

            if (adj_degree > 2)
               n_matter_2++;

            if (_molecule != 0)
            {
               int nei_idx = vert.findNeiVertex(adjacent_list[i]);
               int type = _molecule->getBondOrder(_molecule_edge_mapping[_layout_edges[vert.neiEdge(nei_idx)].ext_idx]);

               if (type == BOND_DOUBLE)
                  n_double_bond++;
            }
         }

         if (n_matter == 1 && n_double_bond < 2) // draw ears
         {
            two_ears = true;
            n_pos = 2;
         } else if (n_matter_2 > 1 || n_double_bond > 1 || n_single == 4) // cross-like case
            n_pos = 3;
      } else
         n_pos = adjacent_list.size();
   }
   else 
   {
      int type1 = 0, type2 = 0;

      if (_molecule != 0)
      {
         int first_nei = vert.neiBegin();
         type1 = _molecule->getBondOrder(_molecule_edge_mapping[_layout_edges[vert.neiEdge(first_nei)].ext_idx]);
         type2 = _molecule->getBondOrder(_molecule_edge_mapping[_layout_edges[vert.neiEdge(vert.neiNext(first_nei))].ext_idx]);
      }
      if (n_pos != 1 || (!(type1 == BOND_TRIPLE || type2 == BOND_TRIPLE) && !(type1 == BOND_DOUBLE && type2 == BOND_DOUBLE)))
         n_pos = 2;
   }

   positions.clear_resize(n_pos);

   phi = 2 * PI / (n_pos + 1);
   v1 = getPos(vert_idx);
   v2 = getPos(vert.neiVertex(drawn_idx));

   _calculatePos(phi, v1, v2, positions[0]);

   for (int i = 1; i < n_pos; i++)
   {
      v2 = positions[i - 1];
      _calculatePos(phi, v1, v2, positions[i]);
   }

   // Check cis/trans
   if (_molecule != 0 && n_pos == 2)
   {
      parity = _molecule->cis_trans.getParity(_molecule_edge_mapping[_layout_edges[vert.neiEdge(drawn_idx)].ext_idx]);

      if (parity != 0)
      {
         int substituents[4];
         _molecule->cis_trans.getSubstituents_All(_molecule_edge_mapping[_layout_edges[vert.neiEdge(drawn_idx)].ext_idx], substituents);

         int to_draw_substituent = -1;

         for (int i = 0; i < 4; i++)
            if (substituents[i] == _layout_vertices[adjacent_list.top()].ext_idx)
            {
               to_draw_substituent = i;
               break;
            }

         const Vertex &drawn_vert = getVertex(vert.neiVertex(drawn_idx));

         int drawn_substituent = -1;
         int drawn_substituent_idx = -1;

         for (int i = drawn_vert.neiBegin(); i < drawn_vert.neiEnd(); i = drawn_vert.neiNext(i))
            if (drawn_vert.neiVertex(i) != vert_idx) // must be drawn
            {
               for (int j = 0; j < 4; j++)
                  if (substituents[j] == _layout_vertices[drawn_vert.neiVertex(i)].ext_idx)
                  {
                     drawn_substituent_idx = drawn_vert.neiVertex(i);
                     drawn_substituent = j;
                     break;
                  }
                  break;
            }

         bool same_side = false;

         if ((parity == MoleculeCisTrans::CIS) == (abs(to_draw_substituent - drawn_substituent) == 2))
            same_side = true;

         int side_sign = MoleculeCisTrans::sameside(Vec3f(_layout_vertices[vert.neiVertex(drawn_idx)].pos), 
            Vec3f(_layout_vertices[vert_idx].pos), Vec3f(_layout_vertices[drawn_substituent_idx].pos), Vec3f(positions[0]));

         if (same_side)
         {
            if (side_sign == -1)
               positions.swap(0, 1);
         } else if (side_sign == 1)
            positions.swap(0, 1);
      }
   }
}

void MoleculeLayoutGraphSmart::_orderByEnergy (Array<Vec2f> &positions)
{
   QS_DEF(Array<double>, energies);
   QS_DEF(Array<double>, norm_a);
   double norm = 0.0;
   float r = 0.f;
   Vec2f p0;

   int n_pos = positions.size();

   energies.clear_resize(n_pos);
   norm_a.clear_resize(vertexEnd());
   energies.zerofill();

   for (int i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
      if (getVertexType(i) != ELEMENT_NOT_DRAWN && getVertexType(i) != ELEMENT_IGNORE)
      {
         norm_a[i] = _layout_vertices[i].morgan_code;
         norm += norm_a[i] * norm_a[i];
      }

   norm = sqrt(norm);

   for (int i = 0; i < n_pos; i++) 
   {
      for (int j = vertexBegin(); j < vertexEnd(); j = vertexNext(j))
         if (getVertexType(j) != ELEMENT_NOT_DRAWN && getVertexType(j) != ELEMENT_IGNORE)
         {
            p0.diff(positions[i], getPos(j));
            r = p0.lengthSqr();

            if  (r < EPSILON)
            {
               energies[i] = 1E+20f;
               continue;
            }

            energies[i] += ((norm_a[j] / norm + 0.5) / r);
         }
   }

   // Sort by energies
   for (int i = 0; i < n_pos; i++)
      for (int j = i + 1; j < n_pos; j++)
         if (energies[j] < energies[i])
         {
            energies.swap(i, j);
            positions.swap(i, j);
         }
}
