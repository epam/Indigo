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

MoleculeLayoutMacrocyclesLattice::MoleculeLayoutMacrocyclesLattice(int size, int* _v_w, int* _v_s, int* _e_s) :
CP_INIT,
TL_CP_GET(_vertex_weight), // tree size
TL_CP_GET(_vertex_stereo), // there is an angle in the vertex
TL_CP_GET(_edge_stereo) // trans-cis configuration
{
   length = size;

   _vertex_weight.clear_resize(size);
   for (int i = 0; i < size; i++) _vertex_weight[i] = _v_w[i];

   _vertex_stereo.clear_resize(size);
   for (int i = 0; i < size; i++) _vertex_stereo[i] = _v_s[i];

   _edge_stereo.clear_resize(size);
   for (int i = 0; i < size; i++) _edge_stereo[i] = _e_s[i];


   calculate_rotate_length();
   rotate_cycle(rotate_length);



   // dividng into two halfs. For each of them:

   // 1. calculating denominators for rotations and coordinates difference
   // 2. building the answers fields (create special class?)
   // 3. fill answers fields.

   // searching of nearest points


   // shifintg back
}

void MoleculeLayoutMacrocyclesLattice::calculate_rotate_length() {

   rotate_length = 0;
   int max_value = -INFINITY;

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





// TriangleLattice definition

// x - y = difference_reminder (mod 3)

IMPL_ERROR(TriangleLattice, "triangle_lattice");

TriangleLattice::TriangleLattice()
{

}

TriangleLattice::TriangleLattice(rectangle rec, int rem, byte* data_link) {
   init(rec, rem, data_link);
}

void TriangleLattice::init(rectangle rec, int rem, byte* data_link)
{
   _BORDER = rec;

   _difference_reminder = rem;

   _starts = (unsigned short**)data_link;
   _starts -= _BORDER.min_x;

   _starts[_BORDER.min_x] = (unsigned short*)(_starts + _BORDER.max_x + 1);
   for (int x = _BORDER.min_x; x < _BORDER.max_x; x++) {
      int left = _BORDER.min_y;
      int right = _BORDER.max_y;
      while ((x - left - _difference_reminder) % 3 != 0) left++;
      while ((x - right - _difference_reminder) % 3 != 0) right--;
      _starts[x + 1] = _starts[x] + (right - left + 2)/3;
   }
}





unsigned short& TriangleLattice::getCell(int x, int y) {
   if ((x - y) % 3 != _difference_reminder) throw Error("difference of coordinates reminder is failed: x = %d, y = %d, rem = %", x, y, _difference_reminder);
   if (!_BORDER.empty && !_BORDER.expand(2).contains(x, y)) throw Error("point (%d, %d) is not close to framework [%d, %d]x[%d, %d].", x, y, _BORDER.min_x, _BORDER.max_x, _BORDER.min_y, _BORDER.max_y);

   if (!_BORDER.contains(x, y)) return _sink;
   
   return _starts[x][(y + _difference_reminder - x) / 3];
}

int TriangleLattice::getFirstValidY(int x) {
   int y = _BORDER.min_y;
   while ((y + _difference_reminder - x) % 3 != 0) y++;
   return y;
}

bool TriangleLattice::isIncreaseForValidY(int y) {
   return y <= _BORDER.max_y;
}





// AnswerField definition

IMPL_ERROR(AnswerField, "answer_field");

CP_DEF(AnswerField);


const int AnswerField::dx[6] = { 1, 0, -1, -1, 0, 1 };
const int AnswerField::dy[6] = { 0, 1, 1, 0, -1, -1 };

AnswerField::AnswerField(int len, int target_x, int target_y, double target_rotation, int* vertex_weight_link, int* vertex_stereo_link, int* edge_stereo_link) :
CP_INIT,
TL_CP_GET(_vertex_weight), // tree size
TL_CP_GET(_vertex_stereo), // there is an angle in the vertex
TL_CP_GET(_edge_stereo), // trans-cis configuration
TL_CP_GET(_rotation_parity),
TL_CP_GET(_coord_diff_reminder),
TL_CP_GET(_field),
TL_CP_GET(_lattices)
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
      if (_edge_stereo[i]) 
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

   ObjArray<Array<rectangle>> border_sample_array;
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

         border_sample[l][rot].set(max(center_x - radius, -l), min(center_x + radius, l), max(center_y - radius, -l), min(center_y + radius, l));
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
      }
   }

   int global_size = 0;

   for (int l = 0; l <= length; l++) {
      for (int rot = -l; rot <= l; rot++) if (rot % 2 == _rotation_parity[l]) {
         for (int p = 0; p < 2; p++) {
            global_size += TriangleLattice::getAllocationSize(border[l][rot]);
         }
      }
   }

   printf("Global Size = %d bytes \n", global_size);

   QS_DEF(Array<byte>, _hidden_data_field_array);
   _hidden_data_field_array.clear_resize(global_size);

   byte* free_area = _hidden_data_field_array.ptr();

   QS_DEF(ObjArray<ObjArray<ObjArray<TriangleLattice>>>, _lattices);

   for (int l = 0; l <= length; l++) {
      _lattices.push();
      for (int rot = -l; rot <= l; rot++) {
         _lattices.top().push();
         for (int p = 0; p < 2; p++) {
            _lattices.top().top().push(border[l][rot], _coord_diff_reminder[l], free_area);
            free_area += TriangleLattice::getAllocationSize(border[l][rot]);
         }
      }
   }

}

TriangleLattice& AnswerField::get_lattice(int l, int rot, int p) {
   if (l < 0 || l > length || rot < -l || rot > l || !!p != p) return _sink_lattice;

   return _lattices[l][rot + l][p];
}

void AnswerField::fill() {
   

}