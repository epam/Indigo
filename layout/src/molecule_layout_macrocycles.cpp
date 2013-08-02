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

using namespace indigo;
using namespace std;

MoleculeLayoutMacrocycles::MoleculeLayoutMacrocycles (int size) : 
   TL_CP_GET(_vertex_weight), // tree size
   TL_CP_GET(_vertex_stereo), // there is an angle in the vertex
   TL_CP_GET(_edge_stereo), // trans-cis configuration
   TL_CP_GET(_positions) // position of vertex
{
   // Set default values...
   length = size;

   _vertex_weight.clear_resize(size);
   _vertex_weight.fill(1);

   _vertex_stereo.clear_resize(size);
   _vertex_stereo.zerofill();

   _edge_stereo.clear_resize(size);
   _edge_stereo.zerofill();

   _positions.clear_resize(size);

}

void MoleculeLayoutMacrocycles::setVertexOutsideWeight (int v, int weight)
{
   _vertex_weight[v] = weight;
}

void MoleculeLayoutMacrocycles::setVertexEdgeParallel (int v, bool parallel) 
{
   _vertex_stereo[v] = !parallel;
}

void MoleculeLayoutMacrocycles::setEdgeStereo (int e, int stereo)
{
   _edge_stereo[e] = stereo;
}

Vec2f &MoleculeLayoutMacrocycles::getPos (int v)
{
   return _positions[v];
}


void MoleculeLayoutMacrocycles::doLayout ()
{
   // ...
/*   QS_DEF(Array<Vec2f>, positions2);

   //while ()
   {
      positions2.swap(_positions);
      
   }*/
   profTimerStart(t, "bc.layout");

//   printf("1\n");
   double b = depictionMacrocycleMol(false);
//   printf("2\n");
   double b2 = 1e9;
   //double b2 = depictionMacrocycleMol(true);
//   printf("3\n");
   double b3 = depictionCircle();

   if (b <= b2 && b <= b3) {
      depictionMacrocycleMol(false);
  //    printf("------------------------------------------------> %d \n", 1);
      return;
   }
   if (b2 <= b && b2 <= b3) {
      depictionMacrocycleMol(true);
    //  printf("------------------------------------------------> %d \n", 2);
     return;
   }
   //printf("------------------------------------------------> %d \n", 3);
   depictionCircle();
   return;
   /*if (b >= 1000000) {
      profTimerStart(t, "bc.layout #2");
      b = depictionMacrocycleMol(mol, true);
   }
   //if (b >= 1000000)
   {
      profTimerStart(t, "bc.layout #3");
      b = depictionCircle(mol);
   }*/
    //  throw Error("Cannot find a layout with all cis-trans constraints");
   /*
   for (int e = mol.edgeBegin(); e != mol.edgeEnd(); e = mol.edgeNext(e))
      if (mol.cis_trans.getParity(e) != 0)
      {
         mol.cis_trans.ignore(e);
         break;
      }
   */

   //return b;

}


bool MoleculeLayoutMacrocycles::canApply (BaseMolecule &mol)
{
   if (!mol.isConnected(mol)) {
      return false;
   }

   for (int v = mol.vertexBegin(); v != mol.vertexEnd(); v = mol.vertexNext(v)) {
      if (mol.getVertex(v).degree() != 2) {
         return false;
      }
   }

   return true;
}

double MoleculeLayoutMacrocycles::layout (BaseMolecule &mol)
{
   profTimerStart(t, "bc.layout");

//   printf("1\n");
//   double b = depictionMacrocycleMol(false);
//   printf("2\n");
   //double b2 = depictionMacrocycleMol(true);
//   printf("3\n");
   double b3 = depictionCircle();
//   double b3 = 1e10;

/*   if (b <= b2 && b <= b3) {
      depictionMacrocycleMol(false);
      return 1;
   }
   if (b2 <= b && b2 <= b3) {
      depictionMacrocycleMol(true);
      return 2;
   }*/
   //depictionCircle(mol);
   return 3;
   /*if (b >= 1000000) {
      profTimerStart(t, "bc.layout #2");
      b = depictionMacrocycleMol(mol, true);
   }
   //if (b >= 1000000)
   {
      profTimerStart(t, "bc.layout #3");
      b = depictionCircle(mol);
   }*/
    //  throw Error("Cannot find a layout with all cis-trans constraints");
   /*
   for (int e = mol.edgeBegin(); e != mol.edgeEnd(); e = mol.edgeNext(e))
      if (mol.cis_trans.getParity(e) != 0)
      {
         mol.cis_trans.ignore(e);
         break;
      }
   */

   //return b;
}

// 
// Original code
//

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


using namespace std;

double sqr(double x) {return x*x;}

int improvement(int ind, int molSize, int *rotateAngle, int *edgeLenght, int *vertexNumber, Vec2f *p, Random* rand, bool profi, double multiplier) {
   int worstVertex = (*rand).next(ind);
   
   Vec2f p1 = p[(worstVertex - 1 + ind) % ind];
   Vec2f p2 = p[(worstVertex + 1 + ind) % ind];
   double r1 = edgeLenght[(ind + worstVertex - 1) % ind];
   double r2 = edgeLenght[(ind + worstVertex) % ind];

   double len1 = Vec2f::dist(p1, p[worstVertex]);
   double len2 = Vec2f::dist(p2, p[worstVertex]);

   double r3 = Vec2f::dist(p1, p2)/sqrt(3.0);
      
   Vec2f p3 = (p1 + p2)/2;

   if (rotateAngle[worstVertex] != 0) {
      Vec2f a = (p2 - p1)/sqrt(12.0);
      a.rotate(PI/2 * rotateAngle[worstVertex]);
      p3 += a;
   } else {
      p3 = (p1*r1 + p2*r2)/(r1 + r2);
   }


   double len3 = Vec2f::dist(p3, p[worstVertex]);
   if (rotateAngle[worstVertex] == 0) r3 = 0;

   //printf("%5.5f %5.5f %5.5f %5.5f\n", len1, len2, len3, r3);
   Vec2f newPoint;
   double eps = 1e-4;
   if (len1 < eps || len2 < eps || len3 < eps) {
      p[worstVertex] = (p1 + p2)/2.0;
   } else {
      double coef1 = (r1/len1 - 1);
      double coef2 = (r2/len2 - 1);
      double coef3 = (r3/len3 - 1);
      if (rotateAngle[worstVertex] != 0) {
         double angle = acos(Vec2f::cross(p1 - p[worstVertex], p2 - p[worstVertex])/(Vec2f::dist(p1, p[worstVertex])*Vec2f::dist(p2, p[worstVertex])));
         //if (angle < 2 * PI / 3) coef3 /= 10;
      }

      //if (!isIntersec(x[worstVertex], y[worstVertex], x3, y3, x1, y1, x2, y2)) coef3 *= 10;
      if (rotateAngle[worstVertex] == 0) coef3 = -1;
      //printf("%5.5f %5.5f %5.5f\n", coef1, coef2, coef3);
      newPoint += (p[worstVertex] - p1)*coef1;
      newPoint += (p[worstVertex] - p2)*coef2;
      newPoint += (p[worstVertex] - p3)*coef3;

      if (profi) {
         for (int i = 0; i < ind; i++) if (i != worstVertex && (i + 1) % ind != worstVertex) {
            double dist = Vec2f::distPointSegment(p[worstVertex], p[i], p[(i + 1) % ind]);
            if (dist < 1 && dist > eps) {
               Vec2f normal = (p[(i + 1) % ind] - p[i]);
               normal.rotate(PI/2);
               double c = Vec2f::cross(p[i], p[(i + 1)%ind]);
               double s = normal.length();

               normal /= s;
               c /= s;

               double t = - c - Vec2f::dot(p[worstVertex], normal);
               
               Vec2f pp;

               if (s < eps) {
                    pp = p[i];
               } else {
                  pp = p[worstVertex] + normal * t;
                  if (Vec2f::dist(p[worstVertex], p[i]) < Vec2f::dist(p[worstVertex], pp)) {
                     pp = p[i];
                  }
                  if (Vec2f::dist(p[worstVertex], p[(i + 1)%ind]) < Vec2f::dist(p[worstVertex], pp)) {
                     pp = p[(i + 1)%ind];
                  }
               }

               double coef = (1 - dist)/dist;

               newPoint += (p[worstVertex] - pp) * coef;
            }
         }
      } else {
         for (int j = 0; j < ind; j++) {
            int nextj = (j + 1) % ind;
            Vec2f pp = p[j];
            Vec2f dpp = (p[nextj] - p[j]) / edgeLenght[j];

            for (int t = vertexNumber[j], s = 0; t != vertexNumber[nextj]; t = (t + 1)%molSize, s++) {
               if (t != vertexNumber[worstVertex] && (t + 1)%molSize != vertexNumber[worstVertex] && t != (vertexNumber[worstVertex] + 1) % molSize) {
                  double dist = 0;
                  double sqrt2 = 1.4142135623730950488016887242097;
                  dist = Vec2f::dist(pp, p[worstVertex]);
                  if (dist < sqrt2 && dist > eps) {
                     double coef = (sqrt2 - dist)/dist;
                     //printf("%5.5f \n", dist);
                     newPoint += (p[worstVertex] - pp)*coef;
                  }
               }
               pp += dpp;
            }
         }
      }

      newPoint *= multiplier;

      p[worstVertex] += newPoint;
   }
   return worstVertex;
}

void MoleculeLayoutMacrocycles::smoothing(int ind, int molSize, int *rotateAngle, int *edgeLenght, int *vertexNumber, Vec2f *p, bool profi) {
   Random rand(931170240);
   for (int i = 0; i < 1000; i++) improvement(ind, molSize, rotateAngle, edgeLenght, vertexNumber, p, &rand, profi, 0.1);
   for (int i = 0; i < 1000; i++) improvement(ind, molSize, rotateAngle, edgeLenght, vertexNumber, p, &rand, profi, 0.01);
}

double MoleculeLayoutMacrocycles::badness(int ind, int molSize, int *rotateAngle, int *edgeLenght, int *vertexNumber, Vec2f *p) {
   double eps = 1e-9;
   double result = 0;
   int add = 0;
   for (int i = 0; i < ind; i++) {
      double len = Vec2f::dist(p[i], p[(i + 1)%ind])/edgeLenght[i];
      if (len < eps) add++;
      else if (len < 1) result = max(result, (1/len - 1));
      else result = max(result, len - 1);
   }
   for (int i = 0; i < ind; i++) {
      Vec2f vp1 = p[i] - p[(i + ind - 1) % ind];
      Vec2f vp2 = p[(i + 1) % ind] - p[i];
      double len1 = vp1.length();
      double len2 = vp2.length();
      vp1 /= len1;
      vp2 /= len2;

      double angle = acos(Vec2f::dot(vp1, vp2));
      if (Vec2f::cross(vp2, vp1) > 0) angle = -angle;
      angle /= (PI/3);
      if (angle * rotateAngle[i] <= 0) add += 1000;
      else if (abs(angle) > 1) result = max(result, abs(angle - rotateAngle[i])/2);
      else result = max(result, abs(1/angle - rotateAngle[i])/2);
   }


   vector<Vec2f> pp;
   for (int i = 0; i < ind; i++)
      for (int a = vertexNumber[i], t = 0; a != vertexNumber[(i + 1) % ind]; a = (a + 1) % molSize, t++) {
         pp.push_back((p[i] * (edgeLenght[i] - t) + p[(i + 1)%ind] * t)/edgeLenght[i]);
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
         else if (dist < 1) result = max(result, 1/dist - 1);
      }

   //printf("%5.5f\n", result);
   return result + 1000.0 * add;

}

void layoutChain(int length, int *can_rotate, int *trans_cis_config, Vec2f *p, Vec2f end_point, double end_angle) {
   
}

double MoleculeLayoutMacrocycles::depictionMacrocycleMol(bool profi)
{
   const int max_size = 100;
//   const int molSize = length;

   //printf("Process started.\n");

   static signed char minRotates[max_size][max_size][2][max_size][max_size];
   //first : number of edge
   //second : summary angle of rotation (in PI/3 times)
   //third : last rotation is contraclockwise
   //fourth : x-coordinate
   //fifth : y-coordinate
   //value : minimum number of vertexes sticked out

   /*    int len = max_size * max_size * max_size * max_size * 2;
   signed char *y = ****minRotates;
   for (int i = 0; i < len; i++) y[i] = CHAR_MAX;*/

   int shift = 0;
   for (int i = 0; i < length; i++) 
         if (_edge_stereo[i] == 1) {
               shift = i;
               break;
         }
   int temporary[max_size];

   for (int i = 0; i < length; i++) 
      if (_vertex_weight[i] > 0) _vertex_weight[i]++;

   for (int i = 0; i < length; i++) temporary[i] = _vertex_weight[(i + shift) % length];
   for (int i = 0; i < length; i++) _vertex_weight[i] = temporary[i];
   for (int i = 0; i < length; i++) temporary[i] = _vertex_stereo[(i + shift) % length];
   for (int i = 0; i < length; i++) _vertex_stereo[i] = temporary[i];
   for (int i = 0; i < length; i++) temporary[i] = _edge_stereo[(i + shift) % length];
   for (int i = 0; i < length; i++) _edge_stereo[i] = temporary[i];

   const int init_x = max_size/2;
   const int init_y = max_size/2;
   const int init_rot = max_size/2 /6*6;

   for (int i = 0; i <= length; i++)
      for (int j = max(init_rot - length, 0); j < min(max_size, init_rot + length + 1); j++)
         for (int p = 0; p < 2; p++) 
            for (int k = max(init_x - length, 0); k < min(init_x + length + 1, max_size); k++)
               for (int t = max(init_y - length, 0); t < min(init_y + length + 1, max_size); t++)
                  minRotates[i][j][p][k][t] = CHAR_MAX;

   minRotates[0][init_rot][0][init_x][init_y] = 0;
   minRotates[1][init_rot][0][init_x + 1][init_y] = 0;
   minRotates[0][init_rot][1][init_x][init_y] = 0;
   minRotates[1][init_rot][1][init_x + 1][init_y] = 0;
   int dx[6];
   int dy[6];
   dx[0] = 1; dy[0] = 0;
   dx[1] = 0; dy[1] = 1;
   dx[2] = -1; dy[2] = 1;
   dx[3] = -1; dy[3] = 0;
   dx[4] = 0; dy[4] = -1;
   dx[5] = 1; dy[5] = -1;

   int max_dist = length;

   for (int k = 1; k < length; k++) {
      //printf("Step number %d\n", k);
      for (int rot = init_rot - length; rot <= init_rot + length; rot++) {
         if (!_vertex_stereo[k]) {
            int xchenge = dx[rot % 6];
            int ychenge = dy[rot % 6];
            for (int p = 0; p < 2; p++) {
               int x_start = max(init_x - max_dist, init_x - max_dist + xchenge);
               int x_finish = min(init_x + max_dist, init_x + max_dist + xchenge);
               int y_start = max(init_y - max_dist, init_y - max_dist + ychenge);
               int y_finish = min(init_y + max_dist, init_y + max_dist + ychenge);
               for (int x = x_start; x <= x_finish; x++) {
                  signed char *ar1 = minRotates[k + 1][rot][p][x + xchenge] + ychenge;
                  signed char *ar2 = minRotates[k][rot][p][x];
                  for (int y = y_start; y <= y_finish; y++) {
                     if (ar1[y] > ar2[y]) {
                        ar1[y] = ar2[y];
                     }
                  }
               }
            }
         } else {
            for (int p = 0; p < 2; p++) {
               // trying to rotate like CIS
               if (_edge_stereo[k - 1] != MoleculeCisTrans::TRANS) {
                  int nextRot = rot;
                  if (p) nextRot++;
                  else nextRot--;

                  int xchenge = dx[nextRot % 6];
                  int ychenge = dy[nextRot % 6];

                  int add = 0;
                  if (!p) add = _vertex_weight[k];

                  int x_start = max(init_x - max_dist, init_x - max_dist + xchenge);
                  int x_finish = min(init_x + max_dist, init_x + max_dist + xchenge);
                  int y_start = max(init_y - max_dist, init_y - max_dist + ychenge);
                  int y_finish = min(init_y + max_dist, init_y + max_dist + ychenge);
                  for (int x = x_start; x <= x_finish; x++) {
                     signed char *ar1 = minRotates[k + 1][nextRot][p][x + xchenge] + ychenge;
                     signed char *ar2 = minRotates[k][rot][p][x];
                     for (int y = y_start; y <= y_finish; y++) {
                        if (ar1[y] > ar2[y] + add) {
                           ar1[y] = ar2[y] + add;
                        }
                     }
                  }
               }
               // trying to rotate like TRANS
               if (_edge_stereo[k - 1] != MoleculeCisTrans::CIS) {
                  int nextRot = rot;
                  if (p) nextRot--;
                  else nextRot++;

                  int add = 0;
                  if (p) add = _vertex_weight[k];

                  int xchenge = dx[nextRot % 6];
                  int ychenge = dy[nextRot % 6];

                  int x_start = max(init_x - max_dist, init_x - max_dist + xchenge);
                  int x_finish = min(init_x + max_dist, init_x + max_dist + xchenge);
                  int y_start = max(init_y - max_dist, init_y - max_dist + ychenge);
                  int y_finish = min(init_y + max_dist, init_y + max_dist + ychenge);
                  for (int x = x_start; x <= x_finish; x++) {
                     signed char *ar1 = minRotates[k + 1][nextRot][p ^ 1][x + xchenge] + ychenge;
                     signed char *ar2 = minRotates[k][rot][p][x];
                     for (int y = y_start; y <= y_finish; y++) {
                        if (ar1[y] > ar2[y] + add) {
                           ar1[y] = ar2[y] + add;
                        }
                     }
                  }
               }
            }
         }
      }
   }

   for (int rot = max(init_rot - length, 0); rot < min(max_size, init_rot + length + 1); rot++)
      for (int p = 0; p < 2; p++) 
         for (int k = max(init_x - length, 0); k < min(init_x + length + 1, max_size); k++)
            for (int t = max(init_y - length, 0); t < min(init_y + length + 1, max_size); t++)
               if (minRotates[length][rot][p][k][t] != CHAR_MAX) {
                  if ((_edge_stereo[length - 1] == MoleculeCisTrans::TRANS && p) ||
                     (_edge_stereo[length - 1] == MoleculeCisTrans::CIS && !p) ||
                        rot > init_rot + 6) minRotates[length][rot][p][k][t] += _vertex_weight[0];
               }


   //printf("Process finished.\n");
   int best_p = 0;
   int best_x = 0;
   int best_y = 0;
   int best_rot = 0;
   int best_diff = 127 * 300;
   for (int rot = max(init_rot - length, 0); rot <= min(init_rot + length, max_size - 1); rot++) {
      for (int p = 0; p < 2; p++) {
         for (int x = max(init_x - length, 0); x <= min(init_x + length, max_size - 1); x++) {
            for (int y = max(init_y - length, 0); y <= min(init_y + length, max_size - 1); y++) {
               //if (rot == init_rot) printf("%d %d %d %d\n", rot, p, x, y);
               if (minRotates[length][rot][p][x][y] < CHAR_MAX) {
                  //printf("!!!");
                  int diffCoord;
                  int startx = init_x;
                  int starty = init_y;
/*                  if (rot % 6 == 1) {
                     startx--;
                     starty--;
                  }
                  if (rot % 6 == 5) {
                     startx -= 2;
                     starty++;
                  }*/
                  if ((x - startx) * (y - starty) >= 0) diffCoord = abs(x - startx) + abs(y - starty); // x and y both positive or negative, vector (y+x) is not neseccary
                  else diffCoord = max(abs(x - startx), abs(y - starty)); // x and y are has got different signs, vector (y-x) is neseccary
                  int diffRot;
                  //TODO: pay attantion to last edge trans-cis configuration
                  diffRot = abs(abs(rot - (init_rot + 6)) - 1);

                  int add = 0;

                  if (2*(diffRot + diffCoord) + (int)minRotates[length][rot][p][x][y] < best_diff) {
                     best_p = p;
                     best_x = x;
                     best_y = y;
                     best_rot = rot;
                     best_diff = 2*(diffRot + diffCoord) + (int)minRotates[length][rot][p][x][y];
                  }
               }
            }
         }
      }
   }

   vector<int> ps;
   vector<int> xs;
   vector<int> ys;
   vector<int> rots;
   vector<int> diffs;

   for (int global_diff = 0; global_diff <= 4; global_diff++) {
      for (int rot = max(init_rot - length, 0); rot <= min(init_rot + length, max_size - 1); rot++) {
         for (int p = 0; p < 2; p++) {
            for (int x = max(init_x - length, 0); x <= min(init_x + length, max_size - 1); x++) {
               for (int y = max(init_y - length, 0); y <= min(init_y + length, max_size - 1); y++) {
                  //if (rot == init_rot) printf("%d %d %d %d\n", rot, p, x, y);
                  if (minRotates[length][rot][p][x][y] < CHAR_MAX) {
                     //printf("!!!");
                     int diffCoord;
                     int startx = init_x;
                     int starty = init_y;
/*                     if (rot % 6 == 1) {
                        startx--;
                        starty--;
                     }
                     if (rot % 6 == 5) {
                        startx -= 2;
                        starty++;
                     }*/
                     if ((x - startx) * (y - starty) >= 0) diffCoord = abs(x - startx) + abs(y - starty); // x and y both positive or negative, vector (y+x) is not neseccary
                     else diffCoord = max(abs(x - startx), abs(y - starty)); // x and y are has got diggerent signs, vector (y-x) is neseccary
                     int diffRot;
                     //TODO: pay attantion to last edge trans-cis configuration
                     diffRot = abs(abs(rot - (init_rot + 6)) - 1);

                     if (2*(diffRot + diffCoord) + minRotates[length][rot][p][x][y] == best_diff + global_diff) {
                        xs.push_back(x);
                        ys.push_back(y);
                        ps.push_back(p);
                        rots.push_back(rot);
                        diffs.push_back(global_diff);
                     }
                  }
               }
            }
         }
      }
   }

   for (int rot = max(init_rot - length, 0); rot < min(max_size, init_rot + length + 1); rot++)
      for (int p = 0; p < 2; p++) 
         for (int k = max(init_x - length, 0); k < min(init_x + length + 1, max_size); k++)
            for (int t = max(init_y - length, 0); t < min(init_y + length + 1, max_size); t++)
               if (minRotates[length][rot][p][k][t] != CHAR_MAX) {
                  if ((_edge_stereo[length - 1] == MoleculeCisTrans::TRANS && p) ||
                     (_edge_stereo[length - 1] == MoleculeCisTrans::CIS && !p) ||
                        rot > init_rot + 6) minRotates[length][rot][p][k][t] -= _vertex_weight[0];
               }

//   printf("Best diff: %d\n", best_diff);
//   printf("Best position: %d %d %d %d\n", best_x, best_y, best_rot, best_p);
//   printf("Inside atoms in best case: %d\n", minRotates[length][best_rot][best_p][best_x][best_y]);

   int x_result[max_size + 1];
   int y_result[max_size + 1];
   int rot_result[max_size + 1];
   int p_result[max_size + 1];

   double bestBadness = 1e30;
   int bestIndex = 0;

//   printf("We will encounted %d variants\n", xs.size());

   int last_rotate_angle = 1;

//   printf("-- Start\n");
   
   for (int index = 0; index < xs.size() && bestBadness > 0.001; index++) {
   //for (int index = 0; index < xs.size(); index++) {
      int displayIndex = index;
      //printf("%d\n", index);
      x_result[length] = xs[index];
      y_result[length] = ys[index];
      rot_result[length] = rots[index];
      p_result[length] = ps[index];

      for (int k = length - 1; k > 0; k--) {
         //printf("k: %d\n", k);
         int xchenge = dx[rot_result[k + 1] % 6];
         int ychenge = dy[rot_result[k + 1] % 6];
         x_result[k] = x_result[k + 1] - xchenge;
         y_result[k] = y_result[k + 1] - ychenge;

         if (!_vertex_stereo[k]) {
            p_result[k] = p_result[k + 1];
            rot_result[k] = rot_result[k + 1];
            //printf("+");
         } else {
            if (p_result[k + 1]) rot_result[k] = rot_result[k + 1] - 1;
            else rot_result[k] = rot_result[k + 1] + 1;

            int add = 0;
            if (!p_result[k + 1]) add = _vertex_weight[k];
            if (_edge_stereo[k - 1] != MoleculeCisTrans::TRANS) {
               //if (minRotates[k][rot_result[k]][p_result[k + 1]][x_result[k]][y_result[k]] + 1 == minRotates[k + 1][rot_result[k + 1]][p_result[k + 1]][x_result[k + 1]][y_result[k + 1]]) {
               if (minRotates[k][rot_result[k]][p_result[k + 1]][x_result[k]][y_result[k]] + add == minRotates[k + 1][rot_result[k + 1]][p_result[k + 1]][x_result[k + 1]][y_result[k + 1]]) {
                  p_result[k] = p_result[k + 1];
                  //printf("+");
               }
            }
            //add = 0;
            //if (p_result[k + 1]) add = _vertex_weight[k];
            if (_edge_stereo[k - 1] != MoleculeCisTrans::CIS) {
               if (minRotates[k][rot_result[k]][p_result[k + 1] ^ 1][x_result[k]][y_result[k]] + add == minRotates[k + 1][rot_result[k + 1]][p_result[k + 1]][x_result[k + 1]][y_result[k + 1]]) {
                  p_result[k] = p_result[k + 1] ^ 1;
                  //printf("+");
               }
            }
         }
         //printf("\n");
         //printf("%d %d %d %d\n", x_result[k], y_result[k], rot_result[k], p_result[k]);
      }
      x_result[0] = init_x;
      y_result[0] = init_y;
      //for (int k = 1; k < length; k++) printf("%d %d %d %d %d %d %d\n", k, rot_result[k], p_result[k], _vertex_weight[k], x_result[k], y_result[k], (int)minRotates[k][rot_result[k]][p_result[k]][x_result[k]][y_result[k]]);

      int rotateAngle[max_size];
      int edgeLenght[max_size];
      int vertexNumber[max_size];

      int ind = 0;
      for (int i = 0; i < length; i++) {
         if (_vertex_stereo[i]) vertexNumber[ind++] = i;
      }

      if (ind < 3) {
         ind = 0;
         for (int i = 0; i < length; i++) vertexNumber[ind++] = i;
      }

      for (int i = 0; i < ind - 1; i++) edgeLenght[i] = vertexNumber[i + 1] - vertexNumber[i];
      edgeLenght[ind - 1] = vertexNumber[0] - vertexNumber[ind - 1] + length;

      for (int i = 0; i < ind; i++) 
         if (vertexNumber[i] != 0) {
            rotateAngle[i] = rot_result[vertexNumber[i] + 1] > rot_result[vertexNumber[i]] ? 1 : rot_result[vertexNumber[i] + 1] == rot_result[vertexNumber[i]] ? 0 : -1;
         }
      if (vertexNumber[0] == 0) {
         if (!_vertex_stereo[0]) rotateAngle[0] = 0;
         else if (_edge_stereo[0] == MoleculeCisTrans::TRANS) rotateAngle[0] = - rotateAngle[1];
         else if (_edge_stereo[0] == MoleculeCisTrans::CIS) rotateAngle[0] = rotateAngle[1];
         else if (_edge_stereo[length - 1] == MoleculeCisTrans::TRANS) rotateAngle[0] = - rotateAngle[ind - 1];
         else if (_edge_stereo[length - 1] == MoleculeCisTrans::CIS) rotateAngle[0] = rotateAngle[ind - 1];
         else if (last_rotate_angle == 1) {
            rotateAngle[0] = 1;
            last_rotate_angle = -1;
            index--;
         } else {
            rotateAngle[0] = -1;
            last_rotate_angle = 1;
         }
      }

      double x[max_size];
      double y[max_size];
      Vec2f p[max_size];
      for (int i = 0; i < ind; i++) {
         p[i] = Vec2f(y_result[vertexNumber[i]], 0);
         p[i].rotate(PI/3);
         p[i] += Vec2f(x_result[vertexNumber[i]], 0);
      }

      double startBadness = badness(ind, length, rotateAngle, edgeLenght, vertexNumber, p);
      if (startBadness > 0.001) smoothing(ind, length, rotateAngle, edgeLenght, vertexNumber, p, profi);

      double newBadness = 0;
      newBadness = badness(ind, length, rotateAngle, edgeLenght, vertexNumber, p);
      //printf("%10.10f\n", newBadness);

      //printf("-- %d %d %d %5.5f %5.5f\n", xs[displayIndex], ys[displayIndex], rots[displayIndex], startBadness, newBadness);


      if (newBadness < bestBadness) {
         bestBadness = newBadness;
         //printf("New best badness: %5.5f\n", bestBadness);
         bestIndex = displayIndex;

         for (int i = 0; i < ind; i++) {
            int nexti = (i + 1) % ind;

            for (int j = vertexNumber[i], t = 0; j != vertexNumber[nexti]; j = (j + 1) % length, t++) {
               _positions[j] = (p[i] * (edgeLenght[i] - t) + p[nexti] * t) / edgeLenght[i];
            }
         }
      }

   }

   Vec2f shifted_positons[max_size];
   for (int i = 0; i < length; i++) shifted_positons[(i + shift) % length] = _positions[i];
   for (int i = 0; i < length; i++) _positions[i] = shifted_positons[i];

   for (int i = 0; i < length; i++) temporary[(i + shift) % length] = _vertex_weight[i];
   for (int i = 0; i < length; i++) _vertex_weight[i] = temporary[i];
   for (int i = 0; i < length; i++) temporary[(i + shift) % length] = _vertex_stereo[i];
   for (int i = 0; i < length; i++) _vertex_stereo[i] = temporary[i];
   for (int i = 0; i < length; i++) temporary[(i + shift) % length] = _edge_stereo[i];
   for (int i = 0; i < length; i++) _edge_stereo[i] = temporary[i];

   for (int i = 0; i < length; i++) 
      if (_vertex_weight[i] > 0) _vertex_weight[i]--;


   //fclose (stdout);
//   printf("%5.5f\n", bestBadness);
//   printf("--- %d\n", bestIndex);
   //    printf("\n");

   //    for (int i = 0; i < ind; i++) printf("%10.10f %10.10f\n", x[i], y[i]);
   //    printf("\n");

//   printf("\n");
   // Saved changed into molfile
   //    indigoSaveMolfileToFile(m, "builded_molecule2.mol");
   // Render changed
   //indigoSetOption("render-output-format", "png");
   //indigoSetOption("render-background-color", "255, 255, 255");
   //    indigoRenderToFile(m, "builded_molecule.png"); 

   return bestBadness;
}

double MoleculeLayoutMacrocycles::depictionCircle() {

   const int max_size = 100;
//   const int molSize = mol.size;

   //printf("Process started.\n");

/*   vector<int> vert;
   vert.push_back(mol.vertexBegin());
   for (int i = 0; i < molSize - 1; i++) {
      int index = mol.getVertex(vert[i]).neiBegin();
      int e = mol.getVertex(vert[i]).neiEdge(index);
      // to select next vertex not equals to previous
      if (i != 0 && mol.getVertex(vert[i]).neiVertex(index) == vert[i - 1]) index = mol.getVertex(vert[i]).neiNext(index);
      // to start chain of atoms not with cis-trans bond
      if (i == 0 && mol.cis_trans.getParity(e) != 0) index = mol.getVertex(vert[i]).neiNext(index);

      e = mol.getVertex(vert[i]).neiEdge(index);

      vert.push_back(mol.getVertex(e).neiVertex(index));
   }*/

//   vector<int> order;
  // for (int i = 0; i < length; i++) order.push_back(mol.order[i]);
/*   {
      switch (mol.getBondOrder(mol.findEdgeIndex(vert[i], vert[(i + 1) % molSize]))) {
         case BOND_SINGLE : order.push_back(1); break;
         case BOND_DOUBLE : order.push_back(2); break;
         case BOND_TRIPLE : order.push_back(3); break;
         case BOND_AROMATIC : order.push_back(1); break;
      }
   }*/

//   vector<int> cisTransConf;
  // for (int i = 0; i < length; i++) cisTransConf.push_back(mol.cis_trans[i]);
/*   {
      int e = mol.findEdgeIndex(vert[i], vert[(i + 1) % molSize]);
      cisTransConf.push_back(mol.cis_trans.getParity(e));
   }*/
   
   int cisCount = 0;
   for (int i = 0; i < length; i++) if (_edge_stereo[i] == MoleculeCisTrans::CIS) cisCount++;

   bool up[100];
   for (int i = 0; i < length; i++)
   {
      if (_edge_stereo[i] == MoleculeCisTrans::CIS) up[i + 1] = up[i];
      else up[i + 1] = !up[i];
   }
   
   if ((cisCount + length) % 2 == 0)
   {
      // if first and last points on the same level
      int upCisCount = 0;
      int downCisCount = 0;

      for (int i = 0; i < length; i++)
      {
         if (_edge_stereo[i] == MoleculeCisTrans::CIS && up[i]) upCisCount++;
         if (_edge_stereo[i] == MoleculeCisTrans::CIS && !up[i]) downCisCount++;
      }
      if (downCisCount > upCisCount) {
         for (int i = 0; i <= length; i++) up[i] = !up[i];
      }
   } else {
      // if first and last points on the different levels
      if (cisCount == 0)
      {
         int index = 0;
         if (_edge_stereo[0] != 0 || _edge_stereo[length - 1] != 0 || _vertex_stereo[0] == 0) {
            for (int i = 1; i < length; i++) if (_edge_stereo[i] != 0 || _edge_stereo[i - 1] != 0 || _vertex_stereo[i] == 1) index = i;
         }
         for (int i = index; i <= length; i++) up[i] = !up[i];
         if (!up[index]) for (int i = 0; i <= length; i++) up[i] = !up[i];
      } else {
         int bestIndex = 0;
         int bestDiff = -1;
         for (int i = 0; i < length; i++) if (_edge_stereo[i] == MoleculeCisTrans::CIS || _edge_stereo[(i - 2 + length) % length] == MoleculeCisTrans::CIS){
            int diff = 0;
            for (int j = 0; j < length; j++) 
            {
               if (_edge_stereo[i] == MoleculeCisTrans::CIS && ((up[i] && j < i) || (!up[i] && j >= i))) diff++;
               if (_edge_stereo[i] == MoleculeCisTrans::CIS && !((up[i] && j < i) || (!up[i] && j >= i))) diff--;
            }
            if (up[i]) diff = -diff;
            if (diff > bestDiff) 
            {
               bestDiff = diff;
               bestIndex = i;
            }
         }

         for (int i = bestIndex; i <= length; i++) up[i] = !up[i];
         
         if (!up[bestIndex]) {
            for (int i = 0; i <= length; i++) up[i] = !up[i];
         }
      }
   }

   double r = length * sqrt(3.0)/2 / (2 * PI);

   double x[100];
   double y[100];
   Vec2f p[100];

   for (int i = 0; i < length; i++) {
      double rr = r;
      if (up[i]) rr += 0.25;
      else rr -= 0.25;

      p[i] = Vec2f(rr, 0);
      p[i].rotate(2*PI/length*i);
   }

   int rotateAngle[100];
   for (int i = 0; i < length; i++) rotateAngle[i] = up[i] ? 1 : -1;

   int edgeLength[100];
   for (int i = 0; i < length; i++) edgeLength[i] = 1;

   int vertexNumber[100];
   for (int i = 0; i < length; i++) vertexNumber[i] = i;

   /*double angle = PI * 2 * rand() / RAND_MAX;
   double sn = sin(angle);
   double cs = cos(angle);
   for (int i = 0; i < molSize; i++) {
      double xx = cs * x[i] - sn * y[i];
      double yy = sn * x[i] + cs * y[i];
      x[i] = xx;
      y[i] = yy;
   }*/

   smoothing(length, length, rotateAngle, edgeLength, vertexNumber, p, false);

   
   for (int i = 0; i < length; i++)
   {
/*      Vec3f &pos = mol.getAtomXyz(vert[i]);
      pos.x = x[i];
      pos.y = y[i];
      pos.z = 0;*/
//      _positions[i].x = x[i];
  //    _positions[i].y = y[i];
      _positions[i] = p[i];
   }

   return badness(length, length, rotateAngle, edgeLength, vertexNumber, p);
}
