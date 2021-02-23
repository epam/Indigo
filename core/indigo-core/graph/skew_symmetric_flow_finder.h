/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#ifndef __skew_symmetric_flow_finder_h__
#define __skew_symmetric_flow_finder_h__

#include "base_cpp/array.h"
#include "base_cpp/exception.h"
#include "base_cpp/tlscont.h"

namespace indigo
{

    class SkewSymmetricNetwork;

    /* Find maximum integer skew-symmetric flow in
     * skew-symmetric network.
     */
    class SkewSymmetricFlowFinder
    {
    public:
        SkewSymmetricFlowFinder(const SkewSymmetricNetwork& network);

        void process();

        int getArcValue(int arc) const;

        DECL_ERROR;

    private:
        void _init();

        bool _findAugmentatingPath(Array<int>& vertices);
        bool _findAugmentatingPathRec(Array<int>& vertices);

        void _increaseFlowByPath(Array<int>& vertices);

        int _getResidualCapacity(int edge, int from);
        bool _isEdgeAugmentating(int edge, int from, int sym_used_dir);

        void _dbgCheckConsistency();

        CP_DECL;
        TL_CP_DECL(Array<int>, _arc_values);
        TL_CP_DECL(Array<int>, _arc_sym);

        // Variables for path finding
        TL_CP_DECL(Array<int>, _edge_used_dir);
        TL_CP_DECL(Array<int>, _vertex_is_used);

        int _network_sink;
        const SkewSymmetricNetwork& _network;
    };

} // namespace indigo

#endif // __skew_symmetric_flow_finder_h__
