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

#include <string.h>

#include "math/algebra.h"

using namespace indigo;

IMPL_ERROR(Matr3x3d, "Matr3x3d");

Matr3x3d::Matr3x3d()
{
    memset(elements, 0, 9 * sizeof(double));
}

void Matr3x3d::copy(const Matr3x3d& matr)
{
    memcpy(elements, matr.elements, 9 * sizeof(double));
}

void Matr3x3d::identity()
{
    memset(elements, 0, 9 * sizeof(double));
    elements[0] = elements[4] = elements[8] = 1.0;
}

void Matr3x3d::_givensRotation(double x0, double x1, double& c, double& s)
{
    if (fabs(x1) < 3e-16)
    {
        c = 1;
        s = 0;
    }
    else if (fabs(x1) > fabs(x0))
    {
        double t = -x0 / x1;
        s = 1.0 / sqrt(1 + t * t);
        c = s * t;
    }
    else
    {
        double t = -x1 / x0;
        c = 1.0 / sqrt(1 + t * t);
        s = c * t;
    }
}

void Matr3x3d::_qrStep(int n, double gc[], double gs[])
{
    double x, z;
    double mu;
    double c, s;

    Matr3x3d rot, tmp;
    int k;

    double d1 = elements[(n - 1) * 3 + (n - 1)];
    double d2 = elements[n * 3 + n];
    double sd = elements[n * 3 + (n - 1)];
    double dt = (d1 - d2) / 2.0;

    if (dt > 0)
    {
        mu = d2 - sd * (sd / (dt + sqrt(dt * dt + sd * sd)));
    }
    else if (fabs(dt) < 3e-15)
    {
        mu = d2 - fabs(sd);
    }
    else
    {
        mu = d2 + sd * (sd / ((-dt) + sqrt(dt * dt + sd * sd)));
    }
    x = elements[0] - mu;
    z = elements[3];

    if (n == 1)
    {
        _givensRotation(x, z, c, s);
        gc[0] = c;
        gs[0] = s;
        double e0 = elements[0], e3 = elements[3], e4 = elements[4];
        elements[0] = c * (c * e0 - s * e3) + s * (s * e4 - c * e3);
        elements[3] = c * (s * e0 + c * e3) - s * (s * e3 + c * e4);
        elements[4] = s * (s * e0 + c * e3) + c * (s * e3 + c * e4);
        return;
    }

    k = 0;
    if (fabs(elements[3]) < 3e-15 * (fabs(elements[0]) + fabs(elements[4])))
    {
        k = 1;
        gc[0] = 1.0;
        gs[0] = 0.0;
        x = elements[4] - mu;
        z = elements[7];
    }

    for (; k < n; k++)
    {
        _givensRotation(x, z, c, s);
        gc[k] = c;
        gs[k] = s;

        rot.identity();
        rot.elements[k * 3 + k] = c;
        rot.elements[(k + 1) * 3 + (k + 1)] = c;
        rot.elements[(k + 1) * 3 + k] = -s;
        rot.elements[k * 3 + (k + 1)] = s;

        rot.transpose();
        rot.matrixMatrixMultiply(*this, tmp);
        rot.transpose();
        tmp.matrixMatrixMultiply(rot, *this);

        x = elements[(k + 1) * 3 + k];
        z = elements[(k + 2) * 3 + k];
    }
}

void Matr3x3d::getTransposed(Matr3x3d& matr_out) const
{
    matr_out.copy(*this);
    matr_out.transpose();
}

void Matr3x3d::matrixVectorMultiply(const Vec3f& a, Vec3f& b) const
{
    b.x = (float)(elements[0] * a.x + elements[1] * a.y + elements[2] * a.z);
    b.y = (float)(elements[3] * a.x + elements[4] * a.y + elements[5] * a.z);
    b.z = (float)(elements[6] * a.x + elements[7] * a.y + elements[8] * a.z);
}

void Matr3x3d::transpose()
{

    std::swap(elements[3], elements[1]);
    std::swap(elements[6], elements[2]);
    std::swap(elements[7], elements[5]);
}

void Matr3x3d::matrixMatrixMultiply(const Matr3x3d& m, Matr3x3d& matrix_out) const
{
    int i, j, k;
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            matrix_out.elements[i * 3 + j] = 0;
            for (k = 0; k < 3; k++)
            {
                matrix_out.elements[i * 3 + j] += elements[i * 3 + k] * m.elements[k * 3 + j];
            }
        }
    }
}

void Matr3x3d::eigenSystem(Matr3x3d& evec_out)
{
    Matr3x3d Q;
    int i, j, k, s;

    // Householder reduction to tridiagonal matrix
    double x0, x1, nrm;
    double alpha, beta, tau;
    x0 = elements[3];
    x1 = elements[6];
    nrm = fabs(x1);
    if (nrm < 3e-16)
    {
        tau = 0;
    }
    else
    {
        alpha = x0;
        beta = -(alpha >= 0.0 ? +1.0 : -1.0) * sqrt(alpha * alpha + nrm * nrm);
        tau = (beta - alpha) / beta;

        x1 = x1 / (alpha - beta);
        x0 = beta;
    }
    evec_out.identity();
    evec_out.elements[4] -= tau;
    evec_out.elements[5] -= tau * x1;
    evec_out.elements[7] -= tau * x1;
    evec_out.elements[8] -= tau * x1 * x1;
    evec_out.matrixMatrixMultiply(*this, Q);
    Q.matrixMatrixMultiply(evec_out, *this);
    //////////////////////////////////////////////////////////////////////////

    // QR Iterations
    i = 2;
    s = 0;
    while (i > 0 && s++ < 100)
    {
        if (fabs(elements[i * 3 + i - 1]) < 3e-15 * (fabs(elements[(i - 1) * 3 + (i - 1)]) + fabs(elements[i * 3 + i])))
        {
            i--;
            s = 0;
            continue;
        }

        double gc[2];
        double gs[2];

        _qrStep(i, gc, gs);

        // Apply Givens rotation to transformation matrix
        for (j = 0; j < i; j++)
        {
            double c = gc[j], s = gs[j];
            for (k = 0; k < 3; k++)
            {
                x0 = evec_out.elements[k * 3 + j];
                x1 = evec_out.elements[k * 3 + j + 1];
                evec_out.elements[k * 3 + j] = x0 * c - x1 * s;
                evec_out.elements[k * 3 + j + 1] = x0 * s + x1 * c;
            }
        }
    }
    //////////////////////////////////////////////////////////////////////////

    // Sort eigenvalues in descending order
    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < 2 - i; j++)
        {
            if (elements[j * 3 + j] < elements[(j + 1) * 3 + (j + 1)])
            {
                std::swap(elements[j * 3 + j], elements[(j + 1) * 3 + (j + 1)]);
                std::swap(evec_out.elements[0 * 3 + j], evec_out.elements[0 * 3 + (j + 1)]);
                std::swap(evec_out.elements[1 * 3 + j], evec_out.elements[1 * 3 + (j + 1)]);
                std::swap(evec_out.elements[2 * 3 + j], evec_out.elements[2 * 3 + (j + 1)]);
            }
        }
    }
    //////////////////////////////////////////////////////////////////////////
}
