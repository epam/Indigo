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

#include "base_cpp/tlscont.h"
#include "graph/morgan_code.h"

using namespace indigo;

MorganCode::MorganCode (const Graph &g) :
_g(g)
{
}

void MorganCode::calculate (Array<long> &codes, int coeff, int iteration_count)
{
   QS_DEF(Array<long>, next_codes);

   next_codes.clear_resize(_g.vertexEnd());
   codes.clear_resize(_g.vertexEnd());

   int i, j, k;

   for (i = _g.vertexBegin(); i < _g.vertexEnd(); i = _g.vertexNext(i))
      codes[i] = _g.getVertex(i).degree();

   for (j = 0; j < iteration_count; j++)
   {
      for (i = _g.vertexBegin(); i < _g.vertexEnd(); i = _g.vertexNext(i))
      {
         next_codes[i] = coeff * codes[i];

         const Vertex &vertex = _g.getVertex(i);

         for (k = vertex.neiBegin(); k < vertex.neiEnd(); k = vertex.neiNext(k)) 
            next_codes[i] += codes[vertex.neiVertex(k)];
      }

      memcpy(codes.ptr(), next_codes.ptr(), sizeof(long) * _g.vertexEnd());
   }
}
