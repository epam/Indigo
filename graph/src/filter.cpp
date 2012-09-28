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


#include "graph/filter.h"
#include "graph/graph.h"

using namespace indigo;

IMPL_ERROR(Filter, "filter");

Filter::Filter () :
_filter(0),
_value(0),
_type(0)
{
}

Filter::Filter (const int *filter, int type, int value) :
_filter(filter),
_value(value),
_type(type)
{
}

void Filter::init (const int *filter, int type, int value)
{
   _own.clear();
   _filter = filter;
   _value = value;
   _type = type;
}

void Filter::initAll (int size)
{
   _own.clear_resize(size);
   _own.zerofill();
   _filter = _own.ptr();
   _type = EQ;
   _value = 0;
}

void Filter::initNone (int size)
{
   _own.clear_resize(size);
   _own.zerofill();
   _filter = _own.ptr();
   _type = NEQ;
   _value = 0;
}

void Filter::hide (int idx)
{
   if (_own.size() < 1)
      throw Error("can not hide() without initAll() or initNone()");

   if (_type == EQ && _value == 0)
      _own[idx] = 1;
   else if (_type == NEQ && _value == 0)
      _own[idx] = 0;
   else
      throw Error("not implemented");

}

void Filter::unhide (int idx)
{
   if (_own.size() < 1)
      throw Error("can not hide() without initAll() or initNone()");

   if (_type == EQ && _value == 0)
      _own[idx] = 0;
   else if (_type == NEQ && _value == 0)
      _own[idx] = 1;
   else
      throw Error("not implemented");
}

bool Filter::valid (int idx) const
{
   if (_filter == 0)
      throw Error("uninitialized");

   if (_type == EQ)
      return _filter[idx] == _value;

   if (_type == NEQ)
      return _filter[idx] != _value;
      
   if (_type == LESS)
      return _filter[idx] < _value;

   if (_type == MORE)
      return _filter[idx] > _value;

   throw Error("unknown filter type %d", _type);
}

int Filter::count (const Graph &graph) const
{
   if (_filter == 0)
      throw Error("uninitialized");

   int n = 0;

   for (int i = graph.vertexBegin(); i != graph.vertexEnd(); i = graph.vertexNext(i))
      if (valid(i))
         n++;

   return n;
}

void Filter::collectGraphVertices (const Graph &graph, Array<int> &indices) const
{
   if (_filter == 0)
      throw Error("uninitialized");

   indices.clear();

   for (int i = graph.vertexBegin(); i != graph.vertexEnd(); i = graph.vertexNext(i))
      if (valid(i))
         indices.push(i);
}

void Filter::collectGraphEdges (const Graph &graph, Array<int> &indices) const
{
   if (_filter == 0)
      throw Error("uninitialized");

   indices.clear();

   for (int i = graph.edgeBegin(); i != graph.edgeEnd(); i = graph.edgeNext(i))
      if (valid(i))
         indices.push(i);
}
