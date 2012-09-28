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

#ifndef _CYCLE_BASIS_H_
#define	_CYCLE_BASIS_H_
#include "base_cpp/obj_array.h"
#include "base_cpp/red_black.h"

namespace indigo {

class Graph;

class CycleBasis {
public:
   CycleBasis() {}
   void create(const Graph& graph);

    int getCyclesCount() const { return _cycles.size(); }
    const Array<int>& getCycle(int num) const {return _cycles[num]; }

    bool containsVertex(int vertex) const;
    
private:
   CycleBasis(const CycleBasis&);// no implicit copy

   ObjArray< Array<int> > _cycles;
   
   RedBlackSet<int> _cycleVertices;
};

}
#endif	/* _CYCLE_BASIS_H */

