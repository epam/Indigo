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

#ifndef __molecule_cleaner_2d__
#define __molecule_cleaner_2d__

#include "base_cpp/array.h"
#include "base_cpp/non_copyable.h"
#include "base_cpp/obj_array.h"
#include "common/math/algebra.h"

namespace indigo
{
    class BaseMolecule;

    class DLLEXPORT MoleculeCleaner2d : public NonCopyable
    {

    public:
        enum
        {
            CLEAN_TIMER_MS = 2000
        };
        MoleculeCleaner2d(BaseMolecule& mol, bool use_biconnected_decompose);
        MoleculeCleaner2d(BaseMolecule& mol, bool use_biconnected_decompose, const Array<int>& selected_vertices);
        static void clean(BaseMolecule& mol);
        void do_clean(bool _clean_external_angles);

    private:
        bool _isZero();
        void _updatePosition(int i);
        void _updatePositions();
        void _addCoef(int ver, int index, Vec2f value);
        void _calcCoef(int to, int from0, int from1);
        void _calcCoef(int to, int from0, int from1, float alpha);
        void _updateGradient();
        void _updateGradient2();
        bool _isBasePoint(int i);
        float _energy();
        Vec2f _energyDiff(int v);
        float _localEnergy(int v);
        float _edgeEnergy(int i, int j);
        float _angleEnergy(int i, int v1, int v2);

        void _initComponents(bool use_beconnected_decomposition);
        void _initBasePointIndex();
        void _initGeometry();
        void _initArtPoints();
        void _initAdjMatrix();
        void _calcTargetLen();
        void _initCommonComp();
        void _initCommonBiconnectedComp();
        void _uniteBondsOnLine();
        void _initBasePointValid();

        BaseMolecule& _mol;

        Array<int> active_points;
        Array<int> base_point;
        Array<int> base_point_index;
        Array<Vec2f> pos;
        bool is_trivial;
        int vertex_size;
        int component_count;
        ObjArray<Array<bool>> in;               // is vertex in component
        ObjArray<Array<int>> definiting_points; // definiting points for component
        ObjArray<Array<Vec2f>> coef;            // linear representation for every vertices throw base points over field of complex numbers
        Array<Vec2f> gradient;
        Array<Vec2f> pregradient;
        Array<int> edge_comp;
        Array<bool> is_art_point;
        Array<bool> is_valid_base;
        ObjArray<Array<bool>> adj_matrix;
        ObjArray<Array<int>> common_comp; // common_comp[i][j] = number of component wich is contains both vertices i and j (or -1 if there isnt such component)
        ObjArray<Array<int>> common_bicon_comp; // common_bicon_comp[i][j] = number of biconnected component wich is contains both vertices i and j (or -1 if
                                                // there isnt such component)
        float target_len;                       // target length of bonds
        bool clean_external_angles;
        Array<bool> _is_trivial; // is component single edge or straightline chain
        Array<bool> _is_straightline_vertex;

        Vec2f plane(Vec3f& v)
        {
            return Vec2f(v.x, v.y);
        } // projection to plane z == 0
        Vec2f mult(Vec2f& a, Vec2f& b)
        {
            return Vec2f(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
        } // complex multiplication of two complex numbers

        const float APPROX_STEP = 0.01f;    // step of derivate approximation
        const Vec2f ZERO = Vec2f(0.f, 0.f); // complex zero
        const Vec2f ONE = Vec2f(1.f, 0.f);  // complex one
    };

} // namespace indigo

#endif // __molecule_cleaner_2d__
