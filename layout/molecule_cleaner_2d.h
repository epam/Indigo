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

#ifndef __molecule_cleaner_2d__
#define __molecule_cleaner_2d__

#include "base_cpp/array.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/non_copyable.h"
#include "common/math/algebra.h"

namespace indigo
{
class Molecule;
   
class DLLEXPORT MoleculeCleaner2d : public NonCopyable {

public:
    MoleculeCleaner2d(Molecule& mol);
    void clean(); 
private:
    void _updatePosition(int i);
    void _updatePositions();
    void _addCoef(int ver, int index, Vec2f value);
    void _calcCoef(int to, int from0, int from1);
    void _updateGradient();
    void _updateGradient2();
    bool _isBasePoint(int i);
    float _energy();

    Molecule& _mol;
    Array<int> base_point;
    Array<int> base_point_index;
    Array<int> base_point_comp;
    Array<Vec2f> pos;
    bool is_biconnected;
    int vertex_count;
    int component_count;
    ObjArray< Array<bool> > in; // is vertex in component
    ObjArray<Array<int> > definiting_points; // definiting points for component
    ObjArray<Array<Vec2f> > coef; // linear representation for every vertices throw base points over field of complex numbers
    Array<Vec2f> gradient;
    Array<Vec2f> pregradient;
    Array<int> edge_comp;
    Array<bool> is_art_point;
    ObjArray<Array<bool> > adj_matrix;
    ObjArray<Array<int> > common_comp; // common_comp[i][j] = number of component wich is contains both vertices i and j (or -1 if there isnt such component)
    float target_len; // target length of bonds

    Vec2f plane(Vec3f& v) { return Vec2f(v.x, v.y); } // projection to plane z == 0 
    Vec2f mult(Vec2f& a, Vec2f& b) { return Vec2f(a.x * b.x  - a.y * b.y, a.x * b.y + a.y * b.x); } // complex multiplication of two complex numbers

    const float APPROX_STEP = 0.01; // step of derivate approximation
    const Vec2f ZERO = Vec2f(0., 0.); // complex zero
    const Vec2f ONE = Vec2f(1., 0.); // complex one
};

    
}

#endif // __molecule_cleaner_2d__
