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

#ifndef _AUX_PATH_FINDER_H__
#define	_AUX_PATH_FINDER_H__

#include "base_cpp/array.h"
#include "base_cpp/queue.h"

namespace indigo {

class AuxiliaryGraph;
class AuxPathFinder {
public:
   AuxPathFinder (AuxiliaryGraph &graph, int max_size);

   bool find (Array<int>& vertices, Array<int>& edges, int u, int v);
private:
   Queue<int> _queue;
   Array<int> _prev;
   AuxiliaryGraph &_graph;
   AuxPathFinder(const AuxPathFinder&);
};

}

#endif	/* _AUX_PATH_FINDER_H */

