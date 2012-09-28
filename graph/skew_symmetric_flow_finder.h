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

#ifndef __skew_symmetric_flow_finder_h__
#define __skew_symmetric_flow_finder_h__

#include "base_cpp/array.h"
#include "base_cpp/tlscont.h"
#include "base_cpp/exception.h"

namespace indigo {

class SkewSymmetricNetwork;

/* Find maximum integer skew-symmetric flow in 
 * skew-symmetric network.
 */
class SkewSymmetricFlowFinder
{
public:
   SkewSymmetricFlowFinder (const SkewSymmetricNetwork &network);

   void process ();

   int  getArcValue (int arc) const;

   DECL_ERROR;
private:
   void _init ();

   bool _findAugmentatingPath    (Array<int> &vertices);
   bool _findAugmentatingPathRec (Array<int> &vertices);

   void _increaseFlowByPath (Array<int> &vertices);

   int  _getResidualCapacity (int edge, int from);
   bool _isEdgeAugmentating (int edge, int from, int sym_used_dir);

   void _dbgCheckConsistency ();

   TL_CP_DECL(Array<int>, _arc_values);
   TL_CP_DECL(Array<int>, _arc_sym);

   // Variables for path finding
   TL_CP_DECL(Array<int>, _edge_used_dir);
   TL_CP_DECL(Array<int>, _vertex_is_used);

   int _network_sink;
   const SkewSymmetricNetwork &_network;
};

}

#endif // __skew_symmetric_flow_finder_h__
