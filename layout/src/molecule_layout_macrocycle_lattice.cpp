/****************************************************************************
* Copyright (C) 2009-2011 GGA Software Services LLC
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

#include "layout/molecule_layout_macrocycles.h"

#include "base_cpp/profiling.h"
#include <limits.h>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <stack>
#include <cmath>
#include <string>
#include <sstream>
#include <map>
#include <stdio.h>
#include <math/random.h>
#include "layout/molecule_layout.h"
#include <algorithm>

using namespace std;
using namespace indigo;

IMPL_ERROR(MoleculeLayoutMacrocyclesLattice, "molecule_layout_macrocycles_lattice");

const double MoleculeLayoutMacrocyclesLattice::CHANGE_FACTOR = 0.999;
const double MoleculeLayoutMacrocyclesLattice::SMOOTHING_MULTIPLIER = 0.1;


CP_DEF(MoleculeLayoutMacrocyclesLattice);
CP_DEF(MoleculeLayoutMacrocyclesLattice::CycleLayout);


static const int SIX = 6;
static const int MAX_ROT = 100; // rotation can be negative. We must add MAX_ROT * SIX to it for correct reminder obtaining

static const int dx[6] = { 1, 0, -1, -1, 0, 1 };
static const int dy[6] = { 0, 1, 1, 0, -1, -1 };

static const int getDx(int x) { return dx[(x + SIX * MAX_ROT) % SIX]; }
static const int getDy(int y) { return dy[(y + SIX * MAX_ROT) % SIX]; }

MoleculeLayoutMacrocyclesLattice::MoleculeLayoutMacrocyclesLattice(int size) :
CP_INIT,
TL_CP_GET(_vertex_weight), // tree size
TL_CP_GET(_vertex_stereo), // there is an angle in the vertex
TL_CP_GET(_edge_stereo), // trans-cis configuration
TL_CP_GET(_positions),
TL_CP_GET(_angle_importance),
TL_CP_GET(_component_finish),
TL_CP_GET(_target_angle),
TL_CP_GET(_vertex_added_square),
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
   for (int i = 0; i < size; i++) _component_finish[i] = i;

   _target_angle.clear_resize(size);
   _target_angle.zerofill();

   _vertex_added_square.clear_resize(size);
   _vertex_drawn.clear_resize(size);


}

void MoleculeLayoutMacrocyclesLattice::doLayout() {
   calculate_rotate_length();

   rotate_cycle(rotate_length);
   AnswerField answfld(length, 0, 0, 0, _vertex_weight.ptr(), _vertex_stereo.ptr(), _edge_stereo.ptr());


   answfld.fill();

   QS_DEF(Array<answer_point>, points);
   points.clear_resize(0);

   for (int rot = -length; rot <= length; rot++) {
      for (int p = 0; p < 2; p++) {
         TriangleLattice& lat = answfld.getLattice(length, rot, p);
         for (int x = lat.getFirstValidX(); lat.isIncreaseForValidX(x); x++) {
            for (int y = lat.getFirstValidY(x); lat.isIncreaseForValidY(y); lat.switchNextY(y)) {
               if (lat.getCell(x, y) < SHORT_INFINITY) {
                  answer_point point(rot, p, x, y);
                  points.push(point);
               }
            }
         }
      }
   }



   _positions.clear_resize(length + 1);

   //printf("%d\n", points.size());

   points.qsort(&AnswerField::_cmp_answer_points, &answfld);

   int count = 0;
   int best_number = -1;
   double best_rating = 1e30;

   Array<answer_point> path;
   path.clear_resize(length + 1);
   printf("%d\n", points.size());
   for (int i = 0; i < 100 && i < points.size() && count < 10; i++) {
      answfld._restore_path(path.ptr(), points[i]);
      CycleLayout cl; initCycleLayout(cl); cl.init(path.ptr());
      smoothing(cl);

      double current_rating = rating(cl);

      if (current_rating + EPSILON < best_rating) {
//         for (int i = 0; i <= length; i++) printf("%d %d %d %d\n", path[i].x, path[i].y, path[i].rot, path[i].p);
         count = 0;
         best_rating = current_rating;
         best_number = i;
//         printf("%d %d %d %d %.5f\n", best_number, points[best_number].x, points[best_number].y, points[best_number].rot, best_rating);

      } else count++;
   }

      answfld._restore_path(path.ptr(), points[best_number]);
      CycleLayout cl; initCycleLayout(cl); cl.init(path.ptr());
      smoothing(cl);

      for (int i = 0, j = 0; i < cl.vertex_count; i++)
      for (int t = cl.external_vertex_number[i]; t < cl.external_vertex_number[i + 1]; t++, j++)
         _positions[j] = cl.point[i] + (cl.point[i + 1] - cl.point[i]) * (t - cl.external_vertex_number[i]) / cl.edge_length[i];
   }


   rotate_cycle(-rotate_length);
}


void MoleculeLayoutMacrocyclesLattice::calculate_rotate_length() {

   rotate_length = 0;
   int max_value = -SHORT_INFINITY;

   for (int i = 0; i < length; i++) {
      if (_edge_stereo[i] != 2) {
         int value = _edge_stereo[i]
            + _vertex_weight[i] + _vertex_weight[(i + 1) % length]
            - _vertex_weight[(i + length - 1) % length] / 2 - _vertex_weight[(i + 2) % length] / 2;

         if (rotate_length == -1 || value > max_value) {
            rotate_length = i;
            max_value = value;
         }
      }
   }
   rotate_length++;
}

void MoleculeLayoutMacrocyclesLattice::rotate_cycle(int shift) {
   shift = (shift % length + length) % length;
   QS_DEF(Array<int>, temp);
   temp.clear_resize(length);

   for (int i = shift; i < length; i++) temp[i - shift] = _vertex_weight[i];
   for (int i = 0; i < shift; i++) temp[i - shift + length] = _vertex_weight[i];
   for (int i = 0; i < length; i++) _vertex_weight[i] = temp[i];

   for (int i = shift; i < length; i++) temp[i - shift] = _vertex_stereo[i];
   for (int i = 0; i < shift; i++) temp[i - shift + length] = _vertex_stereo[i];
   for (int i = 0; i < length; i++) _vertex_stereo[i] = temp[i];

   for (int i = shift; i < length; i++) temp[i - shift] = _edge_stereo[i];
   for (int i = 0; i < shift; i++) temp[i - shift + length] = _edge_stereo[i];
   for (int i = 0; i < length; i++) _edge_stereo[i] = temp[i];

   QS_DEF(Array<Vec2f>, temp_point);
   temp_point.clear_resize(length);

   for (int i = shift; i < length; i++) temp_point[i - shift] = _positions[i];
   for (int i = 0; i < shift; i++) temp_point[i - shift + length] = _positions[i];
   for (int i = 0; i < length; i++) _positions[i] = temp_point[i];

}

Vec2f &MoleculeLayoutMacrocyclesLattice::getPos(int v) const
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

void MoleculeLayoutMacrocyclesLattice::setVertexAddedSquare(int v, double s) {
   _vertex_added_square[v] = s;
}

void MoleculeLayoutMacrocyclesLattice::setVertexDrawn(int v, bool drawn) {
   _vertex_drawn[v] = drawn;
}

void MoleculeLayoutMacrocyclesLattice::setComponentFinish(int v, int f) {
   _component_finish[v] = f;
}

void MoleculeLayoutMacrocyclesLattice::setTargetAngle(int v, double angle) {
   _target_angle[v] = angle;
}

void MoleculeLayoutMacrocyclesLattice::setAngleImportance(int v, double imp) {
   _angle_importance[v] = imp;
}

void AnswerField::_restore_path(answer_point* path, answer_point finish) {

   path[length] = finish;

   for (int len = length - 1; len >= 0; len--) {
//      printf("len = %d, x = %d, y = %d, p = %d, rot = %d, value = %d \n", len + 1, path[len + 1].x, path[len + 1].y, path[len + 1].p, path[len + 1].rot, get_field(len + 1, path[len + 1]));
      path[len] = path[len + 1];
      path[len].x -= getDx(path[len + 1].rot);
      path[len].y -= getDy(path[len + 1].rot);

      if (_vertex_stereo[len]) {
         path[len].rot -= path[len + 1].p ? 1 : -1;
         path[len].p ^= 1;

         int rot = path[len + 1].rot;

         int add = max(0, _vertex_weight[len] * (path[len + 1].p ? -1 : 1));

         // choosing rotation closer to circle
         double l = len * (sqrt(3.0) + 1.5) * PI / 12;

         Vec2f vec(path[len].y, 0);
         vec.rotate(PI / 3);
         vec += Vec2f(path[len].x, 0);
         double x = vec.length();

         double eps = 1e-3;

         double alpha = 2 * PI;
         if (x > eps) {

            double L = eps;
            double R = 2 * PI - eps;

            while (R - L > eps) {
               double M = (L + R) / 2;
               if (M * x / (2 * sin(M / 2)) > l) R = M;
               else L = M;
            }

            alpha = vec.tiltAngle2() + R / 2;

         }

         int preferred_p = alpha > PI / 3 * (path[len].rot) + PI / 6 / length;

         path[len].p = preferred_p ^ 1;

         // enumerating two cases
         for (int i = 0; i < 2; i++) {
            if (get_field(len + 1, path[len + 1]) == add + 
               //(path[len].p == path[len + 1].p) + 
               get_field(len, path[len])) break;
//            printf("----| len = %d, x = %d, y = %d, p = %d, rot = %d, value = %d \n", len + 1, path[len + 1].x, path[len + 1].y, path[len + 1].p, path[len + 1].rot, get_field(len + 1, path[len + 1]));
  //          printf("----| len = %d, x = %d, y = %d, p = %d, rot = %d, value = %d \n", len, path[len].x, path[len].y, path[len].p, path[len].rot, get_field(len, path[len]));
            path[len].p ^= 1;
         }
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

TriangleLattice::TriangleLattice(rectangle rec, int rem, byte* data_link) {
   init(rec, rem, data_link);
}

void TriangleLattice::init(rectangle rec, int rem, byte* data_link)
{
   _BORDER = rec;

   _difference_reminder = rem;

   if (_BORDER.empty) return;

   //byte_start = data_link + sizeof(unsigned short*) * (_BORDER.max_x - _BORDER.min_x + 1);
   //byte_end = data_link + TriangleLattice::getAllocationSize(rec);

   _starts = (unsigned short**)data_link;

   _starts[0] = (unsigned short*)(_starts + _BORDER.max_x - _BORDER.min_x + 1);
   _starts -= _BORDER.min_x;

   for (int x = _BORDER.min_x; x < _BORDER.max_x; x++) {
      int left = _BORDER.min_y;
      int right = _BORDER.max_y;

      while (!isValid(x, left)) left++;
      while (!isValid(x, right)) right--;

      _starts[x + 1] = _starts[x] + (right - left + 3)/3;
   }

   for (int x = _BORDER.min_x; x <= _BORDER.max_x; x++) {
      _starts[x] -= (getFirstValidY(x) + _difference_reminder - x) / 3;
   }
}

void TriangleLattice::init_void() {
   _BORDER.set_empty();
   _difference_reminder = 0;
}



unsigned short& TriangleLattice::getCell(int x, int y) {
   if (_BORDER.empty) return _sink;

#if defined(DEBUG) || defined(_DEBUG) 
   if (!isValid(x, y)) throw Error("difference of coordinates reminder is failed: x = %d, y = %d, rem = %d", x, y, _difference_reminder);
   if (!_BORDER.expand(3).contains(x, y)) throw Error("point (%d, %d) is not close to framework [%d, %d]x[%d, %d].", x, y, _BORDER.min_x, _BORDER.max_x, _BORDER.min_y, _BORDER.max_y);
#endif

   if (!_BORDER.contains(x, y)) return _sink;

   //if ((byte*)(_starts[x] + (y + _difference_reminder - x) / 3) < byte_start || (byte*)(_starts[x] + (y + _difference_reminder - x) / 3) >= byte_end) printf("ACHTUNG!\n");

   return _starts[x][(y + _difference_reminder - x) / 3];
}

int TriangleLattice::getFirstValidY(int x) {
   int y = _BORDER.min_y;
   while (!isValid(x, y)) y++;
   return y;
}

bool TriangleLattice::isValid(int x, int y) {
   return (y + _difference_reminder - x) % 3 == 0;
}

bool TriangleLattice::isIncreaseForValidY(int y) {
   return !_BORDER.empty && y <= _BORDER.max_y;
}

int TriangleLattice::getFirstValidX() {
   return _BORDER.min_x;
}

bool TriangleLattice::isIncreaseForValidX(int x) {
   return !_BORDER.empty && x <= _BORDER.max_x;
}

void TriangleLattice::switchNextY(int& y) { y += 3; }




// AnswerField definition

IMPL_ERROR(AnswerField, "answer_field");

CP_DEF(AnswerField);

AnswerField::AnswerField(int len, int target_x, int target_y, double target_rotation, int* vertex_weight_link, int* vertex_stereo_link, int* edge_stereo_link) :
CP_INIT,
TL_CP_GET(_vertex_weight), // tree size
TL_CP_GET(_vertex_stereo), // there is an angle in the vertex
TL_CP_GET(_edge_stereo), // trans-cis configuration
TL_CP_GET(_rotation_parity),
TL_CP_GET(_coord_diff_reminder),
TL_CP_GET(_lattices),
TL_CP_GET(_hidden_data_field_array)
{
   length = len;

   _vertex_weight.clear_resize(length);
   for (int i = 0; i < length; i++) _vertex_weight[i] = vertex_weight_link[i];

   _vertex_stereo.clear_resize(length);
   for (int i = 0; i < length; i++) _vertex_stereo[i] = vertex_stereo_link[i];

   _edge_stereo.clear_resize(length);
   for (int i = 0; i < length; i++) _edge_stereo[i] = edge_stereo_link[i];

   _rotation_parity.clear_resize(length + 1);
   _rotation_parity[0] = 0;
   for (int i = 0; i < length; i++) {
      if (_vertex_stereo[i]) 
         _rotation_parity[i + 1] = _rotation_parity[i] ^ 1;
      else
         _rotation_parity[i + 1] = _rotation_parity[i];
   }

   _coord_diff_reminder.clear_resize(length + 1);
   _coord_diff_reminder[0] = 0;
   for (int i = 1; i <= length; i++) {
      if (_rotation_parity[i]) 
         _coord_diff_reminder[i] = (_coord_diff_reminder[i - 1] + 2) % 3;
      else
         _coord_diff_reminder[i] = (_coord_diff_reminder[i - 1] + 1) % 3;
   }

   ObjArray<Array<rectangle> > border_sample_array;
   border_sample_array.clear();
   for (int i = 0; i <= length; i++) {
      border_sample_array.push().clear_resize(2 * i + 1);
   }
   Array<rectangle*> border_sample;
   border_sample.clear_resize(length + 1);
   for (int i = 0; i <= length; i++) {
      border_sample[i] = border_sample_array[i].ptr() + i;
   }


   border_sample[0][0].set(0, 0, 0, 0);

   for (int l = 0; l <= length; l++) {
      for (int rot = -l; rot <= l; rot++) {
         int radius = l - abs(rot) + 1;
         int center_x = rot > 0 ? -1 : 0;
         int center_y = rot > 0 ? 1 : -1;

         border_sample[l][rot].set(rectangle::square(0, 0, l).intersec(rectangle::square(center_x, center_y, radius)));
      }
   }

   border_array.clear();
   for (int i = 0; i <= length; i++) {
      border_array.push().clear_resize(2 * i + 1);
   }
   border.clear_resize(length + 1);
   for (int i = 0; i <= length; i++) {
      border[i] = border_array[i].ptr() + i;
   }

   for (int l = 0; l <= length; l++) {
      for (int rot = -l; rot <= l; rot++) {
         border[l][rot].set(border_sample[l][rot]);

         if (l >= ACCEPTABLE_ERROR) {
            if (abs(rot) <= length + ACCEPTABLE_ERROR - l)
               border[l][rot].intersec(border_sample[length + ACCEPTABLE_ERROR - l][rot].shift(target_x, target_y));
            else
               border[l][rot].set_empty();
         }

         //if (border[l][rot].empty) printf("========== [%d, %d]x[%d, %d]\n", border[l][rot].min_x, border[l][rot].max_x, border[l][rot].min_y, border[l][rot].max_y);
      }
   }

   int global_size = 0;

   for (int l = 0; l <= length; l++) {
      for (int rot = -l; rot <= l; rot++) if (abs(rot) % 2 == _rotation_parity[l]) {
         for (int p = 0; p < 2; p++) {
            global_size += TriangleLattice::getAllocationSize(border[l][rot]);
         }
      }
   }

   printf("Global Size = %d bytes \n", global_size);

   _hidden_data_field_array.clear_resize(global_size);

   byte* free_area = _hidden_data_field_array.ptr();

   _lattices.clear();

   for (int l = 0; l <= length; l++) {
      _lattices.push();
      for (int rot = -l; rot <= l; rot++) {
         _lattices.top().push();
         for (int p = 0; p < 2; p++) {
            if (abs(rot) % 2 == _rotation_parity[l]) {
               _lattices.top().top().push(border[l][rot], _coord_diff_reminder[l], free_area);
               free_area += TriangleLattice::getAllocationSize(border[l][rot]);
            } else _lattices.top().top().push();
         }
      }
   }

   _sink_lattice.init_void();
}

int AnswerField::_cmp_answer_points(answer_point& p1, answer_point& p2, void* context) {
   AnswerField& fld = *((AnswerField*)context);
   return p1.quality(fld) - p2.quality(fld);
}

unsigned short& AnswerField::get_field(int len, answer_point p) { return getLattice(len, p.rot, p.p).getCell(p.x, p.y); };
unsigned short& AnswerField::get_field(answer_point p) { return get_field(length, p); };

TriangleLattice& AnswerField::getLattice(int l, int rot, int p) {
   if (l < 0 || l > length || rot < -l || rot > l || !!p != p) return _sink_lattice;

   return _lattices[l][rot + l][p];
}



void AnswerField::fill() {
   

   for (int l = 0; l <= length; l++) {
      for (int rot = -l; rot <= l; rot++) {
         for (int p = 0; p < 2; p++) {
            TriangleLattice& lat = getLattice(l, rot, p);
            for (int x = lat.getFirstValidX(); lat.isIncreaseForValidX(x); x++) {
               for (int y = lat.getFirstValidY(x); lat.isIncreaseForValidY(y); lat.switchNextY(y))
                  lat.getCell(x, y) = SHORT_INFINITY;
            }
         }
      }
   }

   //getLattice(0, 0, 0).getCell(0, 0) = 0;
   getLattice(0, 0, 1).getCell(0, 0) = 0;


   for (int l = 0; l < length; l++) {
      for (int rot = -l; rot <= l; rot++) {
         for (int p = 0; p < 2; p++) {
            bool can[3];
            can[0] = can[1] = can[2] = false;
            if (_vertex_stereo[l]) {
               if (_edge_stereo[l] == 0) can[0] = can[2] = true;
               else if ((_edge_stereo[l] == MoleculeCisTrans::TRANS) ^ (p == 0)) can[0] = true;
               else can[2] = true;
            }
            else can[1] = true;

            bool* acceptable_rotation = &can[1];

            for (int chenge_rotation = -1; chenge_rotation <= 1; chenge_rotation++) if (acceptable_rotation[chenge_rotation]) {
               int newp = chenge_rotation == 0 ? p : chenge_rotation == 1 ? 1 : 0;
               int next_rot = rot + chenge_rotation;
               TriangleLattice& donor = getLattice(l, rot, p);
               TriangleLattice& retsepient = getLattice(l + 1, next_rot, newp);

               int xchenge = getDx(next_rot);
               int ychenge = getDy(next_rot);

               unsigned short add = max(0, _vertex_weight[l] * (newp ? -1 : 1));
               //add += (p && chenge_rotation > 0) || (!p && chenge_rotation < 0);
               //add += p == newp;


               for (int x = donor.getFirstValidX(); donor.isIncreaseForValidX(x); x++) {
                  for (int y = donor.getFirstValidY(x); donor.isIncreaseForValidY(y); donor.switchNextY(y)) {
                     unsigned short& c = retsepient.getCell(x + xchenge, y + ychenge);
                     unsigned short& d = donor.getCell(x, y);

                     if (c > (unsigned short)(d + add))
                        c = (unsigned short)(d + add);
                     //c = min(c, (unsigned short) (donor.getCell(x, y) + add));
                  }
               }
            }
         }
      }
   }


}

MoleculeLayoutMacrocyclesLattice::CycleLayout::CycleLayout() :
CP_INIT,
TL_CP_GET(point),
TL_CP_GET(rotate),
TL_CP_GET(external_vertex_number),
TL_CP_GET(edge_length)
{
}

void MoleculeLayoutMacrocyclesLattice::initCycleLayout(CycleLayout& cl) {
   cl.external_vertex_number.clear_resize(0);
   cl.external_vertex_number.push(0);

   for (int i = 1; i < length; i++)
      if (_vertex_stereo[i]) cl.external_vertex_number.push(i);

   cl.external_vertex_number.push(length);
   
   cl.vertex_count = cl.external_vertex_number.size() - 1;

   cl.edge_length.clear_resize(cl.vertex_count);

   for (int i = 0; i < cl.vertex_count; i++) 
      cl.edge_length[i] = cl.external_vertex_number[i + 1] - cl.external_vertex_number[i];

   //cl.component_finish.clear_resize(cl.vertex_count);
   //for (int i = 0; i < length; i++)
}

void MoleculeLayoutMacrocyclesLattice::CycleLayout::init(answer_point* ext_point) {
   rotate.clear_resize(vertex_count + 1);
   for (int i = 0; i < vertex_count; i++) 
      rotate[i] = ext_point[external_vertex_number[i + 1]].rot - ext_point[external_vertex_number[i]].rot;

   if (ext_point[0].p == 1) rotate[0] = 1; else rotate[0] = -1;
   rotate[vertex_count] = rotate[0];

   point.clear_resize(vertex_count + 1);
   for (int i = 0; i <= vertex_count; i++) {
      point[i] = Vec2f(ext_point[external_vertex_number[i]].y, 0);
      point[i].rotate(PI/3);
      point[i] += Vec2f(ext_point[external_vertex_number[i]].x, 0);
   }
}

int MoleculeLayoutMacrocyclesLattice::internalValue(CycleLayout cl) {
   int val = 0;

   for (int i = 0; i < cl.vertex_count; i++)
      if ((cl.rotate[i] < 0) ^ (_vertex_weight[cl.external_vertex_number[i]] < 0)) val += abs(_vertex_weight[cl.external_vertex_number[i]]);

   return val;
}

double MoleculeLayoutMacrocyclesLattice::CycleLayout::area() {
   double value = 0;
   for (int i = 1; i < vertex_count - 1; i++)
      value += Vec2f::cross(point[i] - point[0], point[(i + 1) % vertex_count] - point[0]) / 2;

   return abs(value);
}

double MoleculeLayoutMacrocyclesLattice::CycleLayout::perimeter() {
   double perimeter = 0;
   for (int i = 0; i < vertex_count; i++) perimeter += (point[(i + 1) % vertex_count] - point[i]).length();
   return perimeter; 
}

double MoleculeLayoutMacrocyclesLattice::rating(CycleLayout cl) {
   double eps = 1e-9;
   double result = 0;
   int add = 0;
   // distances
   for (int i = 0; i < cl.vertex_count; i++) {
      double len = Vec2f::dist(cl.point[i], cl.point[(i + 1) % cl.vertex_count]) / cl.edge_length[i];
      if (len < eps) add++;
      else if (len < 1) result = max(result, (1 / len - 1));
      else result = max(result, len - 1);
   }
   // angles
   for (int i = 0; i < cl.vertex_count; i++) {
      Vec2f vp1 = cl.point[(i + 1) % cl.vertex_count] - cl.point[i];
      Vec2f vp2 = cl.point[(i + cl.vertex_count - 1) % cl.vertex_count] - cl.point[i];
      double len1 = vp1.length();
      double len2 = vp2.length();
      vp1 /= len1;
      vp2 /= len2;

      double angle = acos(Vec2f::dot(vp1, vp2));
      if (Vec2f::cross(vp2, vp1) > 0) angle = -angle;
      angle /= _target_angle[cl.external_vertex_number[i]];
      if (angle * cl.rotate[i] <= 0) add += 1000;
      double angle_badness = abs((((abs(angle) > 1) ? angle : 1 / angle) - cl.rotate[i]) / 2) * _angle_importance[cl.external_vertex_number[i]];
      result = max(result, angle_badness);
   }


   vector<Vec2f> pp;
   for (int i = 0; i < cl.vertex_count; i++)
   for (int a = cl.external_vertex_number[i], t = 0; a != cl.external_vertex_number[(i + 1) % cl.vertex_count]; a = (a + 1) % length, t++) {
      pp.push_back((cl.point[i] * (cl.edge_length[i] - t) + cl.point[(i + 1) % cl.vertex_count] * t) / cl.edge_length[i]);
   }

   int size = pp.size();
   for (int i = 0; i < size; i++)
   for (int j = 0; j < size; j++) if (i != j && (i + 1) % size != j && i != (j + 1) % size) {
      int nexti = (i + 1) % size;
      int nextj = (j + 1) % size;
      double dist = Vec2f::distSegmentSegment(pp[i], pp[nexti], pp[j], pp[nextj]);

      if (abs(dist) < eps) {
         add++;
         //printf("%5.5f %5.5f %5.5f %5.5f %5.5f %5.5f %5.5f %5.5f \n", xx[i], yy[i], xx[nexti], yy[nexti], xx[j], yy[j], xx[nextj], yy[nextj]);
      }
      else if (dist < 1) result = max(result, 1 / dist - 1);
   }
   // tails

   double diff = internalValue(cl);

   result += 1.0 * diff / length;

   double area = cl.area();

   /*for (int i = 0; i < cl.vertex_count; i++)
   if (_component_finish[cl.external_vertex_number[i]] != cl.external_vertex_number[i] && cl.rotate[i] == -1)
      area += _vertex_added_square[cl.external_vertex_number[i]] * (cl.point[_component_finish[cl.external_vertex_number[i]]] - cl.point[i]).lengthSqr();*/

   double perimeter = cl.perimeter();

   //result += (perimeter * perimeter / 4 / PI / area - 1) / 5;

   //printf("%5.5f\n", result);
   return result + 1000.0 * add;


}

void MoleculeLayoutMacrocyclesLattice::CycleLayout::soft_move_vertex(int vertex_number, Vec2f move_vector) {
   profTimerStart(tt, "0:soft_move_vertex");
   int i = vertex_number;
   double count = vertex_count;
   Vec2f shift_vector = move_vector;
   Vec2f add_vector = move_vector * (-1.0 / vertex_count);
   double mult = 1;
   double add_mult = -1.0 / vertex_count;
   do {
      point[i++] += move_vector * (count / vertex_count);
      count -= 1;
      //p[i++] += shift_vector;
      //shift_vector += add_vector;
      //p[i++] += move_vector * mult;
      //mult += add_mult;
      //shift_vector = move_vector * mult;
      if (i == vertex_count) i = 0;
   } while (i != vertex_number);

   point[vertex_count].copy(point[0]);
}

void MoleculeLayoutMacrocyclesLattice::CycleLayout::stright_rotate_chein(int vertex_number, double angle) {
   profTimerStart(tt, "0:stright_rotate_chein");
   for (int i = 0; i <= vertex_count; i++) if (i != vertex_number) point[i] -= point[vertex_number];
   point[vertex_number].set(0, 0);

   for (int i = vertex_number + 1; i <= vertex_count; i++) point[i].rotate(angle);
}

void MoleculeLayoutMacrocyclesLattice::CycleLayout::stright_move_chein(int vertex_number, Vec2f vector) {
   profTimerStart(tt, "0:stright_move_chein");
   for (int i = vertex_number; i <= vertex_count; i++) point[i] += vector;
}


void MoleculeLayoutMacrocyclesLattice::closingStep(CycleLayout cl, int index, int base_vertex, bool fix_angle, bool fix_next, double multiplyer) {
   profTimerStart(tt, "improvement2");

   int prev_vertex = base_vertex - 1;
   int next_vertex = base_vertex + 1;
   if ((cl.point[0] - cl.point[cl.vertex_count]).lengthSqr() == 0) {
      if (next_vertex == cl.vertex_count) next_vertex = 0;
      if (prev_vertex == -1) prev_vertex = cl.vertex_count - 1;
   }
   int move_vertex = fix_next ? next_vertex : prev_vertex;

   Vec2f move_vector(0, 0);

   if (fix_angle) {

      if ((cl.point[prev_vertex] - cl.point[base_vertex]).length() < 2 * EPSILON || (cl.point[next_vertex] - cl.point[base_vertex]).length() < 2 * EPSILON) return;
      double current_angle = cl.point[base_vertex].calc_angle(cl.point[next_vertex], cl.point[prev_vertex]);
      while (current_angle > 2 * PI) current_angle -= 2 * PI;
      while (current_angle < 0) current_angle += 2 * PI;

      double current_target_angle = _target_angle[cl.external_vertex_number[base_vertex]];
      if (cl.rotate[base_vertex] < 0) current_target_angle = 2 * PI - current_target_angle;
      double anti_current_target_angle;
      if (current_target_angle > PI) {
         if (current_angle > current_target_angle) anti_current_target_angle = 2 * PI;
         else anti_current_target_angle = PI;
      }
      else {
         if (current_angle < current_target_angle) anti_current_target_angle = 0;
         else anti_current_target_angle = PI;
      }

      double better_change_angle = 0;
      double worse_chenge_angle = 0;
      if (abs(current_angle - current_target_angle) < EPSILON) {
         better_change_angle = current_angle * multiplyer;
         worse_chenge_angle = -current_angle * multiplyer;
      }
      else {
         better_change_angle = (current_target_angle - current_angle) * multiplyer;
         worse_chenge_angle = (anti_current_target_angle - current_angle) * multiplyer;
      }

      double actual_chenge_angle = 0;

      if ((cl.point[0] - cl.point[cl.vertex_count]).lengthSqr() == 0) {

         if (abs(current_angle - current_target_angle) < EPSILON) actual_chenge_angle = 0;
         else actual_chenge_angle = better_change_angle;

         if (fix_next) actual_chenge_angle *= -1;

         //actual_chenge_angle *= _angle_importance[vertex_number[base_vertex]];

         move_vector.rotateAroundSegmentEnd(cl.point[move_vertex], cl.point[base_vertex], actual_chenge_angle);
         move_vector -= cl.point[move_vertex];

         if (!fix_next) move_vector.negate();

         if (fix_next) cl.soft_move_vertex(move_vertex, move_vector);
         else cl.soft_move_vertex(base_vertex, move_vector);
      }
      else {
         double angle = current_angle;
         while (angle < 0) angle += 2 * PI;
         while (angle >= 2 * PI) angle -= 2 * PI;

         for (int i = next_vertex; i < cl.vertex_count; i++) angle -= cl.point[base_vertex].calc_angle(cl.point[i], cl.point[i + 1]);
         for (int i = prev_vertex; i > 0; i--) angle += cl.point[base_vertex].calc_angle(cl.point[i], cl.point[i - 1]);

         if (abs(angle + actual_chenge_angle) > abs(angle + better_change_angle)) actual_chenge_angle = better_change_angle;
         if (abs(angle + actual_chenge_angle) > abs(angle + worse_chenge_angle)) actual_chenge_angle = worse_chenge_angle;

         //actual_chenge_angle *= _angle_importance[vertex_number[base_vertex]];

         cl.stright_rotate_chein(base_vertex, -actual_chenge_angle);
      }
   }
   else {
      int first_vertex = fix_next ? base_vertex : prev_vertex;
      int second_vertex = fix_next ? next_vertex : base_vertex;
      double current_dist = Vec2f::dist(cl.point[first_vertex], cl.point[second_vertex]);
      double current_target_dist = cl.edge_length[first_vertex];
      Vec2f better_chenge_vector = cl.point[second_vertex] - cl.point[first_vertex];
      Vec2f worse_chenge_vector = cl.point[second_vertex] - cl.point[first_vertex];

      if (abs(current_target_dist - current_dist) > EPSILON) {
         better_chenge_vector *= (current_target_dist - current_dist) / current_dist * multiplyer;
         worse_chenge_vector *= (current_dist - current_target_dist) / current_dist * multiplyer;
      }
      else {
         better_chenge_vector *= multiplyer;
         worse_chenge_vector *= -multiplyer;
      }

      if ((cl.point[0] - cl.point[cl.vertex_count]).lengthSqr() == 0) {
         if (abs(current_target_dist - current_dist) > EPSILON)
            cl.soft_move_vertex(second_vertex, better_chenge_vector);
      }
      else {
         Vec2f actual_chenge_vector(0, 0);

         Vec2f current_discrepancy = cl.point[cl.vertex_count] - cl.point[0];
         if ((current_discrepancy + actual_chenge_vector).lengthSqr() > (current_discrepancy + better_chenge_vector).lengthSqr()) actual_chenge_vector = better_chenge_vector;
         if ((current_discrepancy + actual_chenge_vector).lengthSqr() > (current_discrepancy + worse_chenge_vector).lengthSqr()) actual_chenge_vector = worse_chenge_vector;

         cl.stright_move_chein(second_vertex, actual_chenge_vector);
      }
   }

}

void MoleculeLayoutMacrocyclesLattice::closing(CycleLayout cl) {
   Random rand(931170243);

   int iter_count = max(200 * cl.vertex_count, 10000);
   double multiplyer = 0.3;

   for (int i = 0; i < iter_count; i++) {
      double lenSqr = (cl.point[0] - cl.point[cl.vertex_count]).lengthSqr();
      if (0 < lenSqr && lenSqr < 0.25) {
         cl.point[cl.vertex_count].copy(cl.point[0]);
         //printf("%d/%d\n", i, iter_count);
         break;
      }
      bool angle = rand.next() & 1;
      bool next = rand.next() & 1;
      int base_vertex = rand.next(cl.vertex_count + 1);

      if ((cl.point[0] - cl.point[cl.vertex_count]).lengthSqr() != 0) {
         if (angle && (base_vertex == 0 || base_vertex == cl.vertex_count)) continue;
         if (!angle && ((base_vertex == 0 && !next) || (base_vertex == cl.vertex_count && next))) continue;
      }
      else {
         if (base_vertex == cl.vertex_count) continue;
      }

      closingStep(cl, i, base_vertex, angle, next, multiplyer);
      if (lenSqr == 0) multiplyer *= CHANGE_FACTOR;
      //if (i % 100 == 0) printf("%.5f\n", rating(cl));
   }


}

void MoleculeLayoutMacrocyclesLattice::smoothing(CycleLayout cl) {
   profTimerStart(tt, "smoothing");

   closing(cl);

   Random rand(931170240);
   int iter_count = max(50 * length, 2000);

   for (int i = 0; i < iter_count; i++) 
      smoothingStep(cl, i >= iter_count / 2, rand.next(cl.vertex_count));
}

void MoleculeLayoutMacrocyclesLattice::smoothingStep(CycleLayout cl, bool do_dist, int vertex_number) {

   Vec2f p1 = cl.point[(vertex_number - 1 + cl.vertex_count) % cl.vertex_count];
   Vec2f p2 = cl.point[(vertex_number + 1 + cl.vertex_count) % cl.vertex_count];
   double r1 = cl.edge_length[(cl.vertex_count + vertex_number - 1) % cl.vertex_count];
   double r2 = cl.edge_length[(cl.vertex_count + vertex_number) % cl.vertex_count];

   double len1 = Vec2f::dist(p1, cl.point[vertex_number]);
   double len2 = Vec2f::dist(p2, cl.point[vertex_number]);

   double r3 = Vec2f::dist(p1, p2) / sqrt(3.0);

   Vec2f p3;

   if (cl.rotate[vertex_number] != 0) {
      p3 = (p1 + p2) / 2;
      Vec2f a = (p2 - p1) / sqrt(12.0f);
      a.rotate(PI / 2 * cl.rotate[vertex_number]);
      p3 += a;
   }
   else {
      p3 = (p1*r2 + p2*r1) / (r1 + r2);
   }


   double len3 = Vec2f::dist(p3, cl.point[vertex_number]);
   if (cl.rotate[vertex_number] == 0) r3 = 0;

   //printf("%5.5f %5.5f %5.5f %5.5f\n", len1, len2, len3, r3);
   Vec2f newPoint;
   double eps = 1e-4;
   double eps2 = eps * eps;
   if (len1 < eps || len2 < eps || len3 < eps) {
      cl.point[vertex_number] = (p1 + p2) / 2.0;
   }
   else {
      double coef1 = (r1 / len1 - 1);
      double coef2 = (r2 / len2 - 1);
      double coef3 = (r3 / len3 - 1);
      if (cl.rotate[vertex_number] != 0) {
         double angle = acos(Vec2f::cross(p1 - cl.point[vertex_number], p2 - cl.point[vertex_number]) / (Vec2f::dist(p1, cl.point[vertex_number])*Vec2f::dist(p2, cl.point[vertex_number])));
         //if (angle < 2 * PI / 3) coef3 /= 10;
      }

      //if (!isIntersec(x[worstVertex], y[worstVertex], x3, y3, x1, y1, x2, y2)) coef3 *= 10;
      if (cl.rotate[vertex_number] == 0) coef3 = -1;
      //printf("%5.5f %5.5f %5.5f\n", coef1, coef2, coef3);
      newPoint += (cl.point[vertex_number] - p1)*coef1;
      newPoint += (cl.point[vertex_number] - p2)*coef2;
      newPoint += (cl.point[vertex_number] - p3)*coef3;

      if (do_dist) {
            float good_distance = 1;
            for (int j = 0; j < cl.vertex_count; j++) {
               int nextj = (j + 1) % cl.vertex_count;
               Vec2f pp = cl.point[j];
               Vec2f dpp = (cl.point[nextj] - cl.point[j]) / cl.edge_length[j];

               for (int t = cl.external_vertex_number[j], s = 0; t != cl.external_vertex_number[nextj]; t = (t + 1) % length, s++) {
                  if (t != cl.external_vertex_number[vertex_number] && (t + 1) % length != cl.external_vertex_number[vertex_number] && t != (cl.external_vertex_number[vertex_number] + 1) % length) {
                     double distSqr = Vec2f::distSqr(pp, cl.point[vertex_number]);
                     if (distSqr < good_distance && distSqr > eps2) {
                        double dist = sqrt(distSqr);
                        double coef = (good_distance - dist) / dist;
                        //printf("%5.5f \n", dist);
                        newPoint += (cl.point[vertex_number] - pp)*coef;
                     }
                  }
                  pp += dpp;
               }
            }
         
      }

      newPoint *= SMOOTHING_MULTIPLIER;

      cl.point[vertex_number] += newPoint;
   }
}

Vec2f MoleculeLayoutMacrocyclesLattice::CycleLayout::getWantedVector(int vertex_number) {

   Vec2f p1 = point[(vertex_number - 1 + vertex_count) % vertex_count];
   Vec2f p2 = point[(vertex_number + 1 + vertex_count) % vertex_count];
   double r1 = edge_length[(vertex_count + vertex_number - 1) % vertex_count];
   double r2 = edge_length[(vertex_count + vertex_number) % vertex_count];

   double len1 = Vec2f::dist(p1, point[vertex_number]);
   double len2 = Vec2f::dist(p2, point[vertex_number]);

   double r3 = Vec2f::dist(p1, p2) / sqrt(3.0);

   Vec2f p3 = (p1 + p2) / 2;

   if (rotate[vertex_number] != 0) {
      Vec2f a = (p2 - p1) / sqrt(12.0f);
      a.rotate(PI / 2 * rotate[vertex_number]);
      p3 += a;
   }
   else {
      p3 = (p1*r1 + p2*r2) / (r1 + r2);
   }


   double len3 = Vec2f::dist(p3, point[vertex_number]);
   if (rotate[vertex_number] == 0) r3 = 0;



   //printf("%5.5f %5.5f %5.5f %5.5f\n", len1, len2, len3, r3);
   Vec2f newPoint;
   double eps = 1e-4;
   double eps2 = eps * eps;
   //if (len1 < eps || len2 < eps || len3 < eps) return ;

   double coef1 = (r1 / len1 - 1);
   double coef2 = (r2 / len2 - 1);
   double coef3 = (r3 / len3 - 1);
/*   if (rotate[vertex_number] != 0) {
      double angle = acos(Vec2f::cross(p1 - point[vertex_number], p2 - point[vertex_number]) / (Vec2f::dist(p1, point[vertex_number])*Vec2f::dist(p2, point[vertex_number])));
      //if (angle < 2 * PI / 3) coef3 /= 10;
   }*/

   //if (!isIntersec(x[worstVertex], y[worstVertex], x3, y3, x1, y1, x2, y2)) coef3 *= 10;
   if (rotate[vertex_number] == 0) coef3 = -1;
   //printf("%5.5f %5.5f %5.5f\n", coef1, coef2, coef3);
   newPoint += (point[vertex_number] - p1)*coef1;
   newPoint += (point[vertex_number] - p2)*coef2;
   newPoint += (point[vertex_number] - p3)*coef3;

   return newPoint * SMOOTHING_MULTIPLIER;
}