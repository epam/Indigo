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

#include "base_c/defs.h"
#include "base_cpp/tlscont.h"
#include "graph/graph.h"
#include "graph/embedding_enumerator.h"
#include "graph/graph_vertex_equivalence.h"

EmbeddingEnumerator::EmbeddingEnumerator (Graph &supergraph) :
TL_CP_GET(_core_1),
TL_CP_GET(_core_2),
TL_CP_GET(_s_pool),
TL_CP_GET(_l_pool),
TL_CP_GET(_enumerators)
{
   _g2 = &supergraph;

   _core_2.clear_resize(supergraph.vertexEnd());

   _core_2.fffill(); // fill with UNMAPPED

   cb_embedding = 0;
   cb_match_vertex = 0;
   cb_match_edge = 0;
   cb_vertex_remove = 0;
   cb_edge_add = 0;
   cb_vertex_add = 0;
   userdata = 0;

   allow_many_to_one = false;

   _equivalence_handler = NULL;

   _enumerators.clear();
   _enumerators.push(*this);
}

EmbeddingEnumerator::~EmbeddingEnumerator ()
{
}

void EmbeddingEnumerator::setSubgraph (Graph &subgraph)
{
//   if (subgraph.vertexCount() < 1)
//      throw Error("empty subgraph given");

   _g1 = &subgraph;

   _core_1.clear_resize(_g1->vertexEnd());

   _core_1.fffill(); // fill with UNMAPPED

   _terminatePreviousMatch();
}

void EmbeddingEnumerator::ignoreSubgraphVertex (int idx)
{
   if (_g1 == 0)
      throw Error("no subgraph");

   _core_1[idx] = IGNORE;
}

void EmbeddingEnumerator::ignoreSupergraphVertex (int idx)
{
   _core_2[idx] = IGNORE;
}

void EmbeddingEnumerator::_terminatePreviousMatch ()
{
   for (int i = _g2->vertexBegin(); i < _g2->vertexEnd(); i = _g2->vertexNext(i))
      if (_core_2[i] >= 0)
         _core_2[i] = IGNORE;
      else if (_core_2[i] == TERM_OUT)
         _core_2[i] = UNMAPPED;
   _enumerators[0].reset();
}

void EmbeddingEnumerator::setEquivalenceHandler (GraphVertexEquivalence *equivalence_handler)
{
   _equivalence_handler = equivalence_handler;
}

bool EmbeddingEnumerator::fix (int node1, int node2)
{
   return _enumerators[0].fix(node1, node2, true);
}

bool EmbeddingEnumerator::unsafeFix (int node1, int node2)
{
   return _enumerators[0].fix(node1, node2, false);
}

int EmbeddingEnumerator::process ()
{
   processStart();

   if (processNext())
   {
      while (_enumerators.size() > 1)
         _enumerators.pop();
      return 0;
   }

   return 1;
}

void EmbeddingEnumerator::processStart ()
{
   if (_g1 == 0)
      throw Error("subgraph not set");

   if (_equivalence_handler != NULL)
      _equivalence_handler->prepareForQueries();

   if (_equivalence_handler != NULL)
      _enumerators[0].setUseEquivalence(_equivalence_handler->useHeuristicFurther());
   else
      _enumerators[0].setUseEquivalence(false);
}

bool EmbeddingEnumerator::processNext ()
{
   if (_enumerators.size() > 1)
   {
      _enumerators.top().restore();
      _enumerators.pop();
   }

   while (1)
   {
      int command = _enumerators.top().nextPair();

      if (command == _NOWAY)
      {
         if (_enumerators.size() > 1)
         {
            _enumerators.top().restore();
            _enumerators.pop();
         }
         else
            break;
      }
      else if (command == _ADD_PAIR)
      {
         int node1 = _enumerators.top()._current_node1;
         int node2 = _enumerators.top()._current_node2;

         _enumerators.reserve(_enumerators.size() + 1);
         _enumerators.push(_enumerators.top());
         _enumerators.top().addPair(node1, node2);
      }
      else if (command == _RETURN0)
         return true;
   }

   while (_enumerators.size() > 1)
      _enumerators.pop();

   return false;
}

EmbeddingEnumerator::_Enumerator::_Enumerator (EmbeddingEnumerator &context) :
_context(context),
_mapped_orbit_ids(context._s_pool),
_term1(context._l_pool),
_term2(context._l_pool),
_unterm2(context._l_pool)
{
   _t1_len    = 0;
   _t2_len    = 0;
   _core_len  = 0;

   _selected_node1 = -1;
   _selected_node2 = -1;

   _use_equivalence = false;

   _current_node1 = -1;
   _current_node2 = -1;
   _current_node2_parent = -1;
   _current_node2_nei_index = -1;
}

EmbeddingEnumerator::_Enumerator::_Enumerator (const EmbeddingEnumerator::_Enumerator &other) :
_context(other._context),
_mapped_orbit_ids(other._context._s_pool),
_term1(other._context._l_pool),
_term2(other._context._l_pool),
_unterm2(other._context._l_pool)
{
   _core_len  = other._core_len;
   _t1_len    = other._t1_len;
   _t2_len    = other._t2_len;

   if (other._use_equivalence)
      _use_equivalence = _context._equivalence_handler->useHeuristicFurther();
   else
      _use_equivalence = false;

   _current_node1 = -1;
   _current_node2 = -1;
   _current_node2_parent = -1;
   _current_node2_nei_index = -1;
}

void EmbeddingEnumerator::_Enumerator::addPair (int node1, int node2)
{
   if (_context._core_1[node1] == TERM_OUT)
      _t1_len--;
   if (_context._core_2[node2] == TERM_OUT)
      _t2_len--;

   _selected_node1 = node1;
   _selected_node2 = node2;

   _node1_prev_value = _context._core_1[node1];
   _node2_prev_value = _context._core_2[node2];

   _context._core_1[node1] = node2;
   _context._core_2[node2] = node1;

   _core_len++;

   const Vertex &v1 = _context._g1->getVertex(node1);
   const Vertex &v2 = _context._g2->getVertex(node2);

   int i;

   for (i = v1.neiBegin(); i != v1.neiEnd(); i = v1.neiNext(i))
   {
      int other1 = v1.neiVertex(i);

      if (_context._core_1[other1] == UNMAPPED)
      {
         _context._core_1[other1] = TERM_OUT;
         _t1_len++;
         _term1.add(other1);
      }
   }

   if (_t1_len > 0)
      for (i = v2.neiBegin(); i != v2.neiEnd(); i = v2.neiNext(i))
      {
         int other2 = v2.neiVertex(i);

         if (_context._core_2[other2] == UNMAPPED)
         {
            _context._core_2[other2] = TERM_OUT;
            _t2_len++;
            _term2.add(other2);
         }
      }
   else
   {
      // A connected component of subgraph has been mapped.
      // Need to reset TERM_OUT flags on supergraph.
      Graph *g2 = _context._g2;

      for (i = g2->vertexBegin(); i != g2->vertexEnd(); i = g2->vertexNext(i))
         if (_context._core_2[i] == TERM_OUT)
         {
            _context._core_2[i] = UNMAPPED;
            _unterm2.add(i);
         }

      _t2_len = 0;
   }


   if (_use_equivalence)
      _context._equivalence_handler->fixVertex(node2);
}

bool EmbeddingEnumerator::_Enumerator::_checkNode1 (int node1)
{
   int val = _context._core_1[node1];

   if (val == TERM_OUT)
      return true;

   if (_t1_len == 0 && val == UNMAPPED)
      return true;

   return false;
}

bool EmbeddingEnumerator::_Enumerator::_checkNode2 (int node2, int for_node1)
{
   int val = _context._core_2[node2];

   if (val == TERM_OUT)
      return true;

   if (_t2_len == 0 && val == UNMAPPED)
      return true;

   if (_context.allow_many_to_one)
   {
      if (val == IGNORE)
         return false;
      if (_context.cb_allow_many_to_one != 0)
      {
         if (!_context.cb_allow_many_to_one(*_context._g1, for_node1, _context.userdata))
            return false;
         // Check node that has already been mapped
         if (val >= 0 && !_context.cb_allow_many_to_one(*_context._g1, val, _context.userdata))
            return false;
      }

      return true;
   }

   return false;
}

bool EmbeddingEnumerator::_Enumerator::_checkPair (int node1, int node2)
{
   if (_context.cb_match_vertex != 0)
      if (!_context.cb_match_vertex(*_context._g1, *_context._g2, _context._core_1.ptr(),
                                    node1, node2, _context.userdata))
         return false;

   int j;
   bool needRemove = false;
   const Vertex &v1 = _context._g1->getVertex(node1);

   for (j = v1.neiBegin(); j != v1.neiEnd(); j = v1.neiNext(j))
   {
      int other1 = v1.neiVertex(j);
      int other2 = _context._core_1[other1];

      if (other2 >= 0)
      {
         int edge1 = v1.neiEdge(j);
         int edge2 = _context._g2->findEdgeIndex(node2, other2);

         if (edge2 == -1)
            break;

         if (_context.cb_match_edge != 0)
            if (!_context.cb_match_edge(*_context._g1, *_context._g2, edge1, edge2, _context.userdata))
               break;

         if (_context.cb_edge_add != 0)
            _context.cb_edge_add(*_context._g1, *_context._g2, edge1, edge2, _context.userdata);
         needRemove = true;
      }
   }

   if (j != v1.neiEnd())
   {
      if (needRemove && _context.cb_vertex_remove != 0)
         _context.cb_vertex_remove(*_context._g1, node1, _context.userdata);
      return false;
   }

   // This is vertex equivalence heuristics.
   if (_use_equivalence)
   {
      int eq_class = _context._equivalence_handler->getVertexEquivalenceClassId(node2);

      // Check if class isn't trivial
      if (eq_class != -1)
      {
         int pair_id = (node1 << 16) + eq_class;

         if (_mapped_orbit_ids.find_or_insert(pair_id))
         {
            if (needRemove && _context.cb_vertex_remove != 0)
               _context.cb_vertex_remove(*_context._g1, node1, _context.userdata);
            return false;
         }
      }
   }

   if (_context.cb_vertex_add != 0)
      _context.cb_vertex_add(*_context._g1, *_context._g2, node1, node2, _context.userdata);

   return true;
}

void EmbeddingEnumerator::_Enumerator::restore ()
{
   int i;

   for (i = _term1.begin(); i != _term1.end(); i = _term1.next(i))
      _context._core_1[_term1.at(i)] = UNMAPPED;

   for (i = _term2.begin(); i != _term2.end(); i = _term2.next(i))
      _context._core_2[_term2.at(i)] = UNMAPPED;

   for (i = _unterm2.begin(); i != _unterm2.end(); i = _unterm2.next(i))
      _context._core_2[_term2.at(i)] = TERM_OUT;

   if (_selected_node1 >= 0)
   {
      _context._core_1[_selected_node1] = _node1_prev_value;
      _context._core_2[_selected_node2] = _node2_prev_value;

      if (_context.cb_vertex_remove != 0)
         _context.cb_vertex_remove(*_context._g1, _selected_node1, _context.userdata);

      if (_use_equivalence)
         _context._equivalence_handler->unfixVertex(_selected_node2);
   }

}

int EmbeddingEnumerator::_Enumerator::nextPair ()
{
   Graph *g1 = _context._g1;
   Graph *g2 = _context._g2;

   if (_current_node1 == -1)
   {
      for (_current_node1 = g1->vertexBegin();
           _current_node1 != g1->vertexEnd();
           _current_node1 = g1->vertexNext(_current_node1))
         if (_checkNode1(_current_node1))
            break;
   }

   if (_current_node1 == g1->vertexEnd())
   {
      // all nodes of subgraph are mapped
      if (_context.cb_embedding == 0 || _context.cb_embedding(*g1, *g2,
          _context._core_1.ptr(), _context._core_2.ptr(), _context.userdata) == 0)
         return _RETURN0;
      else
         return _NOWAY;
   }

   // check for dead state
   if (_t1_len > _t2_len && !_context.allow_many_to_one)
      return _NOWAY;

   if (_t2_len == 0)
   {
      if (_current_node2 == -1)
         _current_node2 = g2->vertexBegin();
      else
         _current_node2 = g2->vertexNext(_current_node2);

      for (; _current_node2 != g2->vertexEnd();
             _current_node2 = g2->vertexNext(_current_node2))
      {
         if (!_checkNode2(_current_node2, _current_node1))
            continue;

         if (!_checkPair(_current_node1, _current_node2))
            continue;

         break;
      }

      if (_current_node2 == g2->vertexEnd())
         return _NOWAY;
   }
   else
   {
      // Find parent vertex for query _current_node1 vertex
      // and take coresponding vertex in target
      if (_current_node2_parent == -1)
      {
         const Vertex &v = _context._g1->getVertex(_current_node1);

         int j;
         for (j = v.neiBegin(); j != v.neiEnd(); j = v.neiNext(j))
         {
            int nei_vertex = v.neiVertex(j);
            if (_context._core_1[nei_vertex] >= 0)
            {
               _current_node2_parent = _context._core_1[nei_vertex];
               break;
            }
         }

         if (j == v.neiEnd())
            return _NOWAY;
      }

      const Vertex &v = _context._g2->getVertex(_current_node2_parent);

      if (_current_node2_nei_index == -1)
         _current_node2_nei_index = v.neiBegin();
      else
         _current_node2_nei_index = v.neiNext(_current_node2_nei_index);

      for (; _current_node2_nei_index != v.neiEnd();
             _current_node2_nei_index = v.neiNext(_current_node2_nei_index))
      {
         _current_node2 = v.neiVertex(_current_node2_nei_index);

         if (!_checkNode2(_current_node2, _current_node1))
            continue;

         if (!_checkPair(_current_node1, _current_node2))
            continue;

         break;
      }
      if (_current_node2_nei_index == v.neiEnd())
         return _NOWAY;
   }

   return _ADD_PAIR;
}

bool EmbeddingEnumerator::_Enumerator::fix (int node1, int node2, bool safe)
{
   if (_context._core_1[node1] != UNMAPPED && _context._core_1[node1] != TERM_OUT)
      return false;

   if (_context._core_2[node2] != UNMAPPED && _context._core_2[node2] != TERM_OUT)
      return false;

   if (safe && !_checkPair(node1, node2))
      return false;

   addPair(node1, node2);

   return true;
}

void EmbeddingEnumerator::_Enumerator::setUseEquivalence (bool use)
{
   _use_equivalence = use;
}

void EmbeddingEnumerator::_Enumerator::reset ()
{
   _mapped_orbit_ids.clear();
   _current_node1 = -1;
   _current_node2 = -1;
}

const int * EmbeddingEnumerator::getSubgraphMapping ()
{
   return _core_1.ptr();
}

const int * EmbeddingEnumerator::getSupergraphMapping ()
{
   return _core_2.ptr();
}

int EmbeddingEnumerator::countUnmappedSubgraphVertices ()
{
   if (_g1 == 0)
      throw Error("subgraph not set");

   int i, res = 0;

   for (i = _g1->vertexBegin(); i != _g1->vertexEnd(); i = _g1->vertexNext(i))
      if (_core_1[i] == TERM_OUT || _core_1[i] == UNMAPPED)
         res++;

   return res;
}

int EmbeddingEnumerator::countUnmappedSupergraphVertices ()
{
   int i, res = 0;

   for (i = _g2->vertexBegin(); i != _g2->vertexEnd(); i = _g2->vertexNext(i))
      if (_core_2[i] == TERM_OUT || _core_2[i] == UNMAPPED)
         res++;

   return res;
}

int EmbeddingEnumerator::countUnmappedSubgraphEdges ()
{
   int i, res = 0;

   for (i = _g1->edgeBegin(); i != _g1->edgeEnd(); i = _g1->edgeNext(i))
   {
      const Edge &edge = _g1->getEdge(i);

      if (_core_1[edge.beg] != TERM_OUT && _core_1[edge.beg] != UNMAPPED)
         continue;
      if (_core_1[edge.end] != TERM_OUT && _core_1[edge.end] != UNMAPPED)
         continue;

      res++;
   }

   return res;
}

int EmbeddingEnumerator::countUnmappedSupergraphEdges ()
{
   int i, res = 0;

   for (i = _g2->edgeBegin(); i != _g2->edgeEnd(); i = _g2->edgeNext(i))
   {
      const Edge &edge = _g2->getEdge(i);

      if (_core_2[edge.beg] != TERM_OUT && _core_2[edge.beg] != UNMAPPED)
         continue;
      if (_core_2[edge.end] != TERM_OUT && _core_2[edge.end] != UNMAPPED)
         continue;

      res++;
   }

   return res;
}
