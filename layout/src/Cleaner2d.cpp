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

#include "layout/cleaner2d.h"
#include "graph/biconnected_decomposer.h"
#include "molecule/molecule.h"
#include <algorithm> 
#include <vector>

using namespace indigo;

//IMPL_ERROR(Cleaner2d, "cleaner2d");

Cleaner2d::Cleaner2d(Molecule& mol) : _mol(mol) {
    vertex_count = _mol.vertexCount();
//    printf("%d\n", vertex_count);
    BiconnectedDecomposer bi_decomposer(_mol);
    component_count = bi_decomposer.decompose();
//    printf("%d\n", component_count);
    if (component_count == 1) {
        is_biconnected = true;
        return;
    }
    else is_biconnected = false;
    pos.clear_resize(vertex_count);

    base_point.clear();
    base_point_comp.clear();

    is_art_point.clear_resize(vertex_count);
    for (int i = 0; i < vertex_count; i++) is_art_point[i] = bi_decomposer.isArticulationPoint(i);

    adj_matrix.clear();
    for (int i = 0; i < vertex_count; i++) {
        adj_matrix.push();
        adj_matrix.top().clear_resize(vertex_count);
        adj_matrix.top().zerofill();
    }
    for (int e = _mol.edgeBegin(); e != _mol.edgeEnd(); e = _mol.edgeNext(e)) {
        Edge Ed = _mol.getEdge(e);
        adj_matrix[Ed.beg][Ed.end] = adj_matrix[Ed.end][Ed.beg] = true;
    }

    std::vector<float> lens;
    for (int e = _mol.edgeBegin(); e != _mol.edgeEnd(); e = _mol.edgeNext(e))
        lens.push_back(Vec2f::dist(plane(_mol.getAtomXyz(_mol.getEdge(e).beg)), plane(_mol.getAtomXyz(_mol.getEdge(e).end))));

    sort(lens.begin(), lens.end());

    target_len = lens.size() % 2 == 1 ? lens[lens.size() / 2] : (lens[lens.size() / 2] + lens[lens.size() / 2 - 1]) / 2;

    in.clear();
    for (int i = 0; i < component_count; i++) {
        in.push();
        in.top().clear_resize(vertex_count);
    }
    Filter filter;
    for (int i = 0; i < component_count; i++) {
        bi_decomposer.getComponent(i, filter);
        for (int j = 0; j < vertex_count; j++) in[i][j] = filter.valid(j);
    }

    common_comp.clear();
    for (int i = 0; i < vertex_count; i++) {
        common_comp.push();
        common_comp.top().clear_resize(vertex_count);
        common_comp.top().fffill();
        for (int j = 0; j < vertex_count; j++)
            for (int c = 0; c < component_count; c++) if (in[c][i] && in[c][j]) common_comp[i][j] = c;
    }

    edge_comp.clear_resize(_mol.edgeEnd());

    for (int e = _mol.edgeBegin(); e != _mol.edgeEnd(); e = _mol.edgeNext(e)) {
        int v = _mol.getEdge(e).beg, u = _mol.getEdge(e).end;
        for (int i = 0; i < component_count; i++) if (in[i][u] && in[i][v]) edge_comp[e] = i;
    }

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

    QS_DEF(Array<int>, local_component_list);

    int index = 0;
    for (int c = 0; c < component_count; c++) if (!has_component[c]) {

        int ver = -1;
        for (int i = 0; i < vertex_count; i++) if (in[c][i]) if (bi_decomposer.isArticulationPoint(i)) {
            ver = i;
            break;
        }
        base_point.push(ver);
        add_coef(ver, base_point.size() - 1, ONE);

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
            add_coef(ver, base_point.size() - 1, ONE);
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

            for (int j = 0; j < vertex_count; j++) 
                if (in[comp][j] && j != def[comp][0] && j != def[comp][1] && !block_vertex[j]) 
                    calc_coef(j, def[comp][0], def[comp][1]);

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
    
    base_point_index.clear_resize(vertex_count);
    base_point_index.fffill();
    for (int i = 0; i < base_point.size(); i++) base_point_index[base_point[i]] = i;

    gradient.clear_resize(base_point.size());
    pregradient.clear_resize(vertex_count);
    for (int i = 0; i < vertex_count; i++) if (is_base_point(i)) pos[i] = plane(_mol.getAtomXyz(i));

    /*printf("%d components\n", component_count);
    for (int i = 0; i < component_count; i++) {
        printf("%d: ", i);
        for (int j = 0; j < vertex_count; j++) if (in[i][j]) printf("%d, ", j);
        printf("|| ");
        printf("%d: ", def[i].size());
        for (int j = 0; j < def[i].size(); j++) printf("%d, ", def[i][j]);
        printf("\n");
    }

    for (int i = 0; i < vertex_count; i++) {
        printf("%d = ", i);
        for (int j = 0; j < coef[i].size(); j++) if (coef[i][j].lengthSqr() > 0) 
        {
            printf(" + %d * (%.2f, %.2f)", base_point[j], coef[i][j].x, coef[i][j].y);
        }
        printf("\n");
    }*/
}

bool Cleaner2d::is_base_point(int i) { return base_point_index[i] >= 0; }

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
        Vec2f one_minus_coef = ONE - _coef;
        add_coef(to, i, mult(one_minus_coef, coef[from0][i]));
    }
}

void Cleaner2d::updatePosition(int i) {
    pos[i] = ZERO;
    for (int j = 0; j < coef[i].size(); j++) pos[i] += mult(coef[i][j], pos[base_point[j]]);
}

void Cleaner2d::updatePositions() {
    for (int i = 0; i < vertex_count; i++) if (!is_base_point(i)) updatePosition(i);
}

void Cleaner2d::update_gradient() {
    pregradient.zerofill();

    // 1. edges

    float sqrt3 = sqrt(3.);

    if (1)
    for (int e = _mol.edgeBegin(); e != _mol.edgeEnd(); e = _mol.edgeNext(e)) {
        int c = edge_comp[e];

        int v1 = _mol.getEdge(e).beg;
        int v2 = _mol.getEdge(e).end;

        float len = (pos[v1] - pos[v2]).length();

        pregradient[v1] += (pos[v1] - pos[v2]) * 2 * (len - target_len) / len / (target_len * target_len);
        pregradient[v2] += (pos[v2] - pos[v1]) * 2 * (len - target_len) / len / (target_len * target_len);
    }

    // 2. atoms pairs
    if (1) 
    for (int i = 0; i < vertex_count; i++)
        for (int j = 0; j < i; j++) if (common_comp[i][j] == -1) {
            float dist2 = Vec2f::distSqr(pos[i], pos[j]);
            if (dist2 < target_len * target_len) {
                //printf("[%d, %d]\n", i, j);
                float dist = sqrt(dist2);
                pregradient[i] += (pos[i] - pos[j]) * (2 * (dist - target_len) / dist) / (target_len * target_len);
                pregradient[j] += (pos[j] - pos[i]) * (2 * (dist - target_len) / dist) / (target_len * target_len);
            }
        }
    //printf("\n");
                    
    // 3. angles

    if (1)
    for (int i = 0; i < vertex_count; i++) if (is_art_point[i]) {
        const Vertex& vert = _mol.getVertex(i);
        for (int n1 = vert.neiBegin(); n1 != vert.neiEnd(); n1 = vert.neiNext(n1))
            for (int n2 = vert.neiBegin(); n2 < n1; n2 = vert.neiNext(n2)) {
                int v1 = vert.neiVertex(n1);
                int v2 = vert.neiVertex(n2);
                if (common_comp[v1][v2] >= 0) continue;

                Vec2f vec1 = pos[v1] - pos[i];
                Vec2f vec2 = pos[v2] - pos[i];

                float dot = Vec2f::dot(vec1, vec2);
                float cross = Vec2f::cross(vec1, vec2);
                float signcross = cross > 0 ? 1 : cross == 0 ? 0 : -1;

                float l1 = vec1.length();
                float l2 = vec2.length();

                float cos = dot / (l1 * l2);
                float sin = cross / (l1 * l2);

                if (cos < -1) cos = -1;
                if (cos > 1) cos = 1;
                if (sin < -1) sin = -1;
                if (sin > 1) sin = 1;

                if (fabs(sin) > 1e-6) {

                    float x = 1 - cos * cos;
                    float acosd = -1. / sqrt(x);
                    Vec2f alphadv1 = (vec2 * l1 * l2 - vec1 * dot * l2 / l1) * acosd / (l1 * l1 * l2 * l2) * signcross;
                    Vec2f alphadv2 = (vec1 * l1 * l2 - vec2 * dot * l1 / l2) * acosd / (l1 * l1 * l2 * l2) * signcross;
                    Vec2f alphadv = ((vec1 + vec2) * (-l1 * l2) + (vec1 * l2 / l1 + vec2 * l1 / l2) * dot) * acosd / (l1 * l1 * l2 * l2) * signcross;

                    float alpha = acos(cos) * signcross;
                    float target_alpha = (2 * PI / 3) * signcross;

                    pregradient[i] += alphadv * (alpha - target_alpha) * 2;
                    pregradient[v1] += alphadv1 * (alpha - target_alpha) * 2;
                    pregradient[v2] += alphadv2 * (alpha - target_alpha) * 2;
                }
            }
    }

    // 4. final gradient calc

    gradient.zerofill();
    for (int i = 0; i < vertex_count; i++)
        for (int j = 0; j < coef[i].size(); j++) gradient[j] += mult(pregradient[i], coef[i][j]);
}

void Cleaner2d::update_gradient2() {
    float e0 = energy();
    for (int i = 0; i < base_point.size(); i++) {
        pos[base_point[i]].x += dif_eps;
        updatePositions();
        float e1 = energy();
        pos[base_point[i]].x -= dif_eps;
        pos[base_point[i]].y += dif_eps;
        updatePositions();
        float e2 = energy();
        pos[base_point[i]].y -= dif_eps;
        gradient[i].set((e1 - e0) / dif_eps, (e2 - e0) / dif_eps);
    }
}

void Cleaner2d::clean() {
    if (is_biconnected) return; // nothing to do for biconnected graph

    updatePositions();

    //for (int i = 0; i < vertex_count; i++) printf("%d: (%.5f, %.5f)\n", i, pos[i].x, pos[i].y);

    QS_DEF(Array<float>, mult);
    QS_DEF(Array<float>, energies);

    int k = 20;
    mult.clear_resize(k + 1);
    energies.clear_resize(k + 1);
    mult[0] = 0;
    mult[1] = 1.;
    for (int i = 2; i <= k; i++) mult[i] = mult[i - 1] * 0.5;

    float need_len = target_len;
    for (int iter = 0; iter < 100; iter++) {
        update_gradient2();
        /*if (iter == 99) {
            for (int i = 0; i < gradient.size(); i++) printf("%d: (%.5f, %.5f) : (%.5f, %.5f)\n", base_point[i], gradient[i].x, gradient[i].y, dgradient[i].x, dgradient[i].y);
        }*/
        float len = 0;
        for (int i = 0; i < base_point.size(); i++) len += gradient[i].lengthSqr();
        len = sqrt(len);
        float factor = need_len / len;
        for (int i = 0; i < base_point.size(); i++) gradient[i] *= factor;

        for (int i = 0; i <= k; i++) {
            for (int j = 0; j < base_point.size(); j++) pos[base_point[j]] -= gradient[j] * mult[i];
            updatePositions();
            energies[i] = energy();
            for (int j = 0; j < base_point.size(); j++) pos[base_point[j]] += gradient[j] * mult[i];
        }

        int best_i = 0;
        for (int i = 1; i <= k; i++) if (energies[i] < energies[best_i]) best_i = i;
        //printf("%d\n", best_i);

        for (int i = 0; i < base_point.size(); i++) pos[base_point[i]] -= gradient[i] * mult[best_i];
        updatePositions();

        //need_len *= .95;
    }

    for (int i = 0; i < vertex_count; i++) _mol.setAtomXyz(i, Vec3f(pos[i].x, pos[i].y, 0));
}

float Cleaner2d::energy() {
    float result = 0;

    // 1. edges
    if (1)
    for (int e = _mol.edgeBegin(); e != _mol.edgeEnd(); e = _mol.edgeNext(e)) {
        int c = edge_comp[e];
        float e_len = (pos[_mol.getEdge(e).beg] - pos[_mol.getEdge(e).end]).length();

        float diff = (e_len - target_len) / target_len;
        result += diff * diff;
    }

    // 2. atoms pairs
    if (1)
    for (int i = 0; i < vertex_count; i++)
        for (int j = 0; j < i; j++) if (common_comp[i][j] == -1) {
            float dist2 = Vec2f::distSqr(pos[i], pos[j]);
            if (dist2 < target_len * target_len) {
                float dist = sqrt(dist2);
                float diff = (dist - target_len) / target_len;
                result += diff * diff;
            }
        }
    //printf("\n");

    // 3. angles

    for (int i = 0; i < vertex_count; i++) if (is_art_point[i]) {
        const Vertex& vert = _mol.getVertex(i);
        for (int n1 = vert.neiBegin(); n1 != vert.neiEnd(); n1 = vert.neiNext(n1))
            for (int n2 = vert.neiBegin(); n2 < n1; n2 = vert.neiNext(n2)) {
                int v1 = vert.neiVertex(n1);
                int v2 = vert.neiVertex(n2);
                if (common_comp[v1][v2] >= 0) continue;

                Vec2f vec1 = pos[v1] - pos[i];
                Vec2f vec2 = pos[v2] - pos[i];

                float dot = Vec2f::dot(vec1, vec2);
                float cross = Vec2f::cross(vec1, vec2);
                float signcross = cross > 0 ? 1 : cross == 0 ? 0 : -1;

                float l1 = vec1.length();
                float l2 = vec2.length();

                float cos = dot / (l1 * l2);
                float sin = cross / (l1 * l2);

                float alpha;

                if (fabs(cos) < 0.5) {
                    alpha = acos_stable(cos)* signcross;
                }
                else {
                    alpha = asin_stable(sin);
                    if (cos < 0) {
                        if (alpha > 0) alpha = PI - alpha;
                        else alpha = -PI - alpha;
                    }
                }

                float target_alpha = (2 * PI / 3) * signcross;
                result += (alpha - target_alpha) * (alpha - target_alpha);
            }
    }

    return result;
}