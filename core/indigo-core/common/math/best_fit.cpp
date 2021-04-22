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

#include "base_cpp/array.h"
#include "base_cpp/tlscont.h"
#include "math/algebra.h"

using namespace indigo;

bool Transform3f::bestFit(int npoints, const Vec3f points[], const Vec3f goals[], float* sqsum_out)
{
    QS_DEF(Array<double>, X); // set of points
    QS_DEF(Array<double>, Y); // set of goals
    Matr3x3d R, RT, RTR, evectors_matrix;
    //
    Matr3x3d rotation;
    double scale;
    Vec3f translation;
    //

    bool res = 1;
    Vec3f vec, tmp;
    double cpoints[3] = {0.0}, cgoals[3] = {0.0}; // centroid of points, of goals
    int i, j, k;

    for (i = 0; i < npoints; i++)
    {
        cpoints[0] += points[i].x;
        cpoints[1] += points[i].y;
        cpoints[2] += points[i].z;
        cgoals[0] += goals[i].x;
        cgoals[1] += goals[i].y;
        cgoals[2] += goals[i].z;
    }
    for (i = 0; i < 3; i++)
    {
        cpoints[i] /= npoints;
        cgoals[i] /= npoints;
    }
    X.resize(npoints * 3);
    Y.resize(npoints * 3);

    // move each set to origin
    for (i = 0; i < npoints; i++)
    {
        X[i * 3 + 0] = points[i].x - cpoints[0];
        X[i * 3 + 1] = points[i].y - cpoints[1];
        X[i * 3 + 2] = points[i].z - cpoints[2];
        Y[i * 3 + 0] = goals[i].x - cgoals[0];
        Y[i * 3 + 1] = goals[i].y - cgoals[1];
        Y[i * 3 + 2] = goals[i].z - cgoals[2];
    }

    if (npoints > 1)
    {
        /* compute R */
        for (i = 0; i < 3; i++)
        {
            for (j = 0; j < 3; j++)
            {
                R.elements[i * 3 + j] = 0.0;
                for (k = 0; k < npoints; k++)
                {
                    R.elements[i * 3 + j] += Y[k * 3 + i] * X[k * 3 + j];
                }
            }
        }

        // Compute R^T * R

        R.getTransposed(RT);
        RT.matrixMatrixMultiply(R, RTR);

        RTR.eigenSystem(evectors_matrix);

        if (RTR.elements[0] > 2 * EPSILON)
        {
            //         float norm_b0,norm_b1,norm_b2;
            Vec3f a0, a1, a2;
            Vec3f b0, b1, b2;

            a0.set((float)evectors_matrix.elements[0], (float)evectors_matrix.elements[3], (float)evectors_matrix.elements[6]);
            a1.set((float)evectors_matrix.elements[1], (float)evectors_matrix.elements[4], (float)evectors_matrix.elements[7]);
            a2.cross(a0, a1);

            R.matrixVectorMultiply(a0, b0);
            R.matrixVectorMultiply(a1, b1);
            //         norm_b0 = b0.length();
            //         norm_b1 = b1.length();
            Line3f l1, l2;
            float sqs1, sqs2;
            l1.bestFit(npoints, points, &sqs1);
            l2.bestFit(npoints, goals, &sqs2);
            if (sqs1 < 2 * EPSILON && sqs2 < 2 * EPSILON)
            {
                Transform3f temp;
                temp.rotationVecVec(l1.dir, l2.dir);
                for (i = 0; i < 3; i++)
                    for (j = 0; j < 3; j++)
                        rotation.elements[i * 3 + j] = temp.elements[j * 4 + i];
            }
            else
            {
                b0.normalize();
                b1.normalize();
                b2.cross(b0, b1);
                //            norm_b2 = b2.length();

                evectors_matrix.elements[2] = a2.x;
                evectors_matrix.elements[5] = a2.y;
                evectors_matrix.elements[8] = a2.z;
                evectors_matrix.transpose();

                RTR.elements[0] = b0.x;
                RTR.elements[1] = b1.x;
                RTR.elements[2] = b2.x;
                RTR.elements[3] = b0.y;
                RTR.elements[4] = b1.y;
                RTR.elements[5] = b2.y;
                RTR.elements[6] = b0.z;
                RTR.elements[7] = b1.z;
                RTR.elements[8] = b2.z;
                RTR.matrixMatrixMultiply(evectors_matrix, rotation);
            }
        }
        else
        {
            res = 0;
        }
    }
    else
    {
        res = 0;
    }
    if (!res)
    {
        rotation.identity();
    }

    // Calc scale
    scale = 1.0;
    if (res && npoints > 1)
    {
        float l1 = 0.0;
        float l2 = 0.0;
        Vec3f vx, vy;
        for (i = 0; i < npoints; i++)
        {
            Vec3f vx((float)X[i * 3 + 0], (float)X[i * 3 + 1], (float)X[i * 3 + 2]);
            Vec3f vy((float)Y[i * 3 + 0], (float)Y[i * 3 + 1], (float)Y[i * 3 + 2]);
            rotation.matrixVectorMultiply(vx, vec);
            l1 += Vec3f::dot(vy, vec);
            l2 += Vec3f::dot(vec, vec);
        }
        scale = l1 / l2;
    }

    X.clear();
    Y.clear();

    // Calc translation
    translation.set((float)cgoals[0], (float)cgoals[1], (float)cgoals[2]);
    tmp = Vec3f((float)cpoints[0], (float)cpoints[1], (float)cpoints[2]);
    rotation.matrixVectorMultiply(tmp, vec);
    vec.scale((float)scale);
    translation.sub(vec);

    identity();
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            elements[i * 4 + j] = (float)rotation.elements[j * 3 + i];
        }
    }
    elements[15] = 1.0f;
    translate(translation);
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            elements[i * 4 + j] *= (float)scale;
        }
    }

    // Deviation
    if (sqsum_out)
    {
        *sqsum_out = 0;
        float d = .0f;
        for (i = 0; i < npoints; i++)
        {
            vec.pointTransformation(points[i], *this);
            d = Vec3f::dist(vec, goals[i]);
            *sqsum_out += d * d;
        }
    }
    return true;
}

bool Plane3f::bestFit(int npoints, const Vec3f points[], float* sqsum_out)
{
    QS_DEF(Array<double>, m);
    m.clear_resize(npoints * 3);
    int i, j, k;
    Matr3x3d A, evec;
    Vec3f c;

    for (i = 0; i < npoints; i++)
    {
        c.add(points[i]);
    }
    c.scale(1.0f / npoints);

    for (i = 0; i < npoints; i++)
    {
        m[3 * i + 0] = points[i].x - c.x;
        m[3 * i + 1] = points[i].y - c.y;
        m[3 * i + 2] = points[i].z - c.z;
    }

    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            A.elements[i * 3 + j] = 0;
            for (k = 0; k < npoints; k++)
            {
                A.elements[i * 3 + j] += m[k * 3 + i] * m[k * 3 + j];
            }
        }
    }

    A.eigenSystem(evec);
    _norm.x = (float)evec.elements[2];
    _norm.y = (float)evec.elements[5];
    _norm.z = (float)evec.elements[8];

    _d = -Vec3f::dot(_norm, c);

    if (sqsum_out != 0)
    {
        *sqsum_out = 0;
        for (i = 0; i < npoints; i++)
        {
            float d = distFromPoint(points[i]);
            *sqsum_out += d * d;
        }
    }
    return true;
}

bool Line3f::bestFit(int npoints, const Vec3f points[], float* sqsum_out)
{
    QS_DEF(Array<double>, m);
    Matr3x3d A;
    Matr3x3d evec;
    int i, j, k;
    m.clear_resize(npoints * 3);
    org = Vec3f(0, 0, 0);

    for (i = 0; i < npoints; i++)
    {
        org.add(points[i]);
    }
    org.scale(1.0f / npoints);

    for (i = 0; i < npoints; i++)
    {
        m[3 * i + 0] = points[i].x - org.x;
        m[3 * i + 1] = points[i].y - org.y;
        m[3 * i + 2] = points[i].z - org.z;
    }

    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            A.elements[i * 3 + j] = 0;
            for (k = 0; k < npoints; k++)
            {
                A.elements[i * 3 + j] += m[k * 3 + i] * m[k * 3 + j];
            }
        }
    }
    A.eigenSystem(evec);
    dir.x = (float)evec.elements[0];
    dir.y = (float)evec.elements[3];
    dir.z = (float)evec.elements[6];
    dir.normalize();

    if (sqsum_out != 0)
    {
        *sqsum_out = 0;
        for (int i = 0; i < npoints; i++)
        {
            float d = distFromPoint(points[i]);
            *sqsum_out += d * d;
        }
    }

    return true;
}
