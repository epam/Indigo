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

#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "graph/biconnected_decomposer.h"
#include "molecule/molecule.h"
#include "molecule/molfile_loader.h"

int edge_cmp(const int& e1, const int& e2, const void* context)
{
    Molecule& mol = *(Molecule*)context;

    const Edge& edge1 = mol.getEdge(e1);
    const Edge& edge2 = mol.getEdge(e2);

    Vec3f v1 = mol.getAtomPos(edge1.beg);
    Vec3f v2 = mol.getAtomPos(edge1.end);

    v1.z = 0.f;
    v2.z = 0.f;

    float len1 = Vec3f::dist(v1, v2);

    v1 = mol.getAtomPos(edge2.beg);
    v2 = mol.getAtomPos(edge2.end);

    v1.z = 0.f;
    v2.z = 0.f;

    float len2 = Vec3f::dist(v1, v2);

    if (len2 > len1)
        return 1;
    else if (len2 < len1)
        return -1;
    return 0;
}

bool edge_intersection(const Molecule& mol, int edge1_idx, int edge2_idx, Vec2f& p)
{
    const Edge& edge1 = mol.getEdge(edge1_idx);
    const Edge& edge2 = mol.getEdge(edge2_idx);

    if (edge1.beg == edge2.beg || edge1.beg == edge2.end || edge1.end == edge2.beg || edge1.end == edge2.end)
        return false;

    Vec2f v1_1(mol.getAtomPos(edge1.beg).x, mol.getAtomPos(edge1.beg).y);
    Vec2f v1_2(mol.getAtomPos(edge1.end).x, mol.getAtomPos(edge1.end).y);
    Vec2f v2_1(mol.getAtomPos(edge2.beg).x, mol.getAtomPos(edge2.beg).y);
    Vec2f v2_2(mol.getAtomPos(edge2.end).x, mol.getAtomPos(edge2.end).y);

    return Vec2f::intersection(v1_1, v1_2, v2_1, v2_2, p);
}

void convertMolfile(char* path, char* filename, FileOutput& cpp_file)
{
    FileScanner molfile("%s\\%s", path, filename);
    MolfileLoader mf_loader(molfile);
    Molecule mol;
    QS_DEF(Array<int>, edges);

    printf("%s\n", filename);

    mf_loader.loadMolecule(mol, true);

    BiconnectedDecomposer bd(mol);

    if (bd.decompose() != 1)
    {
        printf("Error: %s is not biconnected\n", filename);
        return;
    }

    int i, j;

    edges.clear_reserve(mol.edgeCount());

    for (i = mol.edgeBegin(); i < mol.edgeEnd(); i = mol.edgeNext(i))
        edges.push(i);

    edges.qsort(edge_cmp, &mol);

    const Edge& edge = mol.getEdge(edges[edges.size() / 2]);

    Vec3f v1 = mol.getAtomPos(edge.beg);
    Vec3f v2 = mol.getAtomPos(edge.end);

    v1.z = 0.f;
    v2.z = 0.f;

    float scale = Vec3f::dist(v1, v2);

    if (scale < 0.0001f)
    {
        printf("Error: %s has zero bond\n", filename);
        return;
    }

    scale = 1.f / scale;

    int first_idx = mol.vertexBegin();
    Vec3f pos = mol.getAtomPos(first_idx);

    for (i = mol.vertexNext(first_idx); i < mol.vertexEnd(); i = mol.vertexNext(i))
    {
        if (mol.getAtomPos(i).y < pos.y)
        {
            pos = mol.getAtomPos(i);
            first_idx = i;
        }
    }

    for (i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
    {
        mol.getAtom2(i).pos.sub(pos);
        mol.getAtom2(i).pos.scale(scale);
    }

    char buf[1024];

    sprintf_s(buf, "BEGIN_PATTERN(\"%s\")", filename);
    cpp_file.writeStringCR(buf);

    for (i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
    {
        sprintf_s(buf, "   ADD_ATOM(%d, %ff, %ff)", i, mol.getAtomPos(i).x, mol.getAtomPos(i).y);
        cpp_file.writeStringCR(buf);
    }

    for (i = mol.edgeBegin(); i < mol.edgeEnd(); i = mol.edgeNext(i))
    {
        const Edge& edge = mol.getEdge(i);
        int type = mol.getBond(i).type;
        int qtype = mol.getQueryBond(i).type;

        sprintf_s(buf, "   ADD_BOND(%d, %d, %d)", edge.beg, edge.end, qtype != 0 ? qtype : type);
        cpp_file.writeStringCR(buf);
    }

    Vec2f v, inter;
    Vec2f pos_i;
    int idx = mol.vertexCount();

    i = first_idx;

    float max_angle, cur_angle;
    float i_angle = 0;
    int next_nei = 0;
    int point_idx = 0;

    pos_i.set(mol.getAtomPos(i).x, mol.getAtomPos(i).y);

    while (true)
    {
        const Vertex& vert = mol.getVertex(i);

        if (i != first_idx)
        {
            v.set(pos_i.x, pos_i.y);
            pos_i.set(mol.getAtomPos(i).x, mol.getAtomPos(i).y);
            v.sub(pos_i);

            i_angle = v.tiltAngle2();
        }
        else if (point_idx > 0)
            break;

        sprintf_s(buf, "   OUTLINE_POINT(%d, %ff, %ff)", point_idx++, pos_i.x, pos_i.y);
        cpp_file.writeStringCR(buf);

        max_angle = 0.f;

        for (j = vert.neiBegin(); j < vert.neiEnd(); j = vert.neiNext(j))
        {
            const Vec3f& pos_nei = mol.getAtomPos(vert.neiVertex(j));

            v.set(pos_nei.x - pos_i.x, pos_nei.y - pos_i.y);

            cur_angle = v.tiltAngle2() - i_angle;

            if (cur_angle < 0.f)
                cur_angle += 2 * M_PI;

            if (max_angle < cur_angle)
            {
                max_angle = cur_angle;
                next_nei = j;
            }
        }

        i = vert.neiVertex(next_nei);

        float dist, min_dist = 0.f;
        int int_edge;
        Vec2f cur_v1 = pos_i;
        Vec2f cur_v2(mol.getAtomPos(i).x, mol.getAtomPos(i).y);

        while (min_dist < 10000.f)
        {
            min_dist = 10001.f;

            for (j = mol.edgeBegin(); j < mol.edgeEnd(); j = mol.edgeNext(j))
            {
                const Edge& edge = mol.getEdge(j);
                Vec2f cur_v3(mol.getAtomPos(edge.beg).x, mol.getAtomPos(edge.beg).y);
                Vec2f cur_v4(mol.getAtomPos(edge.end).x, mol.getAtomPos(edge.end).y);

                if (Vec2f::intersection(cur_v1, cur_v2, cur_v3, cur_v4, v))
                    if ((dist = Vec2f::dist(cur_v1, v)) < min_dist)
                    {
                        inter = v;
                        min_dist = dist;
                        int_edge = j;
                    }
            }

            if (min_dist < 10000.f)
            {
                sprintf_s(buf, "   OUTLINE_POINT(%d, %ff, %ff)", point_idx++, v.x, v.y);
                cpp_file.writeStringCR(buf);

                const Edge& edge = mol.getEdge(int_edge);
                Vec2f cur_v3(mol.getAtomPos(edge.beg).x, mol.getAtomPos(edge.beg).y);
                Vec2f cur_v4(mol.getAtomPos(edge.end).x, mol.getAtomPos(edge.end).y);

                Vec2f cur_v1v;
                Vec2f cur_v3v;
                Vec2f cur_v4v;

                cur_v1v.diff(cur_v1, inter);
                cur_v3v.diff(cur_v3, inter);
                cur_v4v.diff(cur_v4, inter);

                float angle1 = cur_v1v.tiltAngle2();
                float angle3 = cur_v3v.tiltAngle2() - angle1;
                float angle4 = cur_v4v.tiltAngle2() - angle1;

                if (angle3 < 0)
                    angle3 += 2 * M_PI;
                if (angle4 < 0)
                    angle4 += 2 * M_PI;

                cur_v1 = inter;

                if (angle3 > angle4)
                {
                    cur_v2 = cur_v3;
                    i = edge.beg;
                }
                else
                {
                    cur_v2 = cur_v4;
                    i = edge.end;
                }
            }
        }
    }

    cpp_file.writeStringCR("END_PATTERN()");
}

#include <windows.h>

void main(int argc, char* argv[])
{
    WIN32_FIND_DATAA ffd;
    HANDLE h_find;
    char file_pattern[MAX_PATH];

    if (argc != 3)
    {
        printf("patmake.exe <mol-root> <cpp-output>");
        return;
    }

    if (argv[1][strlen(argv[1]) - 1] == '\\')
        argv[1][strlen(argv[1]) - 1] = 0;

    sprintf_s(file_pattern, "%s\\*.mol", argv[1]);

    h_find = FindFirstFileA(file_pattern, &ffd);

    if (h_find == INVALID_HANDLE_VALUE)
    {
        printf("FindFirstFile failed (%d)\n", GetLastError());
        return;
    }
    else
    {
        try
        {
            FileOutput cpp_file(true, argv[2]);
            SYSTEMTIME st;

            GetSystemTime(&st);

            char buf[200];

            sprintf_s(buf, " * Added %02d/%02d/%02d %02d:%02d:%02d", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond);

            cpp_file.writeStringCR("/*");
            cpp_file.writeStringCR(buf);
            cpp_file.writeStringCR(" */");

            do
            {
                try
                {
                    convertMolfile(argv[1], ffd.cFileName, cpp_file);
                }
                catch (Exception& e)
                {
                    printf("Error: %s\n", e.message());
                }
            } while (FindNextFileA(h_find, &ffd) != 0);

            printf("Done.\n");
        }
        catch (Exception& e)
        {
            printf("Error: %s\n", e.message());
        }

        FindClose(h_find);
    }
}
