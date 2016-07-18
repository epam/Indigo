/****************************************************************************
* Copyright (C) 2009-2015 EPAM Systems
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

#include "layout\cleaner2d.h"
#include "graph\biconnected_decomposer.h"
#include <algorithm> 

using namespace indigo;

//IMPL_ERROR(Cleaner2d, "cleaner2d");

Cleaner2d::Cleaner2d(Molecule& mol) : _mol(mol) {
    vertex_count = _mol.vertexCount();
    printf("%d\n", vertex_count);
    BiconnectedDecomposer bi_decomposer(_mol);
    component_count = bi_decomposer.decompose();
    printf("%d\n", component_count);
    if (component_count == 1) {
        is_biconnected = true;
        return;
    }

    base_point.clear();
    base_point_comp.clear();

    in.clear();
    for (int i = 0; i < component_count; i++) {
        in.push();
        in.top().clear_resize(vertex_count);
    }
    printf("A\n");
    Filter filter;
    for (int i = 0; i < component_count; i++) {
        printf("%d\n", i);
        bi_decomposer.getComponent(i, filter);
        for (int j = 0; j < vertex_count; j++) in[i][j] = filter.valid(j);
    }
    printf("B\n");
    def.clear();
    for (int i = 0; i < component_count; i++) {
        def.push();
        def.top().clear();
    }

    QS_DEF(Array<bool>, has_component);
    QS_DEF(Array<int>, component_list);
    QS_DEF(Array<bool>, has_vertex);
    QS_DEF(Array<bool>, block_vertex);
    has_component.clear_resize(component_count);
    has_component.zerofill();
    component_list.clear();
    has_vertex.clear_resize(vertex_count);
    has_vertex.zerofill();
    block_vertex.clear_resize(vertex_count);
    block_vertex.zerofill();
    

    for (int i = 0; i < vertex_count; i++) {
        coef.push();
        coef.top().clear();
    }

    /*printf("%d components\n", component_count);
    for (int i = 0; i < component_count; i++) {
        printf("%d: ", i);
        for (int j = 0; j < vertex_count; j++) if (in[i][j]) printf("%d, ", j);
        printf("\n");
    }*/

    QS_DEF(Array<int>, local_component_list);
    //printf("C\n");

    /*printf("Components for every vertex:\n");
    for (int i = 0; i < vertex_count; i++) {
        printf("%d: ", i);
        bi_decomposer.getVertexComponents(i, local_component_list);
        for (int j = 0; j < local_component_list.size(); j++) printf("%d, ", local_component_list[j]);
        printf("\n");
    }*/

    int index = 0;
    for (int c = 0; c < component_count; c++) if (!has_component[c]) {

        int ver = -1;
        for (int i = 0; i < vertex_count; i++) if (in[c][i]) if (bi_decomposer.isArticulationPoint(i)) {
            ver = i;
            break;
        }
        base_point.push(ver);
        add_coef(ver, ver, ONE);

        base_point_comp.push(c);

        has_vertex[ver] = true;
        block_vertex[ver] = true;

        has_component[c] = true;
        component_list.push(c);

        for (; index < component_list.size(); index++) {
            // 1. Search for new vertex
            int comp = component_list[index];
            ver = -1;
            for (int j = 0; j < vertex_count; j++) if (in[comp][j] && !block_vertex[j] && bi_decomposer.isArticulationPoint(j)) {
                ver = j;
                break;
            }
            if (ver == -1) {
                for (int j = 0; j < vertex_count; j++) if (in[comp][j] && !block_vertex[j]) {
                    ver = j;
                    break;
                }
            }
            base_point.push(ver);
            add_coef(ver, ver, ONE);
            base_point_comp.push(comp);
            has_vertex[ver] = true;

            // 2. Add yet another defining point if it is need

            for (int j = 0; j < base_point.size(); j++) if (in[comp][base_point[j]]) def[comp].push(base_point[j]);
            if (def[comp].size() < 2) {
                int newver = -1;
                for (int j = 0; j < vertex_count; j++) if (block_vertex[j] && in[comp][j]) {
                    newver = j;
                    break;
                }

                def[comp].push(newver);
            }

            // 3. Calculation coefficients

            for (int j = 0; j < vertex_count; j++) if (in[comp][j]) calc_coef(j, def[comp][0], def[comp][1]);

            // 4. Add new components to list

            for (int v = 0; v < vertex_count; v++) if (in[comp][v] && bi_decomposer.isArticulationPoint(v)) {
                for (int j = 0; j < component_count; j++) if (in[j][v] && !has_component[j]) {
                    component_list.push(j);
                    has_component[j] = true;
                }
            }
            for (int v = 0; v < vertex_count; v++) if (in[comp][v]) block_vertex[v] = true;
        }

    }
    
    printf("%d components\n", component_count);
    for (int i = 0; i < component_count; i++) {
        printf("%d: ", i);
        for (int j = 0; j < vertex_count; j++) if (in[i][j]) printf("%d, ", j);
        printf("|| ");
        printf("%d: ", def[i].size());
        for (int j = 0; j < def[i].size(); j++) printf("%d, ", def[i][j]);
        printf("\n");
    }

}

void Cleaner2d::add_coef(int ver, int index, Vec2f value) {
    while (coef[ver].size() <= index) coef[ver].push(ZERO);
    coef[ver][index] += value;
}

void Cleaner2d::calc_coef(int to, int from0, int from1) {
    Vec2f A0 = plane(_mol.getAtomXyz(from0));
    Vec2f A1 = plane(_mol.getAtomXyz(from1));
    Vec2f A2 = plane(_mol.getAtomXyz(to));

    Vec2f vec = A1 - A0;
    float dist2 = vec.lengthSqr();

    float cross = Vec2f::cross(A1 - A0, A2 - A0); // c1
    float dot = Vec2f::dot(A1 - A0, A2 - A0); // c2

    Vec2f _coef = Vec2f(dot, cross) / dist2;

    int len = std::max(coef[from0].size(), coef[from1].size());
    add_coef(from0, len - 1, ZERO);
    add_coef(from1, len - 1, ZERO);

    for (int i = 0; i < len; i++) {
        add_coef(to, i, mult(_coef, coef[from1][i]));
        add_coef(to, i, mult(_coef, coef[from0][i]) * -1);
    }
}

void Cleaner2d::updatePosition(int i) {
    pos[i] = ZERO;
    for (int j = 0; j < coef[i].size(); j++) pos[i] += mult(coef[i][j], pos[base_point[j]]);
}

void Cleaner2d::updatePositions() {
    for (int i = 0; i < vertex_count; i++) updatePosition(i);
}

void Cleaner2d::clean() {
    if (is_biconnected) return; // nothing to do for biconnected graph
}