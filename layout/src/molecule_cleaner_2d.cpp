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

#include "layout/molecule_cleaner_2d.h"
#include "graph/biconnected_decomposer.h"
#include "molecule/molecule.h"
#include <algorithm> 
#include <vector>
#include "base_cpp/profiling.h"

using namespace indigo;

//IMPL_ERROR(MoleculeCleaner2d, "MoleculeCleaner2d");

MoleculeCleaner2d::MoleculeCleaner2d(Molecule& mol) : _mol(mol) {
    vertex_size = _mol.vertexEnd();
//    printf("%d\n", vertex_count);
//    printf("%d\n", component_count);
    _initComponents(true);

    if (is_trivial) return;

    _initArtPoints();
    _initAdjMatrix();
    _calcTargetLen();
    _initCommonComp();

    /*printf("%d components\n", component_count);
    for (int i = 0; i < component_count; i++) {
    printf("%d: ", i);
    for (int j = 0; j < vertex_count; j++) if (in[i][j]) printf("%d, ", j);
    printf("|| ");
    printf("%d: ", definiting_points[i].size());
    for (int j = 0; j < definiting_points[i].size(); j++) printf("%d, ", definiting_points[i][j]);
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
    


void MoleculeCleaner2d::_initComponents(bool use_beconnected_decomposition) {
    pos.clear_resize(vertex_size);
    base_point.clear();
    in.clear();

    for (int i = 0; i < vertex_size; i++) {
        coef.push();
        coef.top().clear();
    }

    if (use_beconnected_decomposition) {
        BiconnectedDecomposer bi_decomposer(_mol);
        component_count = bi_decomposer.decompose();

        for (int i = 0; i < component_count; i++) {
            in.push();
            in.top().clear_resize(vertex_size);
            in.top().zerofill();
        }

        Filter filter;
        for (int i = 0; i < component_count; i++) {
            bi_decomposer.getComponent(i, filter);
            for (int j = _mol.vertexBegin(); j != _mol.vertexEnd(); j = _mol.vertexNext(j)) in[i][j] = filter.valid(j);
        }

        definiting_points.clear();
        for (int i = 0; i < component_count; i++) {
            definiting_points.push();
            definiting_points.top().clear();
        }


        QS_DEF(Array<bool>, has_component);
        QS_DEF(Array<int>, component_list);
        QS_DEF(Array<bool>, has_vertex);
        QS_DEF(Array<bool>, block_vertex);
        has_component.clear_resize(component_count);
        has_component.zerofill();
        component_list.clear();
        has_vertex.clear_resize(vertex_size);
        has_vertex.zerofill();
        block_vertex.clear_resize(vertex_size);
        block_vertex.zerofill();




        QS_DEF(Array<int>, local_component_list);

        int index = 0;
        for (int c = 0; c < component_count; c++) if (!has_component[c]) {

            int ver = -1;
            for (int i = _mol.vertexBegin(); i != _mol.vertexEnd(); i = _mol.vertexNext(i)) if (in[c][i]) if (bi_decomposer.isArticulationPoint(i)) {
                ver = i;
                break;
            }
            base_point.push(ver);
            _addCoef(ver, base_point.size() - 1, ONE);

            has_vertex[ver] = true;
            block_vertex[ver] = true;

            has_component[c] = true;
            component_list.push(c);

            for (; index < component_list.size(); index++) {
                // 1. Search for new vertex
                int comp = component_list[index];
                ver = -1;
                for (int j = _mol.vertexBegin(); j != _mol.vertexEnd(); j = _mol.vertexNext(j))
                    if (in[comp][j] && !block_vertex[j] && bi_decomposer.isArticulationPoint(j)) {
                        ver = j;
                        break;
                    }
                if (ver == -1) {
                    for (int j = _mol.vertexBegin(); j != _mol.vertexEnd(); j = _mol.vertexNext(j))
                        if (in[comp][j] && !block_vertex[j]) {
                            ver = j;
                            break;
                        }
                }
                base_point.push(ver);
                _addCoef(ver, base_point.size() - 1, ONE);
                has_vertex[ver] = true;

                // 2. Add yet another defining point if it is need

                for (int j = 0; j < base_point.size(); j++) if (in[comp][base_point[j]]) definiting_points[comp].push(base_point[j]);
                if (definiting_points[comp].size() < 2) {
                    int newver = -1;
                    for (int j = _mol.vertexBegin(); j != _mol.vertexEnd(); j = _mol.vertexNext(j)) if (block_vertex[j] && in[comp][j]) {
                        newver = j;
                        break;
                    }

                    definiting_points[comp].push(newver);
                }

                // 3. Calculation coefficients

                for (int j = _mol.vertexBegin(); j != _mol.vertexEnd(); j = _mol.vertexNext(j))
                    if (in[comp][j] && j != definiting_points[comp][0] && j != definiting_points[comp][1] && !block_vertex[j])
                        _calcÑoef(j, definiting_points[comp][0], definiting_points[comp][1]);

                // 4. Add new components to list

                for (int v = _mol.vertexBegin(); v != _mol.vertexEnd(); v = _mol.vertexNext(v)) if (in[comp][v] && bi_decomposer.isArticulationPoint(v)) {
                    for (int j = 0; j < component_count; j++) if (in[j][v] && !has_component[j]) {
                        component_list.push(j);
                        has_component[j] = true;
                    }
                }
                for (int v = _mol.vertexBegin(); v != _mol.vertexEnd(); v = _mol.vertexNext(v)) if (in[comp][v]) block_vertex[v] = true;
            }
        }


    } else {
        component_count = _mol.edgeCount();

        
        for (int i = 0; i < component_count; i++) {
            in.push();
            in.top().clear_resize(vertex_size);
            in.top().zerofill();
        }

        for (int i = 0, e = _mol.edgeBegin(); e != _mol.edgeEnd(); i++, e = _mol.edgeNext(e)) {
            const Edge& edge = _mol.getEdge(e);
            in[i][edge.beg] = true;
            in[i][edge.end] = true;
        }

        definiting_points.clear();
        for (int i = 0; i < component_count; i++) {
            definiting_points.push();
            definiting_points.top().clear();
        }

        for (int i = _mol.vertexBegin(); i != _mol.vertexEnd(); i = _mol.vertexNext(i)) {
            base_point.push(i);
            _addCoef(i, i, ONE);
        }
        for (int i = 0; i < component_count; i++) {
            for (int j = _mol.vertexBegin(); j != _mol.vertexEnd(); j = _mol.vertexNext(j)) if (in[i][j]) definiting_points[i].push(j);
        }
    }

    is_trivial = component_count <= 1;

    _initBasePointIndex();
    _initGeometry();
}

void MoleculeCleaner2d::_initBasePointIndex() {
    base_point_index.clear_resize(vertex_size);
    base_point_index.fffill();
    for (int i = 0; i < base_point.size(); i++) base_point_index[base_point[i]] = i;
}

void MoleculeCleaner2d::_initGeometry() {
    gradient.clear_resize(base_point.size());
    pregradient.clear_resize(vertex_size);
    for (int i = _mol.vertexBegin(); i != _mol.vertexEnd(); i = _mol.vertexNext(i)) if (_isBasePoint(i)) pos[i] = plane(_mol.getAtomXyz(i));
}

void MoleculeCleaner2d::_initArtPoints() {
    is_art_point.clear_resize(vertex_size);
    is_art_point.zerofill();
    for (int i = _mol.vertexBegin(); i != _mol.vertexEnd(); i = _mol.vertexNext(i)) {
        int cnt = 0;
        for (int j = 0; j < component_count; j++) {
            cnt += in[j][i];
        }
        if (cnt > 1) is_art_point[i] = true;
    }
}

void MoleculeCleaner2d::_initAdjMatrix() {
    adj_matrix.clear();
    for (int i = 0; i < vertex_size; i++) {
        adj_matrix.push();
        adj_matrix.top().clear_resize(vertex_size);
        adj_matrix.top().zerofill();
    }
    for (int e = _mol.edgeBegin(); e != _mol.edgeEnd(); e = _mol.edgeNext(e)) {
        Edge Ed = _mol.getEdge(e);
        adj_matrix[Ed.beg][Ed.end] = adj_matrix[Ed.end][Ed.beg] = true;
    }
}

void MoleculeCleaner2d::_calcTargetLen() {
    std::vector<float> lens;
    for (int e = _mol.edgeBegin(); e != _mol.edgeEnd(); e = _mol.edgeNext(e))
        lens.push_back(Vec2f::dist(plane(_mol.getAtomXyz(_mol.getEdge(e).beg)), plane(_mol.getAtomXyz(_mol.getEdge(e).end))));

    sort(lens.begin(), lens.end());

    target_len = lens.size() % 2 == 1 ? lens[lens.size() / 2] : (lens[lens.size() / 2] + lens[lens.size() / 2 - 1]) / 2;
}

void MoleculeCleaner2d::_initCommonComp() {
    common_comp.clear();
    for (int i = 0; i < vertex_size; i++) {
        common_comp.push();
        common_comp.top().clear_resize(vertex_size);
        common_comp.top().fffill();
        for (int j = _mol.vertexBegin(); j != _mol.vertexEnd(); j = _mol.vertexNext(j))
            for (int c = 0; c < component_count; c++) if (in[c][i] && in[c][j]) common_comp[i][j] = c;
    }

    edge_comp.clear_resize(_mol.edgeEnd());

    for (int e = _mol.edgeBegin(); e != _mol.edgeEnd(); e = _mol.edgeNext(e)) {
        int v = _mol.getEdge(e).beg, u = _mol.getEdge(e).end;
        edge_comp[e] = common_comp[v][u];
    }
}

bool MoleculeCleaner2d::_isBasePoint(int i) { return base_point_index[i] >= 0; }

void MoleculeCleaner2d::_addCoef(int ver, int index, Vec2f value) {
    while (coef[ver].size() <= index) coef[ver].push(ZERO);
    coef[ver][index] += value;
}

void MoleculeCleaner2d::_calcÑoef(int to, int from0, int from1) {
    Vec2f A0 = plane(_mol.getAtomXyz(from0));
    Vec2f A1 = plane(_mol.getAtomXyz(from1));
    Vec2f A2 = plane(_mol.getAtomXyz(to));

    Vec2f vec = A1 - A0;
    float dist2 = vec.lengthSqr();

    float cross = Vec2f::cross(A1 - A0, A2 - A0); // c1
    float dot = Vec2f::dot(A1 - A0, A2 - A0); // c2

    Vec2f _coef = Vec2f(dot, cross) / dist2;

    int len = std::max(coef[from0].size(), coef[from1].size());
    _addCoef(from0, len - 1, ZERO);
    _addCoef(from1, len - 1, ZERO);

    for (int i = 0; i < len; i++) {
        _addCoef(to, i, mult(_coef, coef[from1][i]));
        Vec2f one_minus_coef = ONE - _coef;
        _addCoef(to, i, mult(one_minus_coef, coef[from0][i]));
    }
}

void MoleculeCleaner2d::_updatePosition(int i) {
    pos[i] = ZERO;
    for (int j = 0; j < coef[i].size(); j++) pos[i] += mult(coef[i][j], pos[base_point[j]]);
}

void MoleculeCleaner2d::_updatePositions() {
    profTimerStart(t, "Update positions");
    for (int i = _mol.vertexBegin(); i != _mol.vertexEnd(); i = _mol.vertexNext(i)) if (!_isBasePoint(i)) _updatePosition(i);
}

void MoleculeCleaner2d::_updateGradient() {
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
        for (int i = _mol.vertexBegin(); i != _mol.vertexEnd(); i = _mol.vertexNext(i))
            for (int j = _mol.vertexBegin(); j < i; j = _mol.vertexNext(j)) if (common_comp[i][j] == -1) {
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
        for (int i = _mol.vertexBegin(); i != _mol.vertexEnd(); i = _mol.vertexNext(i)) if (is_art_point[i]) {
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
    for (int i = _mol.vertexBegin(); i != _mol.vertexEnd(); i = _mol.vertexNext(i))
        for (int j = 0; j < coef[i].size(); j++) gradient[j] += mult(pregradient[i], coef[i][j]);
}

void MoleculeCleaner2d::_updateGradient2() {
    profTimerStart(t, "Update gradient 2");
    for (int i = 0; i < base_point.size(); i++)
        gradient[i] = _energyDiff(base_point[i]);
}

void MoleculeCleaner2d::clean() {
    if (is_trivial) return; // nothing to do for biconnected graph

    _updatePositions();

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
    for (int iter = 0; iter < 1000; iter++) {
        _updateGradient2();
        /*if (iter == 99) {
            for (int i = 0; i < gradient.size(); i++) printf("%d: (%.5f, %.5f) : (%.5f, %.5f)\n", base_point[i], gradient[i].x, gradient[i].y, dgradient[i].x, dgradient[i].y);
        }*/
        float len = 0;
        for (int i = 0; i < base_point.size(); i++) len += gradient[i].lengthSqr();
        len = sqrt(len);
        float factor = need_len / len;
        for (int i = 0; i < base_point.size(); i++) gradient[i] *= factor;

        profTimerStart(t1, "Find len");
        for (int i = 0; i <= k; i++) {
            for (int j = 0; j < base_point.size(); j++) pos[base_point[j]] -= gradient[j] * mult[i];
            _updatePositions();
            energies[i] = _energy();
            for (int j = 0; j < base_point.size(); j++) pos[base_point[j]] += gradient[j] * mult[i];
        }
        profTimerStop(t1);

        int best_i = 0;
        for (int i = 1; i <= k; i++) if (energies[i] < energies[best_i]) best_i = i;
        //printf("%d\n", best_i);

        for (int i = 0; i < base_point.size(); i++) pos[base_point[i]] -= gradient[i] * mult[best_i];
        _updatePositions();

        if (best_i == 0) break;
        //need_len *= .95;
    }

    for (int i = _mol.vertexBegin(); i != _mol.vertexEnd(); i = _mol.vertexNext(i)) _mol.setAtomXyz(i, Vec3f(pos[i].x, pos[i].y, 0));
}

float MoleculeCleaner2d::_energy() {
    profTimerStart(t, "Total energy");
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
        for (int i = _mol.vertexBegin(); i != _mol.vertexEnd(); i = _mol.vertexNext(i))
            for (int j = _mol.vertexBegin(); j < i; j = _mol.vertexNext(j)) if (common_comp[i][j] == -1) {
            float dist2 = Vec2f::distSqr(pos[i], pos[j]);
            if (dist2 < target_len * target_len) {
                float dist = sqrt(dist2);
                float diff = (dist - target_len) / target_len;
                result += diff * diff;
            }
        }
    //printf("\n");

    // 3. angles

    for (int i = _mol.vertexBegin(); i != _mol.vertexEnd(); i = _mol.vertexNext(i)) if (is_art_point[i]) {
        const Vertex& vert = _mol.getVertex(i);
        for (int n1 = vert.neiBegin(); n1 != vert.neiEnd(); n1 = vert.neiNext(n1))
            for (int n2 = vert.neiBegin(); n2 < n1; n2 = vert.neiNext(n2)) {
                int v1 = vert.neiVertex(n1);
                int v2 = vert.neiVertex(n2);
                if (common_comp[v1][v2] >= 0) continue;

                //printf("%d %d %d\n", i, v1, v2);

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

Vec2f MoleculeCleaner2d::_energyDiff(int v) {
    _updatePositions();
    float e = _localEnergy(v);
    pos[v].x += APPROX_STEP;
    _updatePositions();
    float ex = _localEnergy(v);
    pos[v].x -= APPROX_STEP;
    pos[v].y += APPROX_STEP;
    _updatePositions();
    float ey = _localEnergy(v);
    pos[v].y -= APPROX_STEP;

    return Vec2f(ex - e, ey - e) / APPROX_STEP;
}

float MoleculeCleaner2d::_localEnergy(int v) {
    //return _energy();
    float result = 0;

    for (int i = _mol.vertexBegin(); i != _mol.vertexEnd(); i = _mol.vertexNext(i)) if (v != i) result += _edgeEnergy(v, i);

    const Vertex& vert = _mol.getVertex(v);
    for (int n1 = vert.neiBegin(); n1 != vert.neiEnd(); n1 = vert.neiNext(n1)) {
        for (int n2 = vert.neiBegin(); n2 < n1; n2 = vert.neiNext(n2))
            result += _angleEnergy(v, vert.neiVertex(n1), vert.neiVertex(n2));

        int v1 = vert.neiVertex(n1);
        const Vertex& vert1 = _mol.getVertex(v1);
        for (int n2 = vert1.neiBegin(); n2 != vert1.neiEnd(); n2 = vert1.neiNext(n2))
            if (vert1.neiVertex(n2) != v)
                result += _angleEnergy(v1, v, vert1.neiVertex(n2));
    }

    return result;
}

float MoleculeCleaner2d::_edgeEnergy(int i, int j) {
    profTimerStart(t, "Edge enegry");
    float len = Vec2f::distSqr(pos[i], pos[j]);

    if (len < target_len * target_len || adj_matrix[i][j]) {
        len = sqrt(len);
        float diff = (len - target_len) / target_len;
        return diff * diff;
    }
    return 0;
}

float MoleculeCleaner2d::_angleEnergy(int i, int v1, int v2) {
    profTimerStart(t, "Angle enegry");
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
    return (alpha - target_alpha) * (alpha - target_alpha);

}