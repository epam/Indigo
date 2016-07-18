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

#ifndef __cleaner_2d__
#define __cleaner_2d__

#include "molecule\molecule.h"

namespace indigo
{

class DLLEXPORT Cleaner2d {

public:
    Cleaner2d(Molecule& mol);
    void clean(); 
private:
    void updatePosition(int i);
    void updatePositions();
    void add_coef(int ver, int index, Vec2f value);
    void calc_coef(int to, int from0, int from1);

    Molecule& _mol;
    Array<int> base_point;
    Array<int> base_point_comp;
    Array<Vec2f> pos;
    bool is_biconnected;
    int vertex_count;
    int component_count;
    ObjArray< Array<bool> > in;
    ObjArray<Array<int> > def;
    ObjArray<Array<Vec2f> > coef; // linear representation for every vertices throw base points over field of complex numbers

    Vec2f plane(Vec3f v) { return Vec2f(v.x, v.y); }
    Vec2f mult(Vec2f& a, Vec2f& b) { return Vec2f(a.x * b.x  - a.y * b.y, a.x * b.y + a.y * b.x); }

    const Vec2f ZERO = Vec2f(0., 0.);
    const Vec2f ONE = Vec2f(1., 0.);
};

    
}

#endif
