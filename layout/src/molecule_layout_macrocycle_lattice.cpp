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





// TriangleLattice defenition

TriangleLattice::TriangleLattice() :
CP_INIT,
TL_CP_GET(_first_valid_y_array) 
{

}

TriangleLattice::TriangleLattice(int min_x, int max_x, int min_y, int max_y, int hidden_min_x, int hidden_max_x, int hidden_min_y, int hidden_max_y, int rem, unsigned short* data_link):
CP_INIT,
TL_CP_GET(_first_valid_y_array)
{
   MAX_X = max_x;
   MIN_X = min_x;
   MAX_Y = max_y;
   MIN_Y = min_y;
   HIDDEN_MAX_X = hidden_max_x;
   HIDDEN_MIN_X = hidden_min_x;
   HIDDEN_MAX_Y = hidden_max_y;
   HIDDEN_MIN_Y = hidden_min_y;

   difference_reminder = rem;

   QS_DEF(Array<unsigned short*>, starts_array);
   starts_array.clear_resize(hidden_max_x - hidden_min_x + 1);

   starts = &(starts_array[0]) - hidden_min_x;

   starts[hidden_min_x] = data_link;
   for (int x = hidden_min_x; x < hidden_max_x; x++) {
      int left = hidden_min_y;
      int right = hidden_max_y;
      while ((x - left - difference_reminder) % 3 != 0) left++;
      while ((x - right - difference_reminder) % 3 != 0) right--;
      starts[x + 1] = starts[x] + (right - left + 2)/3;
   }

   _first_valid_y_array.clear_resize(hidden_max_x - hidden_min_x + 1);
   _first_valid_y = &_first_valid_y_array[0] - hidden_min_x;

   for (int x = hidden_min_x; x <= hidden_max_x; x++) {
      _first_valid_y[x] = HIDDEN_MIN_X;
      while ((_first_valid_y[x] + difference_reminder - x) % 3 != 0) _first_valid_y[x]++;

      starts[x] += (x - rem - _first_valid_y[x]) / 3;
   }

   grid = data_link;
}


int TriangleLattice::get_value(int min_x, int max_x, int min_y, int max_y) {
   return ((max_x - min_x + 1) * (max_y - min_y + 1)  + 2) / 3;
}



unsigned short& TriangleLattice::get_cell(int x, int y) {
   if ((x - y) % 3 != difference_reminder) throw Error("difference of coordinates reminder is failed");
   if (x < HIDDEN_MIN_X || x > HIDDEN_MAX_X || y < HIDDEN_MIN_Y || y > HIDDEN_MAX_Y) throw Error("point (%d, %d) is out of framework [%d, %d]x[%d, %d].", x, y, HIDDEN_MIN_X, HIDDEN_MAX_X, HIDDEN_MIN_Y, HIDDEN_MAX_Y);
   
   return starts[x][(y + difference_reminder - x) / 3];
}

int TriangleLattice::get_first_valid_y(int x) {
   return _first_valid_y[x];
}



// AnswerField definition

const int AnswerField::dx[6] = { 1, 0, -1, -1, 0, 1 };
const int AnswerField::dy[6] = { 0, 1, 1, 0, -1, -1 };

AnswerField::AnswerField() :
CP_INIT,
TL_CP_GET(_vertex_weight), // tree size
TL_CP_GET(_vertex_stereo), // there is an angle in the vertex
TL_CP_GET(_edge_stereo), // trans-cis configuration
TL_CP_GET(_rotation_parity),
TL_CP_GET(_coord_diff_denominator),
TL_CP_GET(_field)
{};

void AnswerField::set_length(int len) { 
   length = len;

   _vertex_weight.clear_resize(length);
   _vertex_weight.zerofill();

   _edge_stereo.clear_resize(length);
   _edge_stereo.fffill();

   _vertex_stereo.clear_resize(length);
   _vertex_stereo.fffill();

   _rotation_parity.clear_resize(length + 1);
   _rotation_parity.zerofill();

   _coord_diff_denominator.clear_resize(length + 1);
   _coord_diff_denominator.zerofill();
}

void AnswerField::add_vertex_outside_weight(int v, int a) {
   _vertex_weight[v] += a;
}

void AnswerField::set_edge_stereo(int e, int stereo) {
   _edge_stereo[e] = stereo;
}

void AnswerField::set_vertex_edge_parallel(int v, bool par) {
   _vertex_stereo[v] = !par;
}

void AnswerField::init() {

   _rotation_parity[0] = 1;
   for (int i = 0; i < length; i++){
      if (_vertex_stereo[i]) _rotation_parity[i + 1] = !_rotation_parity[i];
      else _rotation_parity[i + 1] = _rotation_parity[i];
   }

   _coord_diff_denominator[0] = 0;
   for (int i = 1; i <= length; i++) {
      if (_rotation_parity[i]) _coord_diff_denominator[i] = (_coord_diff_denominator[i] + 1) % 3;
      else _coord_diff_denominator[i] = (_coord_diff_denominator[i] + 2) % 3;
   }
}

void AnswerField::build() {
   _field.clear_resize((length + 1)*(2 * length - 1) * 2);

   for (int i = 0; i <= length; i++)
   for (int rot = -i + 1; rot < i; rot++) if (abs(rot) & 1 == _rotation_parity[i])
   for (int p = 0; p < 2; p++) {
      TriangleLattice& lat = get_lattice(i, rot, p);

      //TODO...
   }
}

TriangleLattice& AnswerField::get_lattice(int l, int rot, int p) {
   if (l < 0 || l > length || rot <= -l || rot >= l || p < 0 || p > 1) throw Error("incorrect parameters of AnswerField::get_lattice: l = %d, rot = %d, p = %d", l, rot, p);
   return _field[l * (2*length - 1) * 2 + (rot + length - 1) * 2 + p];
}

void AnswerField::fill() {
   get_lattice(1, 0, 1).get_cell(1, 0) = 0;

   for (int l = 1; l < length; l++) {
      for (int rot = -l; rot <= l; rot++) {
         for (int p = 0; p < 2; p++) {
            TriangleLattice& current_lat = get_lattice(l, rot, p);
            if (!_vertex_stereo[l]) {
               TriangleLattice& next_lat = get_lattice(l + 1, rot, p);

            }
         }
      }
   }
   

}