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

CP_DEF(MoleculeLayoutMacrocyclesLattice);


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


   calculate_rotate_length();
   rotate_cycle(rotate_length);

}

void MoleculeLayoutMacrocyclesLattice::doLayout() {
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

   double best_badness = 1e30;
   int best_index = -1;

   _positions.clear_resize(length + 1);

   //printf("%d\n", points.size());

   points.qsort(&AnswerField::_cmp_answer_points, &answfld); 

   answfld._restore_path(_positions.ptr(), points[0]);
}


/*double MoleculeLayoutMacrocyclesLattice::rating(Array<answer_point> intP) {

   int len = intP.size() - 1;

   QS_DEF(Array<Vec2f>, p);
   p.clear_resize(len + 1);
   for (int i = 0; i <= len; i++) {
      p[i] = Vec2f(intP[i].y, 0);
      p[i].rotate(PI / 3);
      p[i] += Vec2f(intP[i].y, 0);
   }



   return 0;
}*/

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

void MoleculeLayoutMacrocyclesLattice::set_vertex_added_square(int v, double s) {
   _vertex_added_square[v] = s;
}

void MoleculeLayoutMacrocyclesLattice::setVertexDrawn(int v, bool drawn) {
   _vertex_drawn[v] = drawn;
}

void MoleculeLayoutMacrocyclesLattice::set_component_finish(int v, int f) {
   _component_finish[v] = f;
}

void MoleculeLayoutMacrocyclesLattice::set_target_angle(int v, double angle) {
   _target_angle[v] = angle;
}

void MoleculeLayoutMacrocyclesLattice::set_angle_importance(int v, double imp) {
   _angle_importance[v] = imp;
}

void AnswerField::_restore_path(Vec2f* point, answer_point finish) {
   QS_DEF(Array<answer_point>, path);

   path.clear_resize(length + 1);

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

   for (int i = 0; i <= length; i++) {
      point[i].set(path[i].y, 0);
      point[i].rotate(PI / 3);
      point[i].x += path[i].x;
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

   getLattice(0, 0, 0).getCell(0, 0) = 0;
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