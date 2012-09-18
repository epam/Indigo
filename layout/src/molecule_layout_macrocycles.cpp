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

double depictionMacrocycleMol(BaseMolecule &mol, bool profi);

double MoleculeLayoutMacrocycles::layout (BaseMolecule &mol)
{
   profTimerStart(t, "bc.layout");
   double b = depictionMacrocycleMol(mol, false);
   if (b >= 1000000) {
      profTimerStart(t, "bc.layout #2");
      b = depictionMacrocycleMol(mol, true);
   }
   if (b >= 1000000)
      throw Error("Cannot find a layout with all cis-trans constraints");

   return b;
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

using namespace std;


double sqr(double x) {return x*x;}

int isIntersec(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4) {
   double s1 = (x3 - x1) * (y4 - y1) - (y3 - y1) * (x4 - x1);
   double s2 = (x3 - x2) * (y4 - y2) - (y3 - y2) * (x4 - x2);
   double s3 = (x1 - x3) * (y2 - y3) - (y1 - y3) * (x2 - x3);
   double s4 = (x1 - x4) * (y2 - y4) - (y1 - y4) * (x2 - x4);

   return s1 * s2 <= 0 && s3 * s4 <= 0;
}

double distPP(double x1, double y1, double x2, double y2) {return sqrt(sqr(x1 - x2) + sqr(y1 - y2));}

double distPL(double x1, double y1, double x2, double y2, double x3, double y3) {
   double phi = rand();
   double sn = sin(phi);
   double cs = cos(phi);

   double xx;
   double yy;
   xx = x1*cs - y1*sn;
   yy = y1*cs + x1*sn;
   x1 = xx;
   y1 = yy;
   xx = x2*cs - y2*sn;
   yy = y2*cs + x2*sn;
   x2 = xx;
   y2 = yy;
   xx = x3*cs - y3*sn;
   yy = y3*cs + x3*sn;
   x3 = xx;
   y3 = yy;

   double a = y2 - y3;
   double b = x3 - x2;
   double c = x2*y3 - x3*y2;
   double s = sqrt(a*a + b*b);
   a /= s;
   b /= s;
   c /= s;

   double t = - c - a*x1 - b*y1;

   xx = x1 + t*a;
   yy = y1 + t*b;

   if (x2 <= xx && xx <= x3 || x3 <= xx && xx <= x2) return abs(t);
   return min(distPP(x1, y1, x2, y2), distPP(x1, y1, x3, y3));
}

double distLL(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4) {
   if (isIntersec(x1, y1, x2, y2, x3, y3, x4, y4)) {
      //printf("%5.5f %5.5f %5.5f %5.5f %5.5f %5.5f %5.5f %5.5f\n", x1, y1, x2, y2, x3, y3, x4, y4);
      return 0;
   }

   return min( min(distPL(x1, y1, x3, y3, x4, y4), distPL(x2, y2, x3, y3, x4, y4)),
      min(distPL(x3, y3, x1, y1, x2, y2), distPL(x4, y4, x1, y1, x2, y2)));
}

void improvement(int ind, int molSize, int *rotateAngle, int *edgeLenght, int *vertexNumber, double *x, double *y, bool profi) {
   int worstVertex = rand() % ind;

   double x1 = x[(worstVertex - 1 + ind) % ind];
   double y1 = y[(worstVertex - 1 + ind) % ind];
   double x2 = x[(worstVertex + 1 + ind) % ind]; 
   double y2 = y[(worstVertex + 1 + ind) % ind]; 
   double r1 = edgeLenght[(ind + worstVertex - 1) % ind];
   double r2 = edgeLenght[(ind + worstVertex) % ind];


   double len1 = sqrt(sqr(x[worstVertex] - x1) + sqr(y[worstVertex] - y1));
   double len2 = sqrt(sqr(x[worstVertex] - x2) + sqr(y[worstVertex] - y2));

   double r3 = sqrt((sqr(x1 - x2) + sqr(y1 - y2))/3.0);

   double x3 = (x1 + x2)/2;
   double y3 = (y1 + y2)/2;

   if (rotateAngle[worstVertex] == 1) {
      x3 -= (y2 - y1)/sqrt(12.0);
      y3 += (x2 - x1)/sqrt(12.0);
   } else if (rotateAngle[worstVertex] == -1) {
      x3 += (y2 - y1)/sqrt(12.0);
      y3 -= (x2 - x1)/sqrt(12.0);
   } else {
      x3 = (r1*x1 + r2*x2)/(r1 + r2);
      y3 = (r1*y1 + r2*y2)/(r1 + r2);
   }

   double len3 = sqrt(sqr(x[worstVertex] - x3) + sqr(y[worstVertex] - y3));
   if (rotateAngle[worstVertex] == 0) r3 = 0;

   //printf("%5.5f %5.5f %5.5f %5.5f\n", len1, len2, len3, r3);
   Vec3f newPoint;
   double eps = 1e-4;
   if (len1 < eps || len2 < eps || len3 < eps) {
      x[worstVertex] = (x1 + x2)/2;
      y[worstVertex] = (y1 + y2)/2;
   } else {
      double coef1 = (r1/len1 - 1);
      double coef2 = (r2/len2 - 1);
      double coef3 = (r3/len3 - 1);

      //if (!isIntersec(x[worstVertex], y[worstVertex], x3, y3, x1, y1, x2, y2)) coef3 *= 10;
      if (rotateAngle[worstVertex] == 0) coef3 = -1;
      //printf("%5.5f %5.5f %5.5f\n", coef1, coef2, coef3);
      newPoint.add(Vec3f((x[worstVertex] - x1)*coef1, (y[worstVertex] - y1)*coef1, 0));
      newPoint.add(Vec3f((x[worstVertex] - x2)*coef2, (y[worstVertex] - y2)*coef2, 0));
      newPoint.add(Vec3f((x[worstVertex] - x3)*coef3, (y[worstVertex] - y3)*coef3, 0));

      if (profi) {
         for (int i = 0; i < ind; i++) if (i != worstVertex && (i + 1) % ind != worstVertex) {
            double dist = distPL(x[worstVertex], y[worstVertex], x[i], y[i], x[(i + 1)%ind], y[(i + 1)%ind]);
            if (dist < 1 && dist > eps) {
               double a = y[i] - y[(i + 1)%ind];
               double b = x[(i + 1)%ind] - x[i];
               double c = x[i]*y[(i + 1)%ind] - x[(i + 1)%ind]*y[i];
               double s = sqrt(a*a + b*b);
               a /= s;
               b /= s;
               c /= s;

               double t = - c - x[worstVertex] * a - y[worstVertex] * b;

               double xx = x[worstVertex] + t*a;
               double yy = y[worstVertex] + t*b;
               if (distPP(x[worstVertex], y[worstVertex], x[i], y[i]) < distPP(x[worstVertex], y[worstVertex], xx, yy)) {
                  xx = x[i];
                  yy = y[i];
               }
               if (distPP(x[worstVertex], y[worstVertex], x[(i + 1)%ind], y[(i + 1)%ind]) < distPP(x[worstVertex], y[worstVertex], xx, yy)) {
                  xx = x[(i + 1)%ind];
                  yy = y[(i + 1)%ind];
               }

               double coef = (1 - dist)/dist;

               newPoint.add(Vec3f((x[worstVertex] - xx)*coef, (y[worstVertex] - yy)*coef, 0));
            }
         }
      } else {
         for (int j = 0; j < ind; j++) {
            int nextj = (j + 1) % ind;
            double xx = x[j];
            double yy = y[j];
            double dxx = (x[nextj] - x[j])/edgeLenght[j];
            double dyy = (y[nextj] - y[j])/edgeLenght[j];
            for (int t = vertexNumber[j], s = 0; t != vertexNumber[nextj]; t = (t + 1)%molSize, s++) {
               if (t != vertexNumber[worstVertex] && (t + 1)%molSize != vertexNumber[worstVertex] && t != (vertexNumber[worstVertex] + 1) % molSize) {
                  double dist = 0;
                  double sqrt2 = sqrt(2.0);
                  //if (abs(xx - x[worstVertex]) <= sqrt2 && abs(yy - y[worstVertex]) <= sqrt2) dist = distPP(xx, yy, x[worstVertex], y[worstVertex]);
                  //else dist = 1000;
                  dist = distPP(xx, yy, x[worstVertex], y[worstVertex]);
                  if (dist < sqrt2 && dist > eps) {
                     double coef = (sqrt2 - dist)/dist;
                     //printf("%5.5f \n", dist);
                     newPoint.add(Vec3f((x[worstVertex] - xx)*coef, (y[worstVertex] - yy)*coef, 0));
                  }
               }
               xx += dxx;
               yy += dyy;
            }
         }
      }

      newPoint.scale(0.01);

      x[worstVertex] += newPoint.x;
      y[worstVertex] += newPoint.y;
   }

}

void MoleculeLayoutMacrocycles::smoothing(int ind, int molSize, int *rotateAngle, int *edgeLenght, int *vertexNumber, double *x, double *y, bool profi) {
   for (int i = 0; i < 10000; i++) improvement(ind, molSize, rotateAngle, edgeLenght, vertexNumber, x, y, profi);
}

double MoleculeLayoutMacrocycles::badness(int ind, int molSize, int *rotateAngle, int *edgeLenght, int *vertexNumber, double *x, double *y) {
   double eps = 1e-9;
   double result = 0;
   int add = 0;
   for (int i = 0; i < ind; i++) {
      double len = sqrt(sqr(x[i] - x[(i + 1)%ind]) + sqr(y[i] - y[(i + 1)%ind]))/edgeLenght[i];
      if (len < eps) add++;
      else if (len < 1) result = max(result, (1/len - 1));
      else result = max(result, len - 1);
   }
   for (int i = 0; i < ind; i++) {
      double vx1 = x[i] - x[(i + ind - 1) % ind];
      double vy1 = y[i] - y[(i + ind - 1) % ind];
      double vx2 = x[(i + 1) % ind] - x[i];
      double vy2 = y[(i + 1) % ind] - y[i];
      double len1 = sqrt(vx1 * vx1 + vy1 * vy1);
      double len2 = sqrt(vx2 * vx2 + vy2 * vy2);
      vx1 /= len1;
      vx2 /= len2;
      vy1 /= len1;
      vy2 /= len2;

      double angle = acos(vx1 * vx2 + vy1 * vy2);
      if (vy1 * vx2 - vy2 * vx1 > 0) angle = -angle;
      angle /= (PI/3);
      if (angle * rotateAngle[i] <= 0) add += 1000;
      else if (abs(angle) > 1) result = max(result, abs(angle - rotateAngle[i])/2);
      else result = max(result, abs(1/angle - rotateAngle[i])/2);
   }


   vector<double> xx;
   vector<double> yy;
   for (int i = 0; i < ind; i++)
      for (int a = vertexNumber[i], t = 0; a != vertexNumber[(i + 1) % ind]; a = (a + 1) % molSize, t++) {
         xx.push_back((x[i] * (edgeLenght[i] - t) + x[(i + 1)%ind] * t)/edgeLenght[i]);
         yy.push_back((y[i] * (edgeLenght[i] - t) + y[(i + 1)%ind] * t)/edgeLenght[i]);
      }

      int size = xx.size();
      for (int i = 0; i < size; i++)
         for (int j = 0; j < size; j++) if (abs((i - j + size + size/2) % size - size/2) >= 2) {
            int nexti = (i + 1) % size;
            int nextj = (j + 1) % size;
            double dist = distLL(xx[i], yy[i], xx[nexti], yy[nexti], xx[j], yy[j], xx[nextj], yy[nextj]);

            if (abs(dist) < eps) {
               add++;
               //printf("%5.5f %5.5f %5.5f %5.5f %5.5f %5.5f %5.5f %5.5f \n", xx[i], yy[i], xx[nexti], yy[nexti], xx[j], yy[j], xx[nextj], yy[nextj]);
            }
            else if (dist < sqrt(2.0)) result = max(result, sqrt(2.0)/dist - 1);
         }

         //printf("%5.5f\n", result);
         return result + 1000.0 * add;

}

double MoleculeLayoutMacrocycles::depictionMacrocycleMol(BaseMolecule &mol, bool profi)
{
   const int max_size = 100;
   const int molSize = mol.vertexCount();

   printf("Process started.\n");

   vector<int> vert;
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
   }

   vector<int> order;
   for (int i = 0; i < molSize; i++) {
      switch (mol.getBondOrder(mol.findEdgeIndex(vert[i], vert[(i + 1) % molSize]))) {
      case BOND_SINGLE : order.push_back(1); break;
      case BOND_DOUBLE : order.push_back(2); break;
      case BOND_TRIPLE : order.push_back(3); break;
      case BOND_AROMATIC : order.push_back(1); break;
      }
   }

   vector<int> cisTransConf;
   for (int i = 0; i < molSize; i++) {
      int e = mol.findEdgeIndex(vert[i], vert[(i + 1) % molSize]);
      cisTransConf.push_back(mol.cis_trans.getParity(e));
   }


   static signed char minRotates[max_size][max_size][2][max_size][max_size];
   //first : number of edge
   //second : summary angle of rotation (in PI/3 times)
   //third : last rotation is contraclockwise
   //fourth : x-coordinate
   //fifth : y-coordinate
   //value : minimum number of pair of consequence same-side rotations

   /*    int len = max_size * max_size * max_size * max_size * 2;
   signed char *y = ****minRotates;
   for (int i = 0; i < len; i++) y[i] = CHAR_MAX;*/

   const int init_x = max_size/2;
   const int init_y = max_size/2;
   const int init_rot = max_size/2 /6*6;

   for (int i = 0; i <= molSize; i++)
      for (int j = max(init_rot - molSize, 0); j < min(max_size, init_rot + molSize + 1); j++)
         for (int p = 0; p < 2; p++) 
            for (int k = max(init_x - molSize, 0); k < min(init_x + molSize + 1, max_size); k++)
               for (int t = max(init_y - molSize, 0); t < min(init_y + molSize + 1, max_size); t++)
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

   int max_dist = molSize;

   for (int k = 1; k < molSize; k++) {
      //printf("Step number %d\n", k);
      for (int rot = init_rot - molSize; rot <= init_rot + molSize; rot++) {
         if (order[k] + order[k - 1] >= 4) {
            int xchenge = dx[rot % 6];
            int ychenge = dy[rot % 6];
            for (int p = 0; p < 2; p++) {
               int x_start = max(init_x - max_dist, init_x - max_dist + xchenge);
               int x_finish = min(init_x + max_dist, init_x + max_dist + xchenge);
               int y_start = max(init_y - max_dist, init_y - max_dist + ychenge);
               int y_finish = min(init_y + max_dist, init_y + max_dist + ychenge);
               for (int x = x_start; x <= x_finish; x++) {
                  signed char *ar1 = *(*(*(*(minRotates + k + 1) + rot) + p) + x + xchenge);
                  //signed char *ar2 = *(*(*(*(minRotates + k) + rot) + p) + x);
                  signed char *ar2 = minRotates[k][rot][p][x];
                  for (int y = y_start; y <= y_finish; y++) {
                     //if (minRotates[k][rot][p][x][y] < CHAR_MAX) printf("%d %d %d\n", minRotates[k][rot][p][x][y], x, y);
                     /*                            if (minRotates[k + 1][rot][p][x + xchenge][y + ychenge] > minRotates[k][rot][p][x][y]) {
                     minRotates[k + 1][rot][p][x + xchenge][y + ychenge] = minRotates[k][rot][p][x][y];
                     }*/
                     if (ar1[y + ychenge] > ar2[y]) {
                        ar1[y + ychenge] = ar2[y];
                     }
                  }
               }
            }
         } else {
            for (int p = 0; p < 2; p++) {
               // trying to rotate like CIS
               if (cisTransConf[k - 1] != MoleculeCisTrans::TRANS) {
                  int nextRot = rot;
                  if (p) nextRot++;
                  else nextRot--;

                  int xchenge = dx[nextRot % 6];
                  int ychenge = dy[nextRot % 6];

                  int x_start = max(init_x - max_dist, init_x - max_dist + xchenge);
                  int x_finish = min(init_x + max_dist, init_x + max_dist + xchenge);
                  int y_start = max(init_y - max_dist, init_y - max_dist + ychenge);
                  int y_finish = min(init_y + max_dist, init_y + max_dist + ychenge);
                  for (int x = x_start; x <= x_finish; x++) {
                     signed char *ar1 = *(*(*(*(minRotates + k + 1) + nextRot) + p) + x + xchenge);
                     signed char *ar2 = *(*(*(*(minRotates + k) + rot) + p) + x);
                     for (int y = y_start; y <= y_finish; y++) {
                        /*                                if (minRotates[k + 1][nextRot][p][x + xchenge][y + ychenge] > 1 + minRotates[k][rot][p][x][y]) {
                        minRotates[k + 1][nextRot][p][x + xchenge][y + ychenge] = 1 + minRotates[k][rot][p][x][y];
                        }*/
                        if (ar1[y + ychenge] > 1 + ar2[y]) {
                           ar1[y + ychenge] = 1 + ar2[y];
                        }
                     }
                  }
               }
               // trying to rotate like TRANS
               if (cisTransConf[k - 1] != MoleculeCisTrans::CIS) {
                  int nextRot = rot;
                  if (p) nextRot--;
                  else nextRot++;

                  int xchenge = dx[nextRot % 6];
                  int ychenge = dy[nextRot % 6];

                  int x_start = max(init_x - max_dist, init_x - max_dist + xchenge);
                  int x_finish = min(init_x + max_dist, init_x + max_dist + xchenge);
                  int y_start = max(init_y - max_dist, init_y - max_dist + ychenge);
                  int y_finish = min(init_y + max_dist, init_y + max_dist + ychenge);
                  for (int x = x_start; x <= x_finish; x++) {
                     signed char *ar1 = *(*(*(*(minRotates + k + 1) + nextRot) + (p ^ 1)) + x + xchenge);
                     signed char *ar2 = *(*(*(*(minRotates + k) + rot) + p) + x);
                     for (int y = y_start; y <= y_finish; y++) {
                        /*                                if (minRotates[k + 1][nextRot][p ^ 1][x + xchenge][y + ychenge] > minRotates[k][rot][p][x][y]) {
                        minRotates[k + 1][nextRot][p ^ 1][x + xchenge][y + ychenge] = minRotates[k][rot][p][x][y];
                        }*/
                        if (ar1[y + ychenge] > ar2[y]) {
                           ar1[y + ychenge] = ar2[y];
                        }
                     }
                  }
               }
            }
         }
      }
   }

   printf("Process finished.\n");
   int best_p = 0;
   int best_x = 0;
   int best_y = 0;
   int best_rot = 0;
   int best_diff = 127 * 300;
   for (int rot = max(init_rot - molSize, 0); rot <= min(init_rot + molSize, max_size - 1); rot++) {
      for (int p = 0; p < 2; p++) {
         for (int x = init_x - molSize/2; x <= init_x + molSize/2 + 5; x++) {
            for (int y = init_y - molSize/2; y <= init_y + molSize/2 + 5; y++) {
               //if (rot == init_rot) printf("%d %d %d %d\n", rot, p, x, y);
               if (minRotates[molSize][rot][p][x][y] < CHAR_MAX) {
                  //printf("!!!");
                  int diffCoord;
                  int startx = init_x;
                  int starty = init_y;
                  if (rot % 6 == 1) {
                     startx--;
                     starty--;
                  }
                  if (rot % 6 == 5) {
                     startx -= 2;
                     starty++;
                  }
                  if ((x - startx) * (y - starty) >= 0) diffCoord = abs(x - startx) + abs(y - starty); // x and y both positive or negative, vector (y-x) is not neseccary
                  else diffCoord = min(abs(x - startx), abs(y - starty)) + abs((x - startx) - (y - starty)); // x and y are has got diggerent signs, vector (y-x) is neseccary
                  int diffRot;
                  //TODO: pay attantion to last edge trans-cis configuration
                  diffRot = abs(abs(rot - (init_rot + 6)) - 1);

                  if (diffRot + diffCoord < best_diff) {
                     best_p = p;
                     best_x = x;
                     best_y = y;
                     best_rot = rot;
                     best_diff = diffRot + diffCoord;
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

   for (int global_diff = 0; global_diff <= 4; global_diff++) {
      for (int rot = max(init_rot - molSize, 0); rot <= min(init_rot + molSize, max_size - 1); rot++) {
         for (int p = 0; p < 2; p++) {
            for (int x = init_x - molSize/2; x <= init_x + molSize/2 + 5; x++) {
               for (int y = init_y - molSize/2; y <= init_y + molSize/2 + 5; y++) {
                  //if (rot == init_rot) printf("%d %d %d %d\n", rot, p, x, y);
                  if (minRotates[molSize][rot][p][x][y] < CHAR_MAX) {
                     //printf("!!!");
                     int diffCoord;
                     int startx = init_x;
                     int starty = init_y;
                     if (rot % 6 == 1) {
                        startx--;
                        starty--;
                     }
                     if (rot % 6 == 5) {
                        startx -= 2;
                        starty++;
                     }
                     if ((x - startx) * (y - starty) >= 0) diffCoord = abs(x - startx) + abs(y - starty); // x and y both positive or negative, vector (y-x) is not neseccary
                     else diffCoord = min(abs(x - startx), abs(y - starty)) + abs((x - startx) - (y - starty)); // x and y are has got diggerent signs, vector (y-x) is neseccary
                     int diffRot;
                     //TODO: pay attantion to last edge trans-cis configuration
                     diffRot = abs(abs(rot - (init_rot + 6)) - 1);

                     if (diffRot + diffCoord == best_diff + global_diff) {
                        xs.push_back(x);
                        ys.push_back(y);
                        ps.push_back(p);
                        rots.push_back(rot);
                     }
                  }
               }
            }
         }
      }
   }

   printf("Best diff: %d\n", best_diff);
   printf("Best position: %d %d %d %d\n", best_x, best_y, best_rot, best_p);
   printf("Double-Rotates in best case: %d\n", minRotates[molSize][best_rot][best_p][best_x][best_y]);

   int x_result[max_size + 1];
   int y_result[max_size + 1];
   int rot_result[max_size + 1];
   int p_result[max_size + 1];

   double bestBadness = 1e30;
   int bestIndex = 0;

   printf("We will encounted %d variants\n", xs.size());

   int last_rotate_angle = 1;


   for (int index = 0; index < xs.size() && bestBadness > 0.5; index++) {
      int displayIndex = index;
      //printf("%d\n", index);
      x_result[molSize] = xs[index];
      y_result[molSize] = ys[index];
      rot_result[molSize] = rots[index];
      p_result[molSize] = ps[index];

      for (int k = molSize - 1; k > 0; k--) {
         //printf("k: %d\n", k);
         int xchenge = dx[rot_result[k + 1] % 6];
         int ychenge = dy[rot_result[k + 1] % 6];
         x_result[k] = x_result[k + 1] - xchenge;
         y_result[k] = y_result[k + 1] - ychenge;

         if (order[k] + order[k - 1] >= 4) {
            p_result[k] = p_result[k + 1];
            rot_result[k] = rot_result[k + 1];
            //printf("+");
         } else {
            if (p_result[k + 1]) rot_result[k] = rot_result[k + 1] - 1;
            else rot_result[k] = rot_result[k + 1] + 1;

            if (cisTransConf[k - 1] != MoleculeCisTrans::CIS) {
               if (minRotates[k][rot_result[k]][p_result[k + 1] ^ 1][x_result[k]][y_result[k]] == minRotates[k + 1][rot_result[k + 1]][p_result[k + 1]][x_result[k + 1]][y_result[k + 1]]) {
                  p_result[k] = p_result[k + 1] ^ 1;
                  //printf("+");
               }
            }
            if (cisTransConf[k - 1] != MoleculeCisTrans::TRANS) {
               if (minRotates[k][rot_result[k]][p_result[k + 1]][x_result[k]][y_result[k]] + 1 == minRotates[k + 1][rot_result[k + 1]][p_result[k + 1]][x_result[k + 1]][y_result[k + 1]]) {
                  p_result[k] = p_result[k + 1];
                  //printf("+");
               }
            }
         }
         //printf("\n");
         //printf("%d %d %d %d\n", x_result[k], y_result[k], rot_result[k], p_result[k]);
      }
      x_result[0] = init_x;
      y_result[0] = init_y;

      int rotateAngle[max_size];
      int edgeLenght[max_size];
      int vertexNumber[max_size];

      int ind = 0;
      for (int i = 0; i < molSize; i++) {
         if (order[i] + order[(molSize + i - 1) % molSize] <= 3) vertexNumber[ind++] = i;
      }

      if (ind < 3) {
         ind = 0;
         for (int i = 0; i < molSize; i++) vertexNumber[ind++] = i;
      }

      for (int i = 0; i < ind - 1; i++) edgeLenght[i] = vertexNumber[i + 1] - vertexNumber[i];
      edgeLenght[ind - 1] = vertexNumber[0] - vertexNumber[ind - 1] + molSize;

      for (int i = 0; i < ind; i++) 
         if (vertexNumber[i] != 0) {
            rotateAngle[i] = rot_result[vertexNumber[i] + 1] > rot_result[vertexNumber[i]] ? 1 : rot_result[vertexNumber[i] + 1] == rot_result[vertexNumber[i]] ? 0 : -1;
         }
         if (vertexNumber[0] == 0) {
            if (order[0] + order[molSize - 1] >= 4) rotateAngle[0] = 0;
            else if (cisTransConf[0] == MoleculeCisTrans::TRANS) rotateAngle[0] = - rotateAngle[1];
            else if (cisTransConf[0] == MoleculeCisTrans::CIS) rotateAngle[0] = rotateAngle[1];
            else if (cisTransConf[molSize - 1] == MoleculeCisTrans::TRANS) rotateAngle[0] = - rotateAngle[ind - 1];
            else if (cisTransConf[molSize - 1] == MoleculeCisTrans::CIS) rotateAngle[0] = rotateAngle[ind - 1];
            else if (last_rotate_angle == 1) {
               rotateAngle[0] = 1;
               last_rotate_angle = -1;
               index--;
            } else {
               rotateAngle[0] = -1;
               last_rotate_angle = 1;
            }
         }
         /*   printf("There are %d angled vertexes.", ind);
         for (int i = 0; i < ind; i++) printf("%d ", vertexNumber[i]);
         printf("\n");
         for (int i = 0; i < ind; i++) printf("%d ", edgeLenght[i]);
         printf("\n");
         for (int i = 0; i < ind; i++) printf("%d ", rotateAngle[i]);
         printf("\n");*/

         double x[max_size];
         double y[max_size];
         for (int i = 0; i < ind; i++) {
            x[i] = x_result[vertexNumber[i]] + y_result[vertexNumber[i]] * 0.5;
            y[i] = sqrt(3.0)/2 * y_result[vertexNumber[i]];
         }
         //    for (int i = 0; i < ind; i++) printf("%10.10f %10.10f\n", x[i], y[i]);
         //    printf("\n");

         smoothing(ind, molSize, rotateAngle, edgeLenght, vertexNumber, x, y, profi);

         double newBadness = 0;
         newBadness = badness(ind, molSize, rotateAngle, edgeLenght, vertexNumber, x, y);
         //printf("%10.10f\n", newBadness);

         if (newBadness < bestBadness) {
            bestBadness = newBadness;
            printf("New best badness: %5.5f\n", bestBadness);
            bestIndex = displayIndex;

            for (int i = 0; i < ind; i++) {
               int nexti = (i + 1) % ind;

               for (int j = vertexNumber[i], t = 0; j != vertexNumber[nexti]; j = (j + 1) % molSize, t++) {
                  // printf("%d ", vert[j]);
                  Vec3f &pos = mol.getAtomXyz(vert[j]);
                  pos.x = ((edgeLenght[i] - t) * x[i] + t * x[nexti])/edgeLenght[i];
                  pos.y = ((edgeLenght[i] - t) * y[i] + t * y[nexti])/edgeLenght[i];
                  pos.z = 0;
                  //printf("%5.5f %5.5f\n", pos.x, pos.y);
               }
            }
         }
   }

   printf("%5.5f\n", bestBadness);
   //    printf("\n");

   //    for (int i = 0; i < ind; i++) printf("%10.10f %10.10f\n", x[i], y[i]);
   //    printf("\n");

   printf("\n");
   // Saved changed into molfile
   //    indigoSaveMolfileToFile(m, "builded_molecule2.mol");
   // Render changed
   //indigoSetOption("render-output-format", "png");
   //indigoSetOption("render-background-color", "255, 255, 255");
   //    indigoRenderToFile(m, "builded_molecule.png"); 

   return bestBadness;
}
