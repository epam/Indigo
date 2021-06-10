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

#ifndef __haworth_projection_finder__
#define __haworth_projection_finder__

#include "base_c/defs.h"
#include "base_cpp/array.h"
#include "base_cpp/list.h"
#include "base_cpp/tlscont.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class BaseMolecule;

    class DLLEXPORT HaworthProjectionFinder
    {
    public:
        HaworthProjectionFinder(BaseMolecule& mol);

        void find();
        void findAndAddStereocenters();

        bool isBoldBond(int e_idx);

        const Array<bool>& getAtomsMask();
        const Array<bool>& getBondsMask();

    private:
        void _find(bool add_stereo);
        bool _processRing(bool add_stereo, const Array<int>& vertices, const Array<int>& edges);

        void _markRingBonds(const Array<int>& vertices, const Array<int>& edges);
        void _addRingStereocenters(const Array<int>& vertices, const Array<int>& edges);

        bool _isCornerVertex(int v, int e1, int e2);
        bool _isHorizontalEdge(int e, float cos_threshold);
        bool _isVerticalEdge(int e, float cos_threshold);
        float _getAngleCos(int v, int e, float dx, float dy);
        float _getAngleCos(int v, int e1, int e2);
        float _getAngleSin(int v, int e1, int e2);

        BaseMolecule& _mol;
        CP_DECL;
        TL_CP_DECL(Array<bool>, _atoms_mask);
        TL_CP_DECL(Array<bool>, _bonds_mask);
        TL_CP_DECL(Array<bool>, _bold_bonds_mask);
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif // __haworth_projection_finder__
