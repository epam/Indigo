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

#include "layout/molecule_layout_macrocycles.h"

#include "base_cpp/profiling.h"
#include "layout/molecule_layout.h"
#include <algorithm>
#include <cmath>
#include <limits.h>
#include <map>
#include <math/random.h>
#include <set>
#include <sstream>
#include <stack>
#include <stdio.h>
#include <string>
#include <vector>

using namespace std;
using namespace indigo;

IMPL_ERROR(MoleculeLayoutMacrocyclesLattice, "molecule_layout_macrocycles_lattice");

const float MoleculeLayoutMacrocyclesLattice::CHANGE_FACTOR = 1.0f;
const float MoleculeLayoutMacrocyclesLattice::SMOOTHING_MULTIPLIER = 0.2f;

CP_DEF(MoleculeLayoutMacrocyclesLattice);
CP_DEF(MoleculeLayoutMacrocyclesLattice::CycleLayout);

static const int MAX_ROT = 100; // rotation can be negative. We must add MAX_ROT * SIX to it for correct reminder obtaining

static const int dx[6] = {1, 0, -1, -1, 0, 1};
static const int dy[6] = {0, 1, 1, 0, -1, -1};

static const int getDx(int x)
{
    return dx[(x + SIX * MAX_ROT) % SIX];
}
static const int getDy(int y)
{
    return dy[(y + SIX * MAX_ROT) % SIX];
}

MoleculeLayoutMacrocyclesLattice::MoleculeLayoutMacrocyclesLattice(int size)
    : CP_INIT, TL_CP_GET(_vertex_weight), // tree size
      TL_CP_GET(_vertex_stereo),          // there is an angle in the vertex
      TL_CP_GET(_edge_stereo),            // trans-cis configuration
      TL_CP_GET(_positions), TL_CP_GET(_angle_importance), TL_CP_GET(_component_finish), TL_CP_GET(_target_angle), TL_CP_GET(_vertex_added_square),
      TL_CP_GET(_vertex_drawn)
{
    length = size;

    _vertex_weight.clear_resize(size);
    _vertex_weight.zerofill();

    _vertex_stereo.clear_resize(size);
    _vertex_stereo.zerofill();

    _edge_stereo.clear_resize(size);
    _edge_stereo.zerofill();

    _positions.clear_resize(size);

    _angle_importance.clear_resize(size);
    _angle_importance.fill(1);

    _component_finish.clear_resize(size);
    for (int i = 0; i < size; i++)
        _component_finish[i] = i;

    _target_angle.clear_resize(size);
    _target_angle.zerofill();

    _vertex_added_square.clear_resize(size);
    _vertex_added_square.zerofill();

    _vertex_drawn.clear_resize(size);
}

void MoleculeLayoutMacrocyclesLattice::doLayout()
{
    if (length <= 9)
    {
        bool has_trans = false;
        for (int i = 0; i < length; i++)
            if (_edge_stereo[i] == 2)
                has_trans = true;
        if (!has_trans)
        {
            float alpha = _2FLOAT(2. * M_PI / length);
            float r = 1 / sqrt(2 * (1 - cos(alpha)));
            for (int i = 0; i < length; i++)
            {
                _positions[i] = Vec2f(0, r);
                _positions[i].rotate(alpha * i);
            }
            return;
        }
    }
    calculate_rotate_length();

    rotate_cycle(rotate_length);
    AnswerField answfld(length, 0, 0, 0, _vertex_weight.ptr(), _vertex_stereo.ptr(), _edge_stereo.ptr());

    answfld.fill();

    QS_DEF(Array<answer_point>, points);
    points.clear_resize(0);

    for (int rot = -length; rot <= length; rot++)
    {
        for (int p = 1; p < 2; p++)
        {
            TriangleLattice& lat = answfld.getLattice(length, rot, p);
            for (int x = lat.getFirstValidX(); lat.isIncreaseForValidX(x); x++)
            {
                for (int y = lat.getFirstValidY(x); lat.isIncreaseForValidY(y); lat.switchNextY(y))
                {
                    if (lat.getCell(x, y) < SHORT_INFINITY)
                    {
                        answer_point point(rot, p, x, y);
                        points.push(point);
                    }
                }
            }
        }
    }

    _positions.clear_resize(length + 1);

    CycleLayout cl;
    initCycleLayout(cl);
    int best_number = -1;
    float best_rating = preliminary_layout(cl);

    points.qsort(&AnswerField::_cmp_answer_points, &answfld);

    Array<answer_point> path;
    path.clear_resize(length + 1);
    for (int i = 0; i < 100 && i < points.size(); i++)
    {
        answfld._restore_path(path.ptr(), points[i]);
        cl.init(path.ptr());
        smoothing(cl);

        float current_rating = rating(cl);

        if (current_rating + EPSILON < best_rating)
        {
            // printf("%d: %.5f\n", i, current_rating);
            best_rating = current_rating;
            best_number = i;
        }
    }

    if (best_number >= 0)
    {
        answfld._restore_path(path.ptr(), points[best_number]);
        cl.init(path.ptr());
        smoothing(cl);
    }
    else
        preliminary_layout(cl);

    for (int i = 0, j = 0; i < cl.vertex_count; i++)
        for (int t = cl.external_vertex_number[i]; t < cl.external_vertex_number[i + 1]; t++, j++)
            _positions[j] = cl.point[i] + (cl.point[i + 1] - cl.point[i]) * _2FLOAT(t - cl.external_vertex_number[i]) / _2FLOAT(cl.edge_length[i]);

    rotate_cycle(-rotate_length);
}

void MoleculeLayoutMacrocyclesLattice::calculate_rotate_length()
{

    rotate_length = 0;
    int max_value = -SHORT_INFINITY;

    for (int i = 0; i < length; i++)
    {
        if ((_edge_stereo[i] != 2) && _vertex_stereo[(i + 1) % length])
        {
            int value = 2 * _edge_stereo[i] + 2 * _vertex_weight[i] + 2 * _vertex_weight[(i + 1) % length] - _vertex_weight[(i + length - 1) % length] -
                        _vertex_weight[(i + 2) % length];

            if (rotate_length == -1 || value > max_value)
            {
                rotate_length = i;
                max_value = value;
            }
        }
    }
    rotate_length++;
}

void MoleculeLayoutMacrocyclesLattice::_rotate_ar_i(Array<int>& ar, Array<int>& tmp, int shift)
{
    for (int i = shift; i < length; i++)
        tmp[i - shift] = ar[i];
    for (int i = 0; i < shift; i++)
        tmp[i - shift + length] = ar[i];
    for (int i = 0; i < length; i++)
        ar[i] = tmp[i];
}

void MoleculeLayoutMacrocyclesLattice::_rotate_ar_d(Array<float>& ar, Array<float>& tmp, int shift)
{
    for (int i = shift; i < length; i++)
        tmp[i - shift] = ar[i];
    for (int i = 0; i < shift; i++)
        tmp[i - shift + length] = ar[i];
    for (int i = 0; i < length; i++)
        ar[i] = tmp[i];
}

void MoleculeLayoutMacrocyclesLattice::_rotate_ar_v(Array<Vec2f>& ar, Array<Vec2f>& tmp, int shift)
{
    for (int i = shift; i < length; i++)
        tmp[i - shift] = ar[i];
    for (int i = 0; i < shift; i++)
        tmp[i - shift + length] = ar[i];
    for (int i = 0; i < length; i++)
        ar[i] = tmp[i];
}

void MoleculeLayoutMacrocyclesLattice::rotate_cycle(int shift)
{
    shift = (shift % length + length) % length;

    QS_DEF(Array<int>, temp);
    temp.clear_resize(length);
    QS_DEF(Array<float>, tempd);
    tempd.clear_resize(length);
    QS_DEF(Array<Vec2f>, temp_v);
    temp_v.clear_resize(length);

    _rotate_ar_i(_vertex_weight, temp, shift);
    _rotate_ar_i(_vertex_stereo, temp, shift);
    _rotate_ar_i(_edge_stereo, temp, shift);
    _rotate_ar_d(_target_angle, tempd, shift);
    _rotate_ar_d(_angle_importance, tempd, shift);
    _rotate_ar_v(_positions, temp_v, shift);
}

const Vec2f& MoleculeLayoutMacrocyclesLattice::getPos(int v) const
{
    return _positions[v];
}

void MoleculeLayoutMacrocyclesLattice::setEdgeStereo(int e, int stereo)
{
    _edge_stereo[e] = stereo;
}

void MoleculeLayoutMacrocyclesLattice::addVertexOutsideWeight(int v, int weight)
{
    _vertex_weight[v] += WEIGHT_FACTOR * weight;
}

void MoleculeLayoutMacrocyclesLattice::setVertexEdgeParallel(int v, bool parallel)
{
    _vertex_stereo[v] = !parallel;
}

bool MoleculeLayoutMacrocyclesLattice::getVertexStereo(int v)
{
    return _vertex_stereo[v];
}

void MoleculeLayoutMacrocyclesLattice::setVertexAddedSquare(int v, float s)
{
    _vertex_added_square[v] = s;
}

void MoleculeLayoutMacrocyclesLattice::setVertexDrawn(int v, bool drawn)
{
    _vertex_drawn[v] = drawn;
}

void MoleculeLayoutMacrocyclesLattice::setComponentFinish(int v, int f)
{
    _component_finish[v] = f;
}

void MoleculeLayoutMacrocyclesLattice::setTargetAngle(int v, float angle)
{
    _target_angle[v] = angle;
}

void MoleculeLayoutMacrocyclesLattice::setAngleImportance(int v, float imp)
{
    _angle_importance[v] = imp;
}

void AnswerField::_restore_path(answer_point* path, answer_point finish)
{

    path[length] = finish;

    for (int len = length - 1; len >= 0; len--)
    {
        //      printf("len = %d, x = %d, y = %d, p = %d, rot = %d, value = %d \n", len + 1, path[len + 1].x, path[len + 1].y, path[len + 1].p, path[len +
        //      1].rot, get_field(len + 1, path[len + 1]));
        path[len] = path[len + 1];
        path[len].x -= getDx(path[len + 1].rot);
        path[len].y -= getDy(path[len + 1].rot);

        if (_vertex_stereo[len])
        {
            path[len].rot -= path[len + 1].p ? 1 : -1;

            if (len > 0 && _edge_stereo[len - 1] == 0)
            {
                path[len].p ^= 1;

                int rot = path[len + 1].rot;

                int add = get_weight(_vertex_weight[len], path[len + 1].p);

                // choosing rotation closer to circle
                float l = _2FLOAT(len * (sqrt(3.0) + 1.5) * M_PI / 12.);

                Vec2f vec(_2FLOAT(path[len].y), 0.f);
                vec.rotate(_2FLOAT(M_PI / 3.));
                vec += Vec2f(_2FLOAT(path[len].x), 0.f);
                float x = vec.length();

                const float eps = 1e-3f;

                float alpha = _2FLOAT(2. * M_PI);
                if (x > eps)
                {

                    float L = eps;
                    float R = _2FLOAT(2. * M_PI - eps);

                    while (R - L > eps)
                    {
                        float M = (L + R) / 2;
                        if (M * x / (2 * sin(M / 2)) > l)
                            R = M;
                        else
                            L = M;
                    }

                    alpha = vec.tiltAngle2() + R / 2;
                }

                int preferred_p = alpha > M_PI / 3 * (path[len].rot); // +M_PI / 6 / length;

                path[len].p = preferred_p ^ 1;

                // enumerating two cases
                for (int i = 0; i < 3; i++)
                {
                    if (i == 2)
                        throw Error("Cannot find path");
                    unsigned short a = get_field(len + 1, path[len + 1]);
                    unsigned short b = get_field(len, path[len]);

                    if (a == add + b)
                        break;
                    path[len].p ^= 1;
                }
            }
            else if (len > 0 && _edge_stereo[len - 1] == 2)
                path[len].p ^= 1;
        }
    }
}

// TriangleLattice definition

// x - y = difference_reminder (mod 3)

IMPL_ERROR(TriangleLattice, "triangle_lattice");

TriangleLattice::TriangleLattice()
{
    _BORDER.set_empty();
}

TriangleLattice::TriangleLattice(rectangle rec, int rem, uint8_t* data_link)
{
    init(rec, rem, data_link);
}

void TriangleLattice::init(rectangle rec, int rem, uint8_t* data_link)
{
    _BORDER = rec;

    _difference_reminder = rem;

    if (_BORDER.empty)
        return;

    // byte_start = data_link + sizeof(unsigned short*) * (_BORDER.max_x - _BORDER.min_x + 1);
    // byte_end = data_link + TriangleLattice::getAllocationSize(rec);

    _starts = (unsigned short**)data_link;

    _starts[0] = (unsigned short*)(_starts + _BORDER.max_x - _BORDER.min_x + 1);
    _starts -= _BORDER.min_x;

    for (int x = _BORDER.min_x; x < _BORDER.max_x; x++)
    {
        int left = _BORDER.min_y;
        int right = _BORDER.max_y;

        while (!isValid(x, left))
            left++;
        while (!isValid(x, right))
            right--;

        _starts[x + 1] = _starts[x] + (right - left + 3) / 3;
    }

    for (int x = _BORDER.min_x; x <= _BORDER.max_x; x++)
    {
        _starts[x] -= (getFirstValidY(x) + _difference_reminder - x) / 3;
    }
}

void TriangleLattice::init_void()
{
    _BORDER.set_empty();
    _difference_reminder = 0;
}

unsigned short& TriangleLattice::getCell(int x, int y)
{
    if (_BORDER.empty)
        return _sink;

#if defined(DEBUG) || defined(_DEBUG)
    if (!isValid(x, y))
        throw Error("difference of coordinates reminder is failed: x = %d, y = %d, rem = %d", x, y, _difference_reminder);
    if (!_BORDER.expand(3).contains(x, y))
        throw Error("point (%d, %d) is not close to framework [%d, %d]x[%d, %d].", x, y, _BORDER.min_x, _BORDER.max_x, _BORDER.min_y, _BORDER.max_y);
#endif

    if (!_BORDER.contains(x, y))
        return _sink;

    // if ((byte*)(_starts[x] + (y + _difference_reminder - x) / 3) < byte_start || (byte*)(_starts[x] + (y + _difference_reminder - x) / 3) >= byte_end)
    // printf("ACHTUNG!\n");

    return _starts[x][(y + _difference_reminder - x) / 3];
}

int TriangleLattice::getFirstValidY(int x)
{
    int y = _BORDER.min_y;
    while (!isValid(x, y))
        y++;
    return y;
}

bool TriangleLattice::isValid(int x, int y)
{
    return (y + _difference_reminder - x) % 3 == 0;
}

bool TriangleLattice::isIncreaseForValidY(int y)
{
    return !_BORDER.empty && y <= _BORDER.max_y;
}

int TriangleLattice::getFirstValidX()
{
    return _BORDER.min_x;
}

bool TriangleLattice::isIncreaseForValidX(int x)
{
    return !_BORDER.empty && x <= _BORDER.max_x;
}

void TriangleLattice::switchNextY(int& y)
{
    y += 3;
}

// AnswerField definition

IMPL_ERROR(AnswerField, "answer_field");

CP_DEF(AnswerField);

AnswerField::AnswerField(int len, int target_x, int target_y, float target_rotation, int* vertex_weight_link, int* vertex_stereo_link, int* edge_stereo_link)
    : CP_INIT, TL_CP_GET(_vertex_weight), // tree size
      TL_CP_GET(_vertex_stereo),          // there is an angle in the vertex
      TL_CP_GET(_edge_stereo),            // trans-cis configuration
      TL_CP_GET(_rotation_parity), TL_CP_GET(_coord_diff_reminder), TL_CP_GET(_lattices), TL_CP_GET(_hidden_data_field_array)
{
    length = len;

    _vertex_weight.clear_resize(length);
    for (int i = 0; i < length; i++)
        _vertex_weight[i] = vertex_weight_link[i];

    _vertex_stereo.clear_resize(length);
    for (int i = 0; i < length; i++)
        _vertex_stereo[i] = vertex_stereo_link[i];

    _edge_stereo.clear_resize(length);
    for (int i = 0; i < length; i++)
        _edge_stereo[i] = edge_stereo_link[i];

    _rotation_parity.clear_resize(length + 1);
    _rotation_parity[0] = 0;
    for (int i = 0; i < length; i++)
    {
        if (_vertex_stereo[i])
            _rotation_parity[i + 1] = _rotation_parity[i] ^ 1;
        else
            _rotation_parity[i + 1] = _rotation_parity[i];
    }

    _coord_diff_reminder.clear_resize(length + 1);
    _coord_diff_reminder[0] = 0;
    for (int i = 1; i <= length; i++)
    {
        if (_rotation_parity[i])
            _coord_diff_reminder[i] = (_coord_diff_reminder[i - 1] + 2) % 3;
        else
            _coord_diff_reminder[i] = (_coord_diff_reminder[i - 1] + 1) % 3;
    }

    ObjArray<Array<rectangle>> border_sample_array;
    border_sample_array.clear();
    for (int i = 0; i <= length; i++)
    {
        border_sample_array.push().clear_resize(2 * i + 1);
    }
    Array<rectangle*> border_sample;
    border_sample.clear_resize(length + 1);
    for (int i = 0; i <= length; i++)
    {
        border_sample[i] = border_sample_array[i].ptr() + i;
    }

    border_sample[0][0].set(0, 0, 0, 0);

    for (int l = 0; l <= length; l++)
    {
        for (int rot = -l; rot <= l; rot++)
        {
            int radius = l - abs(rot) + 1;
            int center_x = rot > 0 ? -1 : 0;
            int center_y = rot > 0 ? 1 : -1;

            border_sample[l][rot].set(rectangle::square(0, 0, l).intersec(rectangle::square(center_x, center_y, radius)));
        }
    }

    border_array.clear();
    for (int i = 0; i <= length; i++)
    {
        border_array.push().clear_resize(2 * i + 1);
    }
    border.clear_resize(length + 1);
    for (int i = 0; i <= length; i++)
    {
        border[i] = border_array[i].ptr() + i;
    }

    for (int l = 0; l <= length; l++)
    {
        for (int rot = -l; rot <= l; rot++)
        {
            border[l][rot].set(border_sample[l][rot]);

            if (l >= ACCEPTABLE_ERROR)
            {
                if (abs(rot) <= length + ACCEPTABLE_ERROR - l)
                    border[l][rot].intersec(border_sample[length + ACCEPTABLE_ERROR - l][rot].shift(target_x, target_y));
                else
                    border[l][rot].set_empty();
            }

            // if (border[l][rot].empty) printf("========== [%d, %d]x[%d, %d]\n", border[l][rot].min_x, border[l][rot].max_x, border[l][rot].min_y,
            // border[l][rot].max_y);
        }
    }

    int global_size = 0;

    for (int l = 0; l <= length; l++)
    {
        for (int rot = -l; rot <= l; rot++)
            if (abs(rot) % 2 == _rotation_parity[l])
            {
                for (int p = 0; p < 2; p++)
                {
                    global_size += TriangleLattice::getAllocationSize(border[l][rot]);
                }
            }
    }

    // printf("Global Size = %d bytes \n", global_size);

    _hidden_data_field_array.clear_resize(global_size);

    uint8_t* free_area = _hidden_data_field_array.ptr();

    _lattices.clear();

    for (int l = 0; l <= length; l++)
    {
        _lattices.push();
        for (int rot = -l; rot <= l; rot++)
        {
            _lattices.top().push();
            for (int p = 0; p < 2; p++)
            {
                if (abs(rot) % 2 == _rotation_parity[l])
                {
                    _lattices.top().top().push(border[l][rot], _coord_diff_reminder[l], free_area);
                    free_area += TriangleLattice::getAllocationSize(border[l][rot]);
                }
                else
                    _lattices.top().top().push();
            }
        }
    }

    _sink_lattice.init_void();
}

int AnswerField::_cmp_answer_points(answer_point& p1, answer_point& p2, void* context)
{
    AnswerField& fld = *((AnswerField*)context);
    return p1.quality(fld) - p2.quality(fld);
}

unsigned short& AnswerField::get_field(int len, answer_point p)
{
    return getLattice(len, p.rot, p.p).getCell(p.x, p.y);
};
unsigned short& AnswerField::get_field(answer_point p)
{
    return get_field(length, p);
};

TriangleLattice& AnswerField::getLattice(int l, int rot, int p)
{
    if (l < 0 || l > length || rot < -l || rot > l || !!p != p)
        return _sink_lattice;

    return _lattices[l][rot + l][p];
}

void AnswerField::fill()
{
    for (int l = 0; l <= length; l++)
    {
        for (int rot = -l; rot <= l; rot++)
        {
            for (int p = 0; p < 2; p++)
            {
                TriangleLattice& lat = getLattice(l, rot, p);
                for (int x = lat.getFirstValidX(); lat.isIncreaseForValidX(x); x++)
                {
                    for (int y = lat.getFirstValidY(x); lat.isIncreaseForValidY(y); lat.switchNextY(y))
                        lat.getCell(x, y) = SHORT_INFINITY;
                }
            }
        }
    }

    // getLattice(0, 0, 0).getCell(0, 0) = 0;
    getLattice(0, 0, 1).getCell(0, 0) = 0;

    for (int l = 0; l < length; l++)
    {
        for (int rot = -l; rot <= l; rot++)
        {
            for (int p = 0; p < 2; p++)
            {
                bool can[3];
                can[0] = can[1] = can[2] = false;
                bool* acceptable_rotation = &can[1];

                if (_vertex_stereo[l])
                {
                    int current_edge_stereo = _edge_stereo[(l - 1 + length) % length];
                    if (current_edge_stereo == 0)
                        acceptable_rotation[-1] = acceptable_rotation[1] = true;
                    else if ((current_edge_stereo == MoleculeCisTrans::TRANS) ^ (p == 0))
                        acceptable_rotation[-1] = true;
                    else
                        acceptable_rotation[1] = true;
                }
                else
                    acceptable_rotation[0] = true;

                for (int chenge_rotation = -1; chenge_rotation <= 1; chenge_rotation++)
                    if (acceptable_rotation[chenge_rotation])
                    {
                        int newp = chenge_rotation == 0 ? p : chenge_rotation == 1 ? 1 : 0;
                        int next_rot = rot + chenge_rotation;
                        TriangleLattice& donor = getLattice(l, rot, p);
                        TriangleLattice& retsepient = getLattice(l + 1, next_rot, newp);

                        int xchenge = getDx(next_rot);
                        int ychenge = getDy(next_rot);

                        unsigned short add = get_weight(_vertex_weight[l], newp);
                        //               unsigned short add = max(0, _vertex_weight[l] * (newp ? -1 : 1));

                        // add += (p && chenge_rotation > 0) || (!p && chenge_rotation < 0);
                        // add += p == newp;

                        for (int x = donor.getFirstValidX(); donor.isIncreaseForValidX(x); x++)
                        {
                            for (int y = donor.getFirstValidY(x); donor.isIncreaseForValidY(y); donor.switchNextY(y))
                            {
                                unsigned short& c = retsepient.getCell(x + xchenge, y + ychenge);
                                unsigned short& d = donor.getCell(x, y);

                                if (d < SHORT_INFINITY && c > (unsigned short)(d + add))
                                    c = (unsigned short)(d + add);
                                // c = min(c, (unsigned short) (donor.getCell(x, y) + add));
                            }
                        }
                    }
            }
        }
    }
}

// CP_DEF(MoleculeLayoutMacrocyclesLattice::CycleLayout);

MoleculeLayoutMacrocyclesLattice::CycleLayout::CycleLayout()
    : CP_INIT, TL_CP_GET(point), TL_CP_GET(rotate), TL_CP_GET(external_vertex_number), TL_CP_GET(edge_length)
{
}

void MoleculeLayoutMacrocyclesLattice::initCycleLayout(CycleLayout& cl)
{
    cl.external_vertex_number.clear_resize(0);
    cl.external_vertex_number.push(0);

    for (int i = 1; i < length; i++)
        if (_vertex_stereo[i])
            cl.external_vertex_number.push(i);

    cl.external_vertex_number.push(length);

    cl.vertex_count = cl.external_vertex_number.size() - 1;

    cl.edge_length.clear_resize(cl.vertex_count);

    for (int i = 0; i < cl.vertex_count; i++)
        cl.edge_length[i] = cl.external_vertex_number[i + 1] - cl.external_vertex_number[i];
}

void MoleculeLayoutMacrocyclesLattice::CycleLayout::init(answer_point* ext_point)
{
    rotate.clear_resize(vertex_count + 1);
    for (int i = 0; i < vertex_count; i++)
        rotate[i] = ext_point[external_vertex_number[i + 1]].rot - ext_point[external_vertex_number[i]].rot;

    if (ext_point[0].p == 1)
        rotate[0] = 1;
    else
        rotate[0] = -1;
    rotate[vertex_count] = rotate[0];

    point.clear_resize(vertex_count + 1);
    for (int i = 0; i <= vertex_count; i++)
    {
        point[i] = Vec2f(_2FLOAT(ext_point[external_vertex_number[i]].y), 0.f);
        point[i].rotate(_2FLOAT(M_PI / 3.));
        point[i] += Vec2f(_2FLOAT(ext_point[external_vertex_number[i]].x), 0.f);
    }
    /*for (int i = 0; i < vertex_count; i++) {
        point[i] = Vec2f(vertex_count / (2 * M_PI), 0);
        point[i].rotate(2*M_PI * i / vertex_count);
    }*/
}

void MoleculeLayoutMacrocyclesLattice::CycleLayout::init(int* up_point)
{
    rotate.clear_resize(vertex_count + 1);
    for (int i = 0; i < vertex_count; i++)
    {
        if (up_point[external_vertex_number[i]])
            rotate[i] = 1;
        else if (up_point[external_vertex_number[(i - 1 + vertex_count) % vertex_count]] ||
                 up_point[external_vertex_number[(i + 1 + vertex_count) % vertex_count]])
            rotate[i] = -1;
        else
            rotate[i] = 1;
    }

    rotate[vertex_count] = rotate[0];

    point.clear_resize(vertex_count + 1);
    int length = external_vertex_number[vertex_count];
    float r = _2FLOAT(_2DOUBLE(length) / 2. / M_PI);
    for (int i = 0; i <= vertex_count; i++)
    {
        point[i] = Vec2f(r + _2FLOAT(up_point[external_vertex_number[i]]), 0.f);
        point[i].rotate(_2FLOAT(2. * M_PI * external_vertex_number[i] / length));
    }
}

float MoleculeLayoutMacrocyclesLattice::preliminary_layout(CycleLayout& cl)
{
    QS_DEF(ObjArray<ObjArray<Array<bool>>>, can);

    can.clear();
    int maxrot = 19;    // |[0, 18]|
    int mask_count = 8; // 1 << 3
    for (int i = 0; i <= length; i++)
    {
        can.push();
        can.top().clear();
        for (int j = 0; j < maxrot; j++)
        {
            can.top().push();
            can.top().top().clear_resize(mask_count);
        }
    }

    bool is_pos_rotate[8];
    for (int i = 0; i < 8; i++)
        is_pos_rotate[i] = i & 2;
    is_pos_rotate[0] = true;
    int is_cis[16];
    for (int i = 0; i < 16; i++)
        is_cis[i] = (is_pos_rotate[i & 7] == is_pos_rotate[i >> 1]) ? MoleculeCisTrans::CIS : MoleculeCisTrans::TRANS;

    int best_rot = -1;
    QS_DEF(Array<int>, up);
    up.clear_resize(length + 1);
    up.zerofill();

    for (int start_mask = 0; start_mask < mask_count; start_mask++)
    {

        for (int i = 0; i <= length; i++)
        {
            for (int j = 0; j < maxrot; j++)
            {
                for (int k = 0; k < mask_count; k++)
                    can[i][j][k] = false;
            }
        }

        can[0][6][start_mask] = true;
        int curr_mask;

        for (int i = 0; i < length; i++)
        {
            for (int j = 0; j < maxrot; j++)
            {
                for (int mask = 0; mask < mask_count; mask++)
                    if (can[i][j][mask])
                    {
                        for (int up = 0; up <= 1; up++)
                        {
                            int previ = i ? i - 1 : length - 1;
                            if (_edge_stereo[previ] == is_cis[curr_mask = ((mask << 1) + up)] || _edge_stereo[previ] == 0)
                            {
                                int rot = j;
                                if (is_pos_rotate[curr_mask & 7])
                                    rot++;
                                else
                                    rot--;
                                if (rot >= 0 && rot < maxrot)
                                    can[i + 1][rot][curr_mask & 7] = true;
                            }
                        }
                    }
            }
        }

        for (int rot = 0; rot < maxrot; rot++)
            if (can[length][rot][start_mask])
            {
                if (abs(best_rot - 12) > abs(rot - 12))
                {
                    best_rot = rot;
                    curr_mask = start_mask;
                    int curr_rot = rot;
                    for (int i = length - 1;; i--)
                    {
                        up[i + 1] = curr_mask & 1;
                        if (i < 0)
                            break;
                        for (int mask = curr_mask + 8; mask >= curr_mask; mask -= 8)
                        {
                            int newrot = curr_rot;

                            if (is_pos_rotate[mask & 7])
                                newrot--;
                            else
                                newrot++;
                            int previ = i ? i - 1 : length - 1;
                            if (_edge_stereo[previ] == 0 || _edge_stereo[previ] == is_cis[mask])
                            {
                                if (can[i][newrot][mask >> 1])
                                {
                                    curr_rot = newrot;
                                    curr_mask = mask >> 1;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
    }

    cl.init(up.ptr());
    smoothing(cl);

    for (int i = 0, j = 0; i < cl.vertex_count; i++)
        for (int t = cl.external_vertex_number[i]; t < cl.external_vertex_number[i + 1]; t++, j++)
        {
            _positions[j] = cl.point[i] + (cl.point[i + 1] - cl.point[i]) * _2FLOAT(t - cl.external_vertex_number[i]) / _2FLOAT(cl.edge_length[i]);
        }

    return rating(cl);
}

int MoleculeLayoutMacrocyclesLattice::internalValue(CycleLayout& cl)
{
    int val = 0;

    for (int i = 0; i < cl.vertex_count; i++)
        val += get_weight(_vertex_weight[cl.external_vertex_number[i]], cl.rotate[i]);

    return val;
}

float MoleculeLayoutMacrocyclesLattice::CycleLayout::area()
{
    float value = 0;
    for (int i = 1; i < vertex_count - 1; i++)
        value += Vec2f::cross(point[i] - point[0], point[(i + 1) % vertex_count] - point[0]) / 2;

    return fabs(value);
}

float MoleculeLayoutMacrocyclesLattice::CycleLayout::perimeter()
{
    float perimeter = 0;
    for (int i = 0; i < vertex_count; i++)
        perimeter += (point[(i + 1) % vertex_count] - point[i]).length();
    return perimeter;
}

bool MoleculeLayoutMacrocyclesLattice::is_period(CycleLayout& cl, int k)
{
    if (cl.vertex_count % k != 0)
        return false;
    int len = cl.vertex_count / k;
    for (int i = 0; i + len < cl.vertex_count; i++)
        if (cl.rotate[i] != cl.rotate[i + len])
            return false;
    for (int i = 0; i + len < cl.vertex_count; i++)
        if (cl.edge_length[i] != cl.edge_length[i + len])
            return false;
    return true;
}

int MoleculeLayoutMacrocyclesLattice::period(CycleLayout& cl)
{
    int answer = 1;
    if (is_period(cl, 2))
    {
        answer = 2;
        if (is_period(cl, 4))
            answer = 4;
    }
    if (is_period(cl, 3))
        answer *= 3;

    return answer;
}

float MoleculeLayoutMacrocyclesLattice::rating(CycleLayout& cl)
{
    const float eps = 1e-9f;
    float result = 0;
    int add = 0;
    // distances
    for (int i = 0; i < cl.vertex_count; i++)
    {
        float len = Vec2f::dist(cl.point[i], cl.point[(i + 1) % cl.vertex_count]) / cl.edge_length[i];
        if (len < eps)
            add++;
        else if (len < 1)
            result = max(result, (1 / len - 1));
        else
            result = max(result, len - 1);
    }
    // angles
    for (int i = 0; i < cl.vertex_count; i++)
    {
        Vec2f vp1 = cl.point[(i + 1) % cl.vertex_count] - cl.point[i];
        Vec2f vp2 = cl.point[(i + cl.vertex_count - 1) % cl.vertex_count] - cl.point[i];
        float len1 = vp1.length();
        float len2 = vp2.length();
        vp1 /= len1;
        vp2 /= len2;

        float angle = acos(Vec2f::dot(vp1, vp2));
        if (Vec2f::cross(vp2, vp1) > 0)
            angle = -angle;
        angle /= _target_angle[cl.external_vertex_number[i]];
        if (angle * cl.rotate[i] <= 0)
            add += 1000;
        float angle_badness = fabs((((abs(angle) > 1) ? angle : 1 / angle) - cl.rotate[i]) / 2) * _angle_importance[cl.external_vertex_number[i]];
        result = max(result, angle_badness);
    }

    vector<Vec2f> pp;
    for (int i = 0; i < cl.vertex_count; i++)
        for (int a = cl.external_vertex_number[i], t = 0; a != cl.external_vertex_number[(i + 1) % cl.vertex_count]; a = (a + 1) % length, t++)
        {
            pp.push_back((cl.point[i] * _2FLOAT(cl.edge_length[i] - t) + cl.point[(i + 1) % cl.vertex_count] * _2FLOAT(t)) / _2FLOAT(cl.edge_length[i]));
        }

    int size = pp.size();
    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
            if (i != j && (i + 1) % size != j && i != (j + 1) % size)
            {
                int nexti = (i + 1) % size;
                int nextj = (j + 1) % size;
                float dist = Vec2f::distSegmentSegment(pp[i], pp[nexti], pp[j], pp[nextj]);

                if (fabs(dist) < eps)
                {
                    add++;
                    // printf("%5.5f %5.5f %5.5f %5.5f %5.5f %5.5f %5.5f %5.5f \n", xx[i], yy[i], xx[nexti], yy[nexti], xx[j], yy[j], xx[nextj], yy[nextj]);
                }
                else if (dist < 1)
                    result = max(result, 1 / dist - 1);
            }
    // tails

    float diff = _2FLOAT(internalValue(cl));

    result += _2FLOAT(1.0 * diff / length);

    float area = cl.area();
    QS_DEF(Array<Vec2f>, current_point);
    current_point.clear_resize(length);
    for (int i = 0, t = 0; i < cl.vertex_count; i++)
        for (int j = cl.external_vertex_number[i], d = 0; j < cl.external_vertex_number[i + 1]; j++, t++, d++)
            current_point[t] = cl.point[i] + (cl.point[i + 1] - cl.point[i]) * _2FLOAT(d);

    for (int i = 0; i < cl.vertex_count; i++)
        if ((_component_finish[cl.external_vertex_number[i]] != cl.external_vertex_number[i]) &&
            ((_component_finish[cl.external_vertex_number[i]] - cl.external_vertex_number[i] + cl.vertex_count) % cl.vertex_count <= cl.vertex_count / 4) &&
            (cl.rotate[i] == -1))
            area += _vertex_added_square[cl.external_vertex_number[i]] *
                    (current_point[_component_finish[cl.external_vertex_number[i]]] - current_point[cl.external_vertex_number[i]]).lengthSqr();

    float perimeter = cl.perimeter();

    result += _2FLOAT((perimeter * perimeter / 4. / M_PI / area - 1.) / 5.);

    result += 1000.0f * add;

    int p = period(cl);
    result *= 1.0f / p;

    return result;
}

void MoleculeLayoutMacrocyclesLattice::CycleLayout::soft_move_vertex(int vertex_number, Vec2f move_vector)
{
    int i = vertex_number;
    float count = _2FLOAT(vertex_count);
    Vec2f shift_vector = move_vector;
    Vec2f add_vector = move_vector * _2FLOAT(-1.0 / vertex_count);
    float mult = 1.f;
    float add_mult = _2FLOAT(-1.0 / vertex_count);
    do
    {
        point[i++] += move_vector * (count / vertex_count);
        count -= 1;
        // p[i++] += shift_vector;
        // shift_vector += add_vector;
        // p[i++] += move_vector * mult;
        // mult += add_mult;
        // shift_vector = move_vector * mult;
        if (i == vertex_count)
            i = 0;
    } while (i != vertex_number);

    point[vertex_count].copy(point[0]);
}

void MoleculeLayoutMacrocyclesLattice::CycleLayout::stright_rotate_chein(int vertex_number, float angle)
{
    for (int i = 0; i <= vertex_count; i++)
        if (i != vertex_number)
            point[i] -= point[vertex_number];
    point[vertex_number].set(0, 0);

    for (int i = vertex_number + 1; i <= vertex_count; i++)
        point[i].rotate(angle);
}

void MoleculeLayoutMacrocyclesLattice::CycleLayout::stright_move_chein(int vertex_number, Vec2f vector)
{
    for (int i = vertex_number; i <= vertex_count; i++)
        point[i] += vector;
}

void MoleculeLayoutMacrocyclesLattice::closingStep(CycleLayout& cl, int index, int base_vertex, bool fix_angle, bool fix_next, float multiplyer)
{

    int prev_vertex = base_vertex - 1;
    int next_vertex = base_vertex + 1;
    if ((cl.point[0] - cl.point[cl.vertex_count]).lengthSqr() == 0)
    {
        if (next_vertex == cl.vertex_count)
            next_vertex = 0;
        if (prev_vertex == -1)
            prev_vertex = cl.vertex_count - 1;
    }
    int move_vertex = fix_next ? next_vertex : prev_vertex;

    Vec2f move_vector(0, 0);

    if (fix_angle)
    {

        if ((cl.point[prev_vertex] - cl.point[base_vertex]).length() < 2 * EPSILON || (cl.point[next_vertex] - cl.point[base_vertex]).length() < 2 * EPSILON)
            return;
        float current_angle = cl.point[base_vertex].calc_angle(cl.point[next_vertex], cl.point[prev_vertex]);
        while (current_angle > _2FLOAT(2. * M_PI))
            current_angle -= _2FLOAT(2. * M_PI);
        while (current_angle < 0.f)
            current_angle += _2FLOAT(2. * M_PI);

        float current_target_angle = _target_angle[cl.external_vertex_number[base_vertex]];
        if (cl.rotate[base_vertex] < 0)
            current_target_angle = _2FLOAT(2. * M_PI - current_target_angle);
        float anti_current_target_angle;
        if (current_target_angle > M_PI)
        {
            if (current_angle > current_target_angle)
                anti_current_target_angle = _2FLOAT(2. * M_PI);
            else
                anti_current_target_angle = _2FLOAT(M_PI);
        }
        else
        {
            if (current_angle < current_target_angle)
                anti_current_target_angle = 0.f;
            else
                anti_current_target_angle = _2FLOAT(M_PI);
        }

        float better_change_angle = 0;
        float worse_chenge_angle = 0;
        if (fabs(current_angle - current_target_angle) < EPSILON)
        {
            better_change_angle = current_angle * multiplyer;
            worse_chenge_angle = -current_angle * multiplyer;
        }
        else
        {
            better_change_angle = (current_target_angle - current_angle) * multiplyer;
            worse_chenge_angle = (anti_current_target_angle - current_angle) * multiplyer;
        }

        float actual_chenge_angle = 0;

        /*if ((cl.point[0] - cl.point[cl.vertex_count]).lengthSqr() == 0) {

           if (fabs(current_angle - current_target_angle) < EPSILON) actual_chenge_angle = 0;
           else actual_chenge_angle = better_change_angle;

           if (fix_next) actual_chenge_angle *= -1;

           //actual_chenge_angle *= _angle_importance[vertex_number[base_vertex]];

           move_vector.rotateAroundSegmentEnd(cl.point[move_vertex], cl.point[base_vertex], actual_chenge_angle);
           move_vector -= cl.point[move_vertex];

           if (!fix_next) move_vector.negate();

           if (fix_next) cl.soft_move_vertex(move_vertex, move_vector);
           else cl.soft_move_vertex(base_vertex, move_vector);
        }
        else*/
        {
            float angle = current_angle;

            for (int i = next_vertex; i < cl.vertex_count; i++)
                angle -= cl.point[base_vertex].calc_angle(cl.point[i], cl.point[i + 1]);
            for (int i = prev_vertex; i > 0; i--)
                angle += cl.point[base_vertex].calc_angle(cl.point[i], cl.point[i - 1]);

            if (fabs(angle + actual_chenge_angle) > fabs(angle + better_change_angle))
                actual_chenge_angle = better_change_angle;
            if (fabs(angle + actual_chenge_angle) > fabs(angle + worse_chenge_angle))
                actual_chenge_angle = worse_chenge_angle;

            // actual_chenge_angle *= _angle_importance[vertex_number[base_vertex]];

            cl.stright_rotate_chein(base_vertex, -actual_chenge_angle);
        }
    }
    else
    {
        int first_vertex = fix_next ? base_vertex : prev_vertex;
        int second_vertex = fix_next ? next_vertex : base_vertex;
        float current_dist = Vec2f::dist(cl.point[first_vertex], cl.point[second_vertex]);
        float current_target_dist = _2FLOAT(cl.edge_length[first_vertex]);
        Vec2f better_chenge_vector = cl.point[second_vertex] - cl.point[first_vertex];
        Vec2f worse_chenge_vector = cl.point[second_vertex] - cl.point[first_vertex];

        if (fabs(current_target_dist - current_dist) > EPSILON)
        {
            better_chenge_vector *= (current_target_dist - current_dist) / current_dist * multiplyer;
            worse_chenge_vector *= (current_dist - current_target_dist) / current_dist * multiplyer;
        }
        else
        {
            better_chenge_vector *= multiplyer;
            worse_chenge_vector *= -multiplyer;
        }

        if ((cl.point[0] - cl.point[cl.vertex_count]).lengthSqr() == 0)
        {
            if (fabs(current_target_dist - current_dist) > EPSILON)
                cl.soft_move_vertex(second_vertex, better_chenge_vector);
        }
        else
        {
            Vec2f actual_chenge_vector(0, 0);

            Vec2f current_discrepancy = cl.point[cl.vertex_count] - cl.point[0];
            if ((current_discrepancy + actual_chenge_vector).lengthSqr() > (current_discrepancy + better_chenge_vector).lengthSqr())
                actual_chenge_vector = better_chenge_vector;
            if ((current_discrepancy + actual_chenge_vector).lengthSqr() > (current_discrepancy + worse_chenge_vector).lengthSqr())
                actual_chenge_vector = worse_chenge_vector;

            cl.stright_move_chein(second_vertex, actual_chenge_vector);
        }
    }
}

void MoleculeLayoutMacrocyclesLattice::closing(CycleLayout& cl)
{
    Random rand(SOME_MAGIC_INT_FOR_RANDOM_1);

    int iter_count = max(200 * cl.vertex_count, 10000);
    float multiplyer = 0.3f;

    for (int i = 0; i < iter_count; i++)
    {
        float lenSqr = (cl.point[0] - cl.point[cl.vertex_count]).lengthSqr();
        if (lenSqr < 0.25)
        {
            float angle = _2FLOAT(-M_PI * cl.vertex_count);
            for (int i = 0; i < cl.vertex_count; i++)
                angle += cl.point[i].calc_angle_pos(cl.point[(i + 1) % cl.vertex_count], cl.point[(i + cl.vertex_count - 1) % cl.vertex_count]);
            if (angle < 0)
            {
                cl.point[cl.vertex_count].copy(cl.point[0]);
                break;
            }
        }

        bool is_angle = rand.next() & 1;
        bool is_next = rand.next() & 1;
        int base_vertex = rand.next(cl.vertex_count + 1);

        if ((cl.point[0] - cl.point[cl.vertex_count]).lengthSqr() != 0)
        {
            if (is_angle && (base_vertex == 0 || base_vertex == cl.vertex_count))
                continue;
            if (!is_angle && ((base_vertex == 0 && !is_next) || (base_vertex == cl.vertex_count && is_next)))
                continue;
        }
        else
        {
            if (base_vertex == cl.vertex_count)
                continue;
        }

        closingStep(cl, i, base_vertex, is_angle, is_next, multiplyer);
        if (lenSqr == 0)
            multiplyer *= CHANGE_FACTOR;
        // if (i % 100 == 0) printf("%.5f\n", rating(cl));
    }
}

void MoleculeLayoutMacrocyclesLattice::updateTouchingPoints(Array<local_pair_id>& pairs, CycleLayout& cl)
{
    int len = cl.vertex_count;
    const float eps = 1e-4f;
    const float eps2 = eps * eps;
    float good_distance = 1.f;
    pairs.clear();

    QS_DEF(Array<Vec2f>, all_points);
    QS_DEF(Array<float>, all_numbers);
    all_points.clear();
    all_numbers.clear();
    for (int j = 0; j < len; j++)
    {
        float s;
        int t;
        for (t = cl.external_vertex_number[j], s = 0; t < cl.external_vertex_number[(j + 1) % len]; t++, s += 1.0f / cl.edge_length[j])
        {
            all_points.push(cl.point[j] * (1 - s) + cl.point[j] * s);
            all_numbers.push(j + s);
        }
    }

    for (int i = 0; i < len; i++)
    {
        for (int j = 0; j < all_points.size(); j++)
        {
            int diff = (i - (int)all_numbers[j] + len) % len;
            if (diff > 1 && diff != len - 1)
            {
                float distSqr = (cl.point[i] - all_points[j]).lengthSqr();
                if (eps2 < distSqr && distSqr < good_distance)
                {
                    pairs.push(local_pair_id(i, all_numbers[j]));
                }
            }
        }
    }
}

void MoleculeLayoutMacrocyclesLattice::smoothing(CycleLayout& cl)
{
    closing(cl);

    Random rand(SOME_MAGIC_INT_FOR_RANDOM_2);
    int iter_count = max(50 * length, 2000);

    QS_DEF(Array<local_pair_id>, touching_points);

    float coef = SMOOTHING_MULTIPLIER;
    for (int i = 0; i < iter_count; i++)
    {
        if ((i & (i - 1)) == 0)
            updateTouchingPoints(touching_points, cl);
        int current_vertex = rand.next(cl.vertex_count);
        smoothingStep(cl, current_vertex, coef *= CHANGE_FACTOR, touching_points);
    }
}

void MoleculeLayoutMacrocyclesLattice::smoothingStep(CycleLayout& cl, int vertex_number, float coef, Array<local_pair_id>& touching_points)
{
    Vec2f p1 = cl.point[(vertex_number - 1 + cl.vertex_count) % cl.vertex_count];
    Vec2f p2 = cl.point[(vertex_number + 1 + cl.vertex_count) % cl.vertex_count];
    float r1 = _2FLOAT(cl.edge_length[(cl.vertex_count + vertex_number - 1) % cl.vertex_count]);
    float r2 = _2FLOAT(cl.edge_length[(cl.vertex_count + vertex_number) % cl.vertex_count]);

    float len1 = Vec2f::dist(p1, cl.point[vertex_number]);
    float len2 = Vec2f::dist(p2, cl.point[vertex_number]);

    float r3 = _2FLOAT(Vec2f::dist(p1, p2) / sqrt(3.0));

    Vec2f p3;

    if (cl.rotate[vertex_number] != 0)
    {
        p3 = (p1 + p2) / 2;
        Vec2f a = (p2 - p1) / sqrt(12.0f);
        a.rotate(_2FLOAT(M_PI / 2. * cl.rotate[vertex_number]));
        p3 += a;
    }
    else
    {
        p3 = (p1 * r2 + p2 * r1) / (r1 + r2);
    }

    float len3 = Vec2f::dist(p3, cl.point[vertex_number]);
    if (cl.rotate[vertex_number] == 0)
        r3 = 0;

    // printf("%5.5f %5.5f %5.5f %5.5f\n", len1, len2, len3, r3);
    Vec2f newPoint;
    const float eps = 1e-4f;
    const float eps2 = eps * eps;
    if (len1 < eps || len2 < eps || len3 < eps)
    {
        cl.point[vertex_number] = (p1 + p2) / 2.0;
    }
    else
    {
        float coef1 = (r1 / len1 - 1);
        float coef2 = (r2 / len2 - 1);
        float coef3 = (r3 / len3 - 1);

        // if (!isIntersec(x[worstVertex], y[worstVertex], x3, y3, x1, y1, x2, y2)) coef3 *= 10;
        if (cl.rotate[vertex_number] == 0)
            coef3 = -1;
        // printf("%5.5f %5.5f %5.5f\n", coef1, coef2, coef3);
        newPoint += (cl.point[vertex_number] - p1) * coef1;
        newPoint += (cl.point[vertex_number] - p2) * coef2;
        newPoint += (cl.point[vertex_number] - p3) * coef3;

        float good_distance = 1;
        for (int i = 0; i < touching_points.size(); i++)
            if (touching_points[i].left == vertex_number)
            {
                int j = (int)touching_points[i].right;
                float s = touching_points[i].right - j;
                Vec2f pp = cl.point[j] * (1 - s) + cl.point[(j + 1) % cl.vertex_count] * s;
                float distSqr = Vec2f::distSqr(cl.point[vertex_number], pp);
                float dist = sqrt(distSqr);
                float coef = (good_distance - dist) / dist;
                // printf("%5.5f \n", dist);
                newPoint += (cl.point[vertex_number] - pp) * coef;
            }

        newPoint *= coef;

        cl.point[vertex_number] += newPoint;
    }
}

Vec2f MoleculeLayoutMacrocyclesLattice::CycleLayout::getWantedVector(int vertex_number)
{

    Vec2f p1 = point[(vertex_number - 1 + vertex_count) % vertex_count];
    Vec2f p2 = point[(vertex_number + 1 + vertex_count) % vertex_count];
    float r1 = _2FLOAT(edge_length[(vertex_count + vertex_number - 1) % vertex_count]);
    float r2 = _2FLOAT(edge_length[(vertex_count + vertex_number) % vertex_count]);

    float len1 = Vec2f::dist(p1, point[vertex_number]);
    float len2 = Vec2f::dist(p2, point[vertex_number]);

    float r3 = _2FLOAT(Vec2f::dist(p1, p2) / sqrt(3.0));

    Vec2f p3 = (p1 + p2) / 2;

    if (rotate[vertex_number] != 0)
    {
        Vec2f a = (p2 - p1) / sqrt(12.0f);
        a.rotate(_2FLOAT(M_PI / 2. * rotate[vertex_number]));
        p3 += a;
    }
    else
    {
        p3 = (p1 * r1 + p2 * r2) / (r1 + r2);
    }

    float len3 = Vec2f::dist(p3, point[vertex_number]);
    if (rotate[vertex_number] == 0)
        r3 = 0;

    // printf("%5.5f %5.5f %5.5f %5.5f\n", len1, len2, len3, r3);
    Vec2f newPoint;
    const float eps = 1e-4f;
    const float eps2 = eps * eps;
    // if (len1 < eps || len2 < eps || len3 < eps) return ;

    float coef1 = (r1 / len1 - 1);
    float coef2 = (r2 / len2 - 1);
    float coef3 = (r3 / len3 - 1);
    /*   if (rotate[vertex_number] != 0) {
          float angle = acos(Vec2f::cross(p1 - point[vertex_number], p2 - point[vertex_number]) / (Vec2f::dist(p1, point[vertex_number])*Vec2f::dist(p2,
       point[vertex_number])));
          //if (angle < 2 * M_PI / 3) coef3 /= 10;
       }*/

    // if (!isIntersec(x[worstVertex], y[worstVertex], x3, y3, x1, y1, x2, y2)) coef3 *= 10;
    if (rotate[vertex_number] == 0)
        coef3 = -1;
    // printf("%5.5f %5.5f %5.5f\n", coef1, coef2, coef3);
    newPoint += (point[vertex_number] - p1) * coef1;
    newPoint += (point[vertex_number] - p2) * coef2;
    newPoint += (point[vertex_number] - p3) * coef3;

    return newPoint * SMOOTHING_MULTIPLIER;
}