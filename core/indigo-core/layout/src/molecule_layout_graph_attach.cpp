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

#include "layout/molecule_layout_graph.h"

using namespace indigo;

void MoleculeLayoutGraphSimple::_setChainType(const Array<int>& chain, const Array<int>& mapping, int type)
{
    for (int i = 0; i < chain.size() - 1; i++)
    {
        if (i > 0)
            _layout_vertices[mapping[chain[i]]].type = type;

        const Vertex& vert = getVertex(mapping[chain[i]]);
        int edge_idx = vert.neiEdge(vert.findNeiVertex(mapping[chain[i + 1]]));

        _layout_edges[edge_idx].type = type;
    }
}

bool MoleculeLayoutGraphSimple::_splitCycle(const Cycle& cycle, const Array<int>& cycle_vertex_types, bool check_boundary, Array<int>& chain_ext,
                                            Array<int>& chain_int, int& c_beg, int& c_end) const
{
    int i, j, k;

    // 1. First vertex is drawn
    if (cycle_vertex_types[0] != ELEMENT_NOT_DRAWN)
    {
        // 1. Find first not drawn
        for (i = 0; i < cycle.vertexCount(); i++)
            if (cycle_vertex_types[i] == ELEMENT_NOT_DRAWN)
                break;

        // 2. Find the last not drawn
        for (j = i; j < cycle.vertexCount(); j++)
            if (cycle_vertex_types[j] != ELEMENT_NOT_DRAWN)
                break;
        j--;

        // 3. Check other are drawn
        for (k = j + 1; k < cycle.vertexCount(); k++)
            if (cycle_vertex_types[k] == ELEMENT_NOT_DRAWN)
                return false;

        // 4. Check boundary vertices are marked as boundary.
        // In sequence_layout, ELEMENT_NOT_PLANAR (set by _attachCycleWithIntersections)
        // is also treated as a valid drawn boundary.  In standard layout, only
        // ELEMENT_BOUNDARY is accepted to preserve the original strict semantics.
        if (check_boundary)
        {
            int t_beg = cycle_vertex_types[i - 1];
            int t_end = cycle_vertex_types[(j + 1) % cycle.vertexCount()];
            bool beg_ok = (t_beg == ELEMENT_BOUNDARY) || (sequence_layout && t_beg == ELEMENT_NOT_PLANAR);
            bool end_ok = (t_end == ELEMENT_BOUNDARY) || (sequence_layout && t_end == ELEMENT_NOT_PLANAR);
            if (!beg_ok || !end_ok)
                return false;
        }

        // 5. Make internal and external chains
        c_beg = cycle.getVertex(i - 1);
        c_end = cycle.getVertex((j + 1) % cycle.vertexCount());

        chain_ext.clear();
        chain_int.clear();

        for (k = i - 1; k < j + 2; k++)
            chain_ext.push(cycle.getVertex(k % cycle.vertexCount()));
        for (k = i - 1; k >= 0; k--)
            chain_int.push(cycle.getVertex(k));
        for (k = cycle.vertexCount() - 1; k > j; k--)
            chain_int.push(cycle.getVertex(k));
    }
    else // First vertex is not drawn
    {
        // 1. Find first vertex after the last not drawn from the beginning
        for (i = 0; i < cycle.vertexCount(); i++)
            if (cycle_vertex_types[i] != ELEMENT_NOT_DRAWN)
                break;

        // 2. Find first vertex after the last not drawn from the end
        for (j = cycle.vertexCount() - 1; j >= 0; j--)
            if (cycle_vertex_types[j] != ELEMENT_NOT_DRAWN)
                break;

        // 3. Check other are drawn
        for (k = i; k < j + 1; k++)
            if (cycle_vertex_types[k] == ELEMENT_NOT_DRAWN)
                return false;

        // 4. Check boundary vertices are marked as boundary.
        // Same logic as above: ELEMENT_NOT_PLANAR allowed only in sequence_layout.
        if (check_boundary)
        {
            int t_beg2 = cycle_vertex_types[i];
            int t_end2 = cycle_vertex_types[j];
            bool beg_ok2 = (t_beg2 == ELEMENT_BOUNDARY) || (sequence_layout && t_beg2 == ELEMENT_NOT_PLANAR);
            bool end_ok2 = (t_end2 == ELEMENT_BOUNDARY) || (sequence_layout && t_end2 == ELEMENT_NOT_PLANAR);
            if (!beg_ok2 || !end_ok2)
                return false;
        }

        // 5. Make internal and external chains
        c_beg = cycle.getVertex(i);
        c_end = cycle.getVertex(j);

        chain_int.clear();
        chain_ext.clear();
        for (k = i; k < j + 1; k++)
            chain_int.push(cycle.getVertex(k));
        for (k = i; k >= 0; k--)
            chain_ext.push(cycle.getVertex(k));
        for (k = cycle.vertexCount() - 1; k > j - 1; k--)
            chain_ext.push(cycle.getVertex(k));
    }

    return true;
}

// Split cycle into separate chains which are not drawn
void MoleculeLayoutGraphSimple::_splitCycle2(const Cycle& cycle, const Array<int>& cycle_vertex_types, ObjArray<Array<int>>& chains_ext) const
{
    int i;

    chains_ext.clear();

    i = 0;

    while (cycle_vertex_types[i] == ELEMENT_NOT_DRAWN)
        i++;

    while (i < cycle.vertexCount())
    {
        for (; i < cycle.vertexCount(); i++)
            if (cycle_vertex_types[i] == ELEMENT_NOT_DRAWN)
                break;

        if (i == cycle.vertexCount())
            break;

        Array<int>& chain_ext = chains_ext.push();

        chain_ext.push(cycle.getVertex(i - 1));

        for (; i < cycle.vertexCount() && cycle_vertex_types[i] == ELEMENT_NOT_DRAWN; i++)
            chain_ext.push(cycle.getVertex(i));

        if (i < cycle.vertexCount() || cycle_vertex_types[0] != ELEMENT_NOT_DRAWN)
            chain_ext.push(cycle.getVertex(i % cycle.vertexCount()));
    }

    if (cycle_vertex_types[0] == ELEMENT_NOT_DRAWN)
    {
        i = cycle.vertexCount() - 1;

        Array<int>* chain_ext = 0;

        if (cycle_vertex_types[i] != ELEMENT_NOT_DRAWN)
        {
            chain_ext = &chains_ext.push();
            chain_ext->push(cycle.getVertex(i));
        }
        else
            chain_ext = &chains_ext.top();

        for (i = 0; cycle_vertex_types[i] == ELEMENT_NOT_DRAWN; i++)
            chain_ext->push(cycle.getVertex(i));

        chain_ext->push(cycle.getVertex(i));
    }
}

// Attach cycle outside component border. Component must have given number of common edges or any (if 0)
bool MoleculeLayoutGraphSimple::_attachCycleOutside(const Cycle& cycle, float length, int n_common_edges)
{
    int n_common_e = 0, n_common_v = 0;
    QS_DEF(Array<int>, cycle_vertex_types);

    cycle_vertex_types.clear_resize(cycle.vertexCount());
    cycle_vertex_types.zerofill();

    for (int i = 0; i < cycle.vertexCount(); i++)
    {
        cycle_vertex_types[i] = _layout_vertices[cycle.getVertex(i)].type;
        if (cycle_vertex_types[i] > 0)
            n_common_v++;
        if (_layout_edges[cycle.getEdge(i)].type > 0)
            n_common_e++;
    }

    if (n_common_edges > 0 && n_common_e != n_common_edges)
        return false;

    // Everything is drawn
    if (n_common_e == cycle.vertexCount())
        return true;

    // If all vertices are drawn then draw edges without intersections with already drawn
    // Find new border
    // If then all edges are drawn return true, else return false
    if (n_common_v == cycle.vertexCount())
        return _drawEdgesWithoutIntersection(cycle, cycle_vertex_types);

    // Attach cycle of two parts:
    //   chain_int - internal and boundary vertices of component
    //   chain_ext - not drawn vertices and two boundary
    // If number of common vertices is less than 2 then skip cycle
    if (n_common_v < 2)
        return false;

    QS_DEF(Array<int>, chain_ext);
    QS_DEF(Array<int>, chain_int);
    int c_beg, c_end;

    if (!_splitCycle(cycle, cycle_vertex_types, true, chain_ext, chain_int, c_beg, c_end))
    {
        if (sequence_layout)
        {
            // Multiple separated NOT_DRAWN blocks (or isolated NOT_DRAWN vertices).
            // Robust strategy: identify only the "arc-consistent" drawn vertices —
            // those that look like they belong to the same regular-polygon arc
            // (mutual distances ≈ bond multiples of bond_length).  Use only those
            // to compute the circumscribed circle, then place each NOT_DRAWN vertex
            // at the angular midpoint between its drawn neighbors on that circle.

            // Collect drawn vertices and their positions.
            QS_DEF(Array<int>, drawn_idx); // index into cycle
            drawn_idx.clear();
            for (int di = 0; di < cycle.vertexCount(); di++)
                if (cycle_vertex_types[di] > 0)
                    drawn_idx.push(di);

            if (drawn_idx.size() < 2)
                return false;

            // Estimate bond_length from adjacent drawn-vertex pairs in the cycle.
            float sum_bond = 0.f;
            int n_bond = 0;
            int nc = cycle.vertexCount();
            for (int bi = 0; bi < drawn_idx.size(); bi++)
            {
                int di = drawn_idx[bi];
                int di_next = (di + 1) % nc;
                if (cycle_vertex_types[di_next] > 0)
                {
                    float d = Vec2f::dist(_layout_vertices[cycle.getVertex(di)].pos, _layout_vertices[cycle.getVertex(di_next)].pos);
                    sum_bond += d;
                    n_bond++;
                }
            }
            float est_bond = (n_bond > 0) ? sum_bond / n_bond : length;

            // Keep only drawn verts whose bond-distance to a drawn cycle-neighbor
            // is ≤ est_bond × kArcBondTolerance.
            // Rationale: adjacent ring vertices have d ≈ bond_length (= est_bond).
            // Backbone-grid vertices can sit at an oblique angle where the in-cycle
            // distance to the next drawn vertex ≈ sqrt(2) × bond_length ≈ 1.41.
            // A threshold of 1.3 sits between these two cases: arc neighbours pass,
            // non-arc backbone neighbours are rejected.
            const float kArcBondTolerance = 1.3f;
            QS_DEF(Array<int>, arc_idx);
            arc_idx.clear();
            for (int bi = 0; bi < drawn_idx.size(); bi++)
            {
                int di = drawn_idx[bi];
                bool ok = false;
                int di_prev = (di - 1 + nc) % nc;
                int di_next = (di + 1) % nc;
                if (cycle_vertex_types[di_prev] > 0)
                {
                    float d = Vec2f::dist(_layout_vertices[cycle.getVertex(di)].pos, _layout_vertices[cycle.getVertex(di_prev)].pos);
                    if (d < est_bond * kArcBondTolerance)
                        ok = true;
                }
                if (!ok && cycle_vertex_types[di_next] > 0)
                {
                    float d = Vec2f::dist(_layout_vertices[cycle.getVertex(di)].pos, _layout_vertices[cycle.getVertex(di_next)].pos);
                    if (d < est_bond * kArcBondTolerance)
                        ok = true;
                }
                if (ok)
                    arc_idx.push(di);
            }

            if (arc_idx.size() < 2)
                return false;

            // Compute centroid and mean radius from arc-consistent drawn verts only.
            float cx = 0.f, cy = 0.f;
            for (int bi = 0; bi < arc_idx.size(); bi++)
            {
                cx += _layout_vertices[cycle.getVertex(arc_idx[bi])].pos.x;
                cy += _layout_vertices[cycle.getVertex(arc_idx[bi])].pos.y;
            }
            cx /= arc_idx.size();
            cy /= arc_idx.size();

            float radius = 0.f;
            for (int bi = 0; bi < arc_idx.size(); bi++)
            {
                float dx = _layout_vertices[cycle.getVertex(arc_idx[bi])].pos.x - cx;
                float dy = _layout_vertices[cycle.getVertex(arc_idx[bi])].pos.y - cy;
                radius += sqrtf(dx * dx + dy * dy);
            }
            radius /= arc_idx.size();
            if (radius < 0.01f)
                return false;

            // Check consistency: spread of radii of arc verts must be < 15% of radius.
            float rmin = radius, rmax = radius;
            for (int bi = 0; bi < arc_idx.size(); bi++)
            {
                float dx = _layout_vertices[cycle.getVertex(arc_idx[bi])].pos.x - cx;
                float dy = _layout_vertices[cycle.getVertex(arc_idx[bi])].pos.y - cy;
                float r = sqrtf(dx * dx + dy * dy);
                if (r < rmin)
                    rmin = r;
                if (r > rmax)
                    rmax = r;
            }
            if (rmax - rmin > 0.15f * radius)
                return false; // positions too inconsistent to derive a reliable circle

            // Place each NOT_DRAWN vertex at angular midpoint between its drawn
            // cycle-neighbors (using the full drawn_idx set — backbone verts are OK
            // as angular references, they just can't define the circle).
            bool any_placed = false;
            for (int di = 0; di < nc; di++)
            {
                if (cycle_vertex_types[di] != 0)
                    continue; // already drawn

                // Find prev/next drawn neighbors (wrap around).
                int prev_di = (di - 1 + nc) % nc;
                int next_di = (di + 1) % nc;
                int steps = 0;
                while (cycle_vertex_types[prev_di] == 0 && steps++ < nc)
                    prev_di = (prev_di - 1 + nc) % nc;
                steps = 0;
                while (cycle_vertex_types[next_di] == 0 && steps++ < nc)
                    next_di = (next_di + 1) % nc;
                if (cycle_vertex_types[prev_di] == 0 || cycle_vertex_types[next_di] == 0)
                    continue;

                // Use only arc-consistent neighbors to compute angle.
                // Prefer arc_idx neighbors; if neither is in arc_idx, skip.
                bool prev_arc = false, next_arc = false;
                for (int bi = 0; bi < arc_idx.size(); bi++)
                {
                    if (arc_idx[bi] == prev_di)
                    {
                        prev_arc = true;
                        break;
                    }
                }
                for (int bi = 0; bi < arc_idx.size(); bi++)
                {
                    if (arc_idx[bi] == next_di)
                    {
                        next_arc = true;
                        break;
                    }
                }
                if (!prev_arc && !next_arc)
                    continue;

                int vp = cycle.getVertex(prev_di);
                int vn = cycle.getVertex(next_di);
                float ap = atan2f(_layout_vertices[vp].pos.y - cy, _layout_vertices[vp].pos.x - cx);
                float an = atan2f(_layout_vertices[vn].pos.y - cy, _layout_vertices[vn].pos.x - cx);
                float diff = an - ap;
                while (diff > (float)M_PI)
                    diff -= 2.f * (float)M_PI;
                while (diff < -(float)M_PI)
                    diff += 2.f * (float)M_PI;
                float a_mid = ap + 0.5f * diff;

                int v = cycle.getVertex(di);
                _layout_vertices[v].pos.x = cx + radius * cosf(a_mid);
                _layout_vertices[v].pos.y = cy + radius * sinf(a_mid);
                _layout_vertices[v].type = ELEMENT_BOUNDARY;
                _layout_vertices[v].is_nailed = true;
                any_placed = true;
            }
            return any_placed;
        }
        return false;
    }

    int i, k;
    bool is_attached = false;
    QS_DEF(Array<int>, border1v);
    QS_DEF(Array<int>, border1e);
    QS_DEF(Array<int>, border2v);
    QS_DEF(Array<int>, border2e);
    Vec2f p;

    // Make Border1, Border2 from component border (borders have two common vertices)
    // First check that both vertices are on the border
    Cycle border;
    _getBorder(border);

    bool use_chain_int_border = false;
    if (border.findVertex(c_beg) == -1 || border.findVertex(c_end) == -1)
    {
        // _getBorder returned a border that doesn't include our attachment points.
        // This can happen when pre-marked backbone cycles confuse the boundary traversal.
        // Fall back to using chain_int as border1 directly. chain_int = the drawn path
        // through the already-laid-out cycle (exactly what we need for attachment).
        if (sequence_layout)
            use_chain_int_border = true;
        else
            return false;
    }

    if (!use_chain_int_border)
        _splitBorder(c_beg, c_end, border1v, border1e, border2v, border2e);
    else
    {
        // Use chain_int as border1 (direct path through the drawn cycle).
        border1v.copy(chain_int);
        border1e.clear();
        // Build border1e from chain_int vertices using edge lookups.
        // chain_int is a path through the drawn cycle so adjacent vertices
        // are guaranteed to share an edge; nei == -1 would be a bug.
        for (int bi = 0; bi + 1 < chain_int.size(); bi++)
        {
            const Vertex& bv = getVertex(chain_int[bi]);
            int ei = bv.findNeiVertex(chain_int[bi + 1]);
            border1e.push(ei >= 0 ? bv.neiEdge(ei) : -1);
        }
        // border2 = short/empty (just c_beg-c_end direct if exists, else empty)
        border2v.clear();
        border2e.clear();
        border2v.push(c_beg);
        border2v.push(c_end);
    }

    QS_DEF(MoleculeLayoutGraphSimple, next_bc);
    QS_DEF(Array<int>, mapping);

    // n_try iteration plan:
    //   0 — ccw=true,  with convexity checks  (standard)
    //   1 — ccw=false, with convexity checks  (standard)
    //   2 — ccw=true,  bypass checks (sequence_layout concave fused ring)
    //   3 — ccw=false, bypass checks
    // Bypass passes are enabled when the shared boundary has 3+ edges, which
    // indicates a fused multi-ring where the attachment region is concave and
    // the convexity checks give false negatives.
    static const int kMaxTryStandard = 2;       // passes 0-1
    static const int kMaxTryWithBypass = 4;     // passes 0-3
    static const int kBypassMinSharedEdges = 3; // shared edges threshold for bypass
    int max_try = (sequence_layout && n_common_e >= kBypassMinSharedEdges) ? kMaxTryWithBypass : kMaxTryStandard;

    for (int n_try = 0; n_try < max_try && !is_attached; n_try++)
    {
        // Complete regular polygon by chain_ext (on the one side if n_try == 1 and other side if n_try == 2
        next_bc.cloneLayoutGraph(*this, &mapping);

        if (length > 1)
        {
            k = chain_ext.size() - 2;
            float dist = Vec2f::dist(_layout_vertices[c_beg].pos, _layout_vertices[c_end].pos);

            if (dist > (k + 1) * length)
                length = 0.2f + dist / (k + 1);
        }

        if (n_try == 0)
        {
            if (!next_bc._drawRegularCurveEx(chain_ext, c_beg, c_end, length, true, ELEMENT_BOUNDARY, mapping))
                return false;
        }
        else if (n_try == 1)
        {
            if (!next_bc._drawRegularCurveEx(chain_ext, c_beg, c_end, length, false, ELEMENT_BOUNDARY, mapping))
                continue; // try bypass modes
        }
        else if (n_try == 2)
        {
            // Bypass mode: force ccw=true, skip convexity checks below
            if (!next_bc._drawRegularCurveEx(chain_ext, c_beg, c_end, length, true, ELEMENT_BOUNDARY, mapping))
                continue;
        }
        else // n_try == 3
        {
            // Bypass mode: force ccw=false, skip convexity checks below
            if (!next_bc._drawRegularCurveEx(chain_ext, c_beg, c_end, length, false, ELEMENT_BOUNDARY, mapping))
                continue;
        }

        bool bypass_checks = (n_try >= 2);

        if (!bypass_checks && !_checkBadTryChainOutside(chain_ext, next_bc, mapping))
            continue;

        // Check edges from chain_ext intersect previous border other than in the ends
        if (!bypass_checks && !_checkBadTryBorderIntersection(chain_ext, next_bc, mapping))
            continue;

        // If Border1 lays inside cycle [chain_ext,border2] than it becomes internal and Border2 becomes boundary
        // If Border2 lays inside cycle [chain_ext,border1] than it becomes internal and Border1 becomes boundary
        // In both cases chain_ext becomes external.
        // Ignore border1 and check if vertices are inside new bound
        // if no then restore Border1 and ignore Border2
        next_bc._setChainType(border1v, mapping, ELEMENT_IGNORE);

        p.lineCombin2(next_bc._layout_vertices[mapping[border1v[0]]].pos, 0.9f, next_bc._layout_vertices[mapping[border1v[1]]].pos, 0.1f);

        if (!next_bc._isPointOutside(p))
            next_bc._setChainType(border1v, mapping, ELEMENT_INTERNAL);
        else
        {
            next_bc._setChainType(border1v, mapping, ELEMENT_BOUNDARY);
            next_bc._setChainType(border2v, mapping, ELEMENT_INTERNAL);
        }
        // Replace chain_ext by single edge and check if chain_ext is outside (try to draw convex polygon)
        if (n_try == 0 && chain_ext.size() > 2)
        {
            next_bc._setChainType(chain_ext, mapping, ELEMENT_IGNORE);

            const Vertex& vert = next_bc.getVertex(mapping[c_beg]);
            int edge_idx;
            int type = -1;

            if ((edge_idx = vert.findNeiVertex(mapping[c_end])) != -1)
            {
                edge_idx = vert.neiEdge(edge_idx);
                type = next_bc._layout_edges[edge_idx].type;
                next_bc._layout_edges[edge_idx].type = ELEMENT_BOUNDARY;
            }
            else
                edge_idx = next_bc.addLayoutEdge(mapping[c_beg], mapping[c_end], -1, ELEMENT_BOUNDARY);

            if (!next_bc._isPointOutside(next_bc._layout_vertices[mapping[chain_ext[1]]].pos))
                continue;

            next_bc._setChainType(chain_ext, mapping, ELEMENT_BOUNDARY);

            if (type < 0)
                next_bc.removeEdge(edge_idx);
            else
                next_bc._layout_edges[edge_idx].type = type;
        }
        // Check if border1, border2 are outside cycle
        // (draw cycle outside not inside)
        // n_try=2 is ccw=true bypass (same check as n_try=0)
        // n_try=3 is ccw=false bypass (always accepted like n_try=1)
        if (n_try == 0 || n_try == 2)
        {
            for (i = 1; i < border1v.size() - 1 && !is_attached; i++)
                if (next_bc._isPointOutsideCycleEx(cycle, next_bc._layout_vertices[mapping[border1v[i]]].pos, mapping))
                    is_attached = true;

            for (i = 1; i < border2v.size() - 1 && !is_attached; i++)
                if (next_bc._isPointOutsideCycleEx(cycle, next_bc._layout_vertices[mapping[border2v[i]]].pos, mapping))
                    is_attached = true;
        }
        else
            is_attached = true;
    }

    // Copy the new layout back.
    // We use a full copyLayoutTo (needed to keep the border graph valid), but
    // then restore NOT_DRAWN status for any vertex that (a) was NOT_DRAWN before
    // this attachment and (b) does NOT belong to the current cycle.
    // Without this, border1v/border2v type mutations (IGNORE → INTERNAL → BOUNDARY)
    // propagate to future cycles and inflate n_cv (causing layout failures).
    if (is_attached)
    {
        // Mark cycle vertices as nailed before the copy (original behaviour).
        // Without this, copyLayoutTo may reposition already-drawn atoms.
        for (int ci = 0; ci < cycle.vertexCount(); ci++)
        {
            int cv = cycle.getVertex(ci);
            _layout_vertices[cv].is_nailed = true;
            _layout_vertices[cv].is_inside = true;
        }

        if (sequence_layout)
        {
            // In sequence_layout, border1v/border2v type mutations (IGNORE → INTERNAL)
            // from copyLayoutTo propagate to vertices that belong to future cycles,
            // incorrectly pre-marking them as drawn (n_cv inflation).
            // Fix: snapshot which vertices are NOT_DRAWN before the copy, then
            // restore that status for any vertex outside the just-attached cycle.

            // Record which vertices were NOT_DRAWN (future cycles).
            QS_DEF(Array<int>, was_not_drawn);
            was_not_drawn.clear();
            for (int vi = vertexBegin(); vi < vertexEnd(); vi = vertexNext(vi))
                if (_layout_vertices[vi].type == ELEMENT_NOT_DRAWN)
                    was_not_drawn.push(vi);

            // Build a set of current-cycle vertex indices for O(1) lookup.
            QS_DEF(Array<byte>, in_cycle);
            in_cycle.resize(vertexEnd());
            in_cycle.zerofill();
            for (int ci = 0; ci < cycle.vertexCount(); ci++)
                in_cycle[cycle.getVertex(ci)] = 1;

            // Full copy — restores all positions, types, and edge types correctly.
            next_bc.copyLayoutTo(*this, mapping);

            // Restore NOT_DRAWN for vertices outside the just-attached cycle.
            for (int k = 0; k < was_not_drawn.size(); k++)
            {
                int vi = was_not_drawn[k];
                if (vi < (int)in_cycle.size() && !in_cycle[vi])
                    _layout_vertices[vi].type = ELEMENT_NOT_DRAWN;
            }
        }
        else
        {
            // Standard layout: plain copy is correct.
            next_bc.copyLayoutTo(*this, mapping);
        }
    }

    return is_attached;
}

// Attach cycle inside component border.
// Everything can be attached outside is already attached.
bool MoleculeLayoutGraphSimple::_attachCycleInside(const Cycle& cycle, float length)
{
    int n_common_e = 0, n_common_v = 0;
    int i, j;

    QS_DEF(Array<int>, cycle_vertex_types);

    cycle_vertex_types.clear_resize(cycle.vertexCount());
    cycle_vertex_types.zerofill();

    for (i = 0; i < cycle.vertexCount(); i++)
    {
        cycle_vertex_types[i] = _layout_vertices[cycle.getVertex(i)].type;

        if (cycle_vertex_types[i] > 0)
            n_common_v++;
        if (_layout_edges[cycle.getEdge(i)].type > 0)
            n_common_e++;
    }

    // Everything is drawn
    if (n_common_e == cycle.vertexCount())
        return true;

    bool attached = false;

    // If all vertices are drawn then draw edges without intersections with already drawn
    // Find new border
    // If then all edges are drawn return true, else return false
    if (n_common_v == cycle.vertexCount())
    {
        attached = true;

        for (i = 0; i < cycle.vertexCount(); i++)
        {
            if (_layout_edges[cycle.getEdge(i)].type == ELEMENT_NOT_DRAWN)
            {
                for (j = edgeBegin(); j < edgeEnd(); j = edgeNext(j))
                    if (_layout_edges[j].type > ELEMENT_NOT_DRAWN)
                        if ((_calcIntersection(i, j) % 10) != 1)
                        {
                            attached = false;
                            break;
                        }

                if (!attached)
                    continue;

                _layout_edges[cycle.getEdge(i)].type = ELEMENT_INTERNAL;
            }
        }

        return attached;
    }

    // Attach cycle of two parts:
    //   chain_int - internal and boundary vertices of component
    //   chain_ext - not drawn vertices and two boundary
    // If number of common vertices is less than 2 then skip cycle
    if (n_common_v < 2)
        return false;

    QS_DEF(Array<int>, chain_ext);
    QS_DEF(Array<int>, chain_int);
    int c_beg, c_end;

    if (!_splitCycle(cycle, cycle_vertex_types, false, chain_ext, chain_int, c_beg, c_end))
        return false;

    QS_DEF(MoleculeLayoutGraphSimple, next_bc);
    QS_DEF(Array<int>, mapping);

    for (int n_try = 0; n_try < 2 && !attached; n_try++)
    {
        // Complete regular polygon by chain_ext (on the one side if n_try == 1 and other side if n_try == 2
        next_bc.cloneLayoutGraph(*this, &mapping);

        if (n_try == 0)
        {
            if (!next_bc._drawRegularCurveEx(chain_ext, c_beg, c_end, length, true, ELEMENT_INTERNAL, mapping))
                return false;
        }
        else // (n_try == 1)
        {
            if (!next_bc._drawRegularCurveEx(chain_ext, c_beg, c_end, length, false, ELEMENT_INTERNAL, mapping))
                return false;
        }

        bool bad_try = false;

        // Check edges from chain_ext intersect previous border other than in the ends
        for (i = 0; i < chain_ext.size() - 1 && !bad_try; i++)
            for (j = next_bc.edgeBegin(); j < next_bc.edgeEnd(); j = next_bc.edgeNext(j))
            {
                if (_layout_edges[next_bc._layout_edges[j].ext_idx].type != ELEMENT_NOT_DRAWN)
                {
                    const Vertex& vert = next_bc.getVertex(mapping[chain_ext[i]]);
                    int edge1_idx = vert.neiEdge(vert.findNeiVertex(mapping[chain_ext[i + 1]]));
                    int intersect = next_bc._calcIntersection(edge1_idx, j);

                    const Edge& edge1 = next_bc.getEdge(edge1_idx);
                    const Edge& edge2 = next_bc.getEdge(j);

                    // Check if edges intersect
                    if ((intersect % 10) != 1 ||
                        (intersect == 21 && edge1.beg != edge2.beg && edge1.beg != edge2.end && edge1.end != edge2.beg && edge1.end != edge2.end))
                    {
                        bad_try = true;
                        break;
                    }
                }
            }

        if (!bad_try)
            attached = true;
    }

    // Copy new layout
    if (attached)
    {
        for (int i = 0; i < cycle.vertexCount(); i++)
        {
            _layout_vertices[cycle.getVertex(i)].is_nailed = true;
            _layout_vertices[cycle.getVertex(i)].is_inside = true;
        }
        next_bc.copyLayoutTo(*this, mapping);
    }
    return attached;
}

// Attach cycle with intersections.
// Everything can be attached w/o intersections is already attached.
// Not all cycle vertices are drawn
bool MoleculeLayoutGraphSimple::_attachCycleWithIntersections(const Cycle& cycle, float length)
{
    int n_common_e = 0, n_common_v = 0;
    int i, j, k;

    QS_DEF(Array<int>, cycle_vertex_types);

    cycle_vertex_types.clear_resize(cycle.vertexCount());
    cycle_vertex_types.zerofill();

    for (i = 0; i < cycle.vertexCount(); i++)
    {
        cycle_vertex_types[i] = _layout_vertices[cycle.getVertex(i)].type;

        if (cycle_vertex_types[i] > 0)
            n_common_v++;
        if (_layout_edges[cycle.getEdge(i)].type > 0)
            n_common_e++;
    }

    // All vertices are drawn - return false
    if (n_common_v == cycle.vertexCount())
        return false;

    // Attach cycle of two parts:
    //   chain_int - internal and boundary vertices of component
    //   chain_ext - not drawn vertices and two boundary
    // If number of common vertices is less than 2 then skip cycle
    if (n_common_v < 2)
        return false;

    QS_DEF(ObjArray<Array<int>>, chains_ext);

    // Split cycle into separate external (not drawn) chains
    _splitCycle2(cycle, cycle_vertex_types, chains_ext);

    // Attach each chain separately
    for (int chain_idx = 0; chain_idx < chains_ext.size(); chain_idx++)
    {
        Array<int>& chain_ext = chains_ext[chain_idx];
        int c_beg = chain_ext[0], c_end = chain_ext[chain_ext.size() - 1];

        float max_length = length * 4; // to avoid infinite values

        // Complete regular polygon by chain_ext (on the one side if n_try == 1 and other side if n_try == 2
        // Mark new vertices and edges as not planar
        k = chain_ext.size() - 2;
        float dist = Vec2f::dist(getPos(c_beg), getPos(c_end));

        if (dist > (k + 1) * length)
        {
            length = 0.2f + dist / (k + 1);
            max_length = std::max(max_length, length * 1.5f); // update max length if needed
        }

        bool attached = false;

        while (!attached && length < max_length)
        {
            attached = true;

            while (!_drawRegularCurve(chain_ext, c_beg, c_end, length, true, ELEMENT_NOT_PLANAR) && length < max_length)
                length *= 1.2f;

            // If we hit max_length without success, give up on this chain
            if (length >= max_length)
                break;

            // Choose position with minimal energy
            Vec2f& pos1 = getPos(chain_ext[1]);
            float s = 0;

            for (i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
                if (_layout_vertices[i].type == ELEMENT_INTERNAL || _layout_vertices[i].type == ELEMENT_BOUNDARY)
                {
                    Vec2f& pos = getPos(i);
                    s += Vec2f::distSqr(pos, pos1);
                }

            _drawRegularCurve(chain_ext, c_beg, c_end, length, false, ELEMENT_NOT_PLANAR);

            float sn = 0;

            for (i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
            {
                int type = _layout_vertices[i].type;

                if (type == ELEMENT_INTERNAL || type == ELEMENT_BOUNDARY)
                {
                    Vec2f& pos = getPos(i);
                    sn += Vec2f::distSqr(pos, pos1);
                }
            }

            if (sn < s - 0.001)
                _drawRegularCurve(chain_ext, c_beg, c_end, length, true, ELEMENT_NOT_PLANAR);

            // Try to change edge length to avoid bad layout
            for (i = 1; i < chain_ext.size() - 1; i++)
            {
                if (_isVertexOnSomeEdge(chain_ext[i]))
                {
                    length *= 1.2f;
                    attached = false;
                    break;
                }
            }

            if (!attached)
                continue;

            for (j = 0; j < chain_ext.size() - 1 && attached; j++)
            {
                for (i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
                {
                    int type = _layout_vertices[i].type;

                    if (i != chain_ext[j] && i != chain_ext[j + 1] && (type == ELEMENT_INTERNAL || type == ELEMENT_BOUNDARY))
                    {
                        if (_isVertexOnEdge(i, chain_ext[j], chain_ext[j + 1]))
                        {
                            length *= 1.2f;
                            attached = false;
                            break;
                        }
                    }
                }
            }
        }
    }

    // In sequence_layout, reclassify NOT_PLANAR vertices/edges to BOUNDARY so that
    // subsequent _attachCycleOutside/_getBorder can find them as valid attachment anchors.
    // In standard layout NOT_PLANAR is the intended final state — do not change it.
    if (sequence_layout)
    {
        for (i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
            if (_layout_vertices[i].type == ELEMENT_NOT_PLANAR)
                _layout_vertices[i].type = ELEMENT_BOUNDARY;
        for (i = edgeBegin(); i < edgeEnd(); i = edgeNext(i))
            if (_layout_edges[i].type == ELEMENT_NOT_PLANAR)
                _layout_edges[i].type = ELEMENT_BOUNDARY;
    }

    return true;
}

// Attach two atoms to the same side of chain
void MoleculeLayoutGraph::_attachEars(int vert_idx, int drawn_idx, int* ears, const Vec2f& rest_pos)
{
    Vec2f v1, v2, v3, v4;
    float phi = _2FLOAT(13. * M_PI / 24.);
    const Vertex& vert = getVertex(vert_idx);

    _layout_vertices[ears[0]].type = ELEMENT_IGNORE;
    _layout_vertices[ears[1]].type = ELEMENT_IGNORE;
    _layout_edges[vert.neiEdge(vert.findNeiVertex(ears[0]))].type = ELEMENT_BOUNDARY;
    _layout_edges[vert.neiEdge(vert.findNeiVertex(ears[1]))].type = ELEMENT_BOUNDARY;

    v1 = getPos(vert_idx);
    v2 = getPos(drawn_idx);
    _calculatePos(phi, v1, rest_pos, v3);
    _calculatePos(_2FLOAT(phi + 2. * M_PI / 3.), v1, rest_pos, v4);

    if (Vec2f::dist(v3, v2) < Vec2f::dist(v4, v2))
        v3 = v4;

    _layout_vertices[ears[0]].pos = v3;
    _calculatePos(_2FLOAT(M_PI / 4.), v1, v3, getPos(ears[1]));
}

// Attach set of trivial components
void MoleculeLayoutGraph::_attachDandlingVertices(int vert_idx, Array<int>& adjacent_list)
{
    int n_pos = 0, not_drawn_idx = 0, drawn_idx = -1;
    Vec2f v1, v2;
    QS_DEF(Array<Vec2f>, positions);
    int parity = 0;
    bool two_ears = false; // mark the case with two atoms to be drawn on the same side of chain

    const Vertex& vert = getVertex(vert_idx);

    // Calculate number of drawn edges
    for (int i = vert.neiBegin(); i < vert.neiEnd(); i = vert.neiNext(i))
    {
        if (getVertexType(vert.neiVertex(i)) != ELEMENT_NOT_DRAWN && getEdgeType(vert.neiEdge(i)) != ELEMENT_NOT_DRAWN)
        {
            n_pos++;
            // amount of drown neibourhoods
            drawn_idx = i;
        }
        else
            not_drawn_idx = i;
    }

    if (n_pos > 1)
    {
        if (adjacent_list.size() == 1)
            _calculatePositionsOneNotDrawn(positions, n_pos, vert_idx, not_drawn_idx); // n_pos of drawn edges and one not drawn
        else
            _calculatePositionsManyNotDrawn(vert_idx, adjacent_list, positions); // n_pos of drawn edges and more than one not drawn
    }
    else
    {
        // Single drawn edge
        _calculatePositionsSingleDrawn(vert_idx, adjacent_list, n_pos, drawn_idx, two_ears, positions, parity);
    }

    int ears[2] = {-1, -1};

    if (two_ears)
    {
        for (int i = 0; i < adjacent_list.size(); i++)
        {
            if (getVertex(adjacent_list[i]).degree() != 1)
                continue;
            if (ears[0] == -1)
                ears[0] = adjacent_list[i];
            else
                ears[1] = adjacent_list[i];
        }
    }

    // Calculate energy
    if (parity == 0)
        _orderByEnergy(positions);

    // Assign coordinates
    if (two_ears)
    {
        for (int i = 0; i < adjacent_list.size(); i++)
        {
            int j = adjacent_list[i];
            if (getVertex(j).degree() != 1)
            {
                _layout_vertices[j].type = ELEMENT_BOUNDARY;
                _layout_edges[vert.neiEdge(vert.findNeiVertex(j))].type = ELEMENT_BOUNDARY;
                _layout_vertices[j].pos = positions[0];
                break;
            }
        }

        _attachEars(vert_idx, vert.neiVertex(drawn_idx), ears, positions[0]);

        return;
    }

    int j = 0;
    while (adjacent_list.size() > 0)
    {
        int i = adjacent_list.pop();

        _layout_vertices[i].pos = positions[j];
        _layout_vertices[i].type = ELEMENT_BOUNDARY;
        _layout_edges[vert.neiEdge(vert.findNeiVertex(i))].type = ELEMENT_BOUNDARY;
        j++;
    }
}

bool MoleculeLayoutGraphSimple::_drawEdgesWithoutIntersection(const Cycle& cycle, Array<int>& cycle_vertex_types)
{
    bool is_attached = true;
    Vec2f p;

    QS_DEF(Array<int>, border1v);
    QS_DEF(Array<int>, border1e);
    QS_DEF(Array<int>, border2v);
    QS_DEF(Array<int>, border2e);

    for (int i = 0; i < cycle.vertexCount(); i++)
    {
        if (_layout_edges[cycle.getEdge(i)].type == ELEMENT_NOT_DRAWN)
        {
            for (int j = edgeBegin(); j < edgeEnd(); j = edgeNext(j))
                if (_layout_edges[j].type > ELEMENT_NOT_DRAWN)
                    if ((_calcIntersection(i, j) % 10) != 1)
                    {
                        is_attached = false;
                        break;
                    }

            if (!is_attached)
                continue;

            if (cycle_vertex_types[i] == ELEMENT_INTERNAL || cycle_vertex_types[(i + 1) % cycle.vertexCount()] == ELEMENT_INTERNAL)
            {
                _layout_edges[cycle.getEdge(i)].type = ELEMENT_INTERNAL;
            }
            else
            { // Both vertices are boundary.
                // Check if edge is boundary by its center
                p.lineCombin2(_layout_vertices[cycle.getVertex(i)].pos, 0.9f, _layout_vertices[cycle.getVertexC(i + 1)].pos, 0.1f);

                if (_isPointOutside(p))
                {
                    _splitBorder(cycle.getVertex(i), cycle.getVertexC(i + 1), border1v, border1e, border2v, border2e);

                    _layout_edges[cycle.getEdge(i)].type = ELEMENT_BOUNDARY;

                    // Ignore border1 and check if vertices are inside new bound
                    // if no then restore Border1 and ignore Border2
                    for (int j = 0; j < border1e.size(); j++)
                    {
                        if (j > 0)
                            _layout_vertices[border1v[j]].type = ELEMENT_IGNORE;
                        _layout_edges[border1e[j]].type = ELEMENT_IGNORE;
                    }

                    p.lineCombin2(_layout_vertices[border1v[1]].pos, 0.9f, _layout_vertices[border1v[2]].pos, 0.1f);

                    if (!_isPointOutside(p))
                    {
                        for (int j = 0; j < border1e.size(); j++)
                        {
                            if (j > 0)
                                _layout_vertices[border1v[j]].type = ELEMENT_INTERNAL;
                            _layout_edges[border1e[j]].type = ELEMENT_INTERNAL;
                        }
                    }
                    else
                    {
                        for (int j = 0; j < border1e.size(); j++)
                        {
                            if (j > 0)
                                _layout_vertices[border1v[j]].type = ELEMENT_BOUNDARY;
                            _layout_edges[border1e[j]].type = ELEMENT_BOUNDARY;
                        }
                        for (int j = 0; j < border2e.size(); j++)
                        {
                            if (j > 0)
                                _layout_vertices[border2v[j]].type = ELEMENT_INTERNAL;
                            _layout_edges[border2e[j]].type = ELEMENT_INTERNAL;
                        }
                    }
                }
                else
                    _layout_edges[cycle.getEdge(i)].type = ELEMENT_INTERNAL;
            }
        }
    }

    return is_attached;
}

bool MoleculeLayoutGraph::_checkBadTryBorderIntersection(Array<int>& chain_ext, MoleculeLayoutGraph& next_bc, Array<int>& mapping)
{
    for (int i = 0; i < chain_ext.size() - 1; i++)
        for (int j = next_bc.edgeBegin(); j < next_bc.edgeEnd(); j = next_bc.edgeNext(j))
        {
            if (_layout_edges[next_bc._layout_edges[j].ext_idx].type == ELEMENT_BOUNDARY)
            {
                const Vertex& vert = next_bc.getVertex(mapping[chain_ext[i]]);
                int edge1_idx = vert.neiEdge(vert.findNeiVertex(mapping[chain_ext[i + 1]]));
                int intersect = next_bc._calcIntersection(edge1_idx, j);

                const Edge& edge1 = next_bc.getEdge(edge1_idx);
                const Edge& edge2 = next_bc.getEdge(j);

                // Check if edges intersect
                if ((intersect % 10) != 1 ||
                    (intersect == 21 && edge1.beg != edge2.beg && edge1.beg != edge2.end && edge1.end != edge2.beg && edge1.end != edge2.end))
                {
                    return false;
                }
            }
        }

    return true;
}

bool MoleculeLayoutGraph::_checkBadTryChainOutside(Array<int>& chain_ext, MoleculeLayoutGraph& next_bc, Array<int>& mapping)
{
    // Check chain_ext is outside bound
    for (int i = 1; i < chain_ext.size() - 1; i++)
    {
        if (!_isPointOutside(next_bc._layout_vertices[mapping[chain_ext[i]]].pos))
            return false;
    }
    return true;
}

using index_angles = std::vector<std::pair<int, float>>;

static float calc_median_diff(float optimal_angle, const index_angles& angles, const index_angles& splits)
{
    int count = 0;
    float sum = 0.0;
    for (size_t i = 0; i < angles.size(); i++)
    {
        if (i < splits.size())
        {
            int angle_count = splits[i].first + 1; // angle count = new edge count +1
            sum += angle_count * std::fabs(optimal_angle - splits[i].second);
            count += angle_count;
        }
        else
        {
            sum += std::fabs(optimal_angle - angles[i].second);
            count++;
        }
    }
    if (count > 0)
        return sum / count;
    return std::numeric_limits<float>::max();
}

static float find_edge_splits(float optimal_angle, const index_angles& angles, size_t index, int edges_to_insert, index_angles& splits)
{
    int angle_index = angles[index].first;
    float angle = angles[index].second;
    float best_metric = std::numeric_limits<float>::max();
    size_t next_index = index + 1;

    if (next_index >= angles.size())
    { // last angle - insert all desired edges
        splits.emplace_back(edges_to_insert, angle / (edges_to_insert + 1));
        best_metric = calc_median_diff(optimal_angle, angles, splits);
    }
    else
    { // not last angle. Try to find best split.
        int edges_to_try = 0;

        if (angle > optimal_angle) // Wll not divide angle < optimal
            edges_to_try = static_cast<int>(angle / optimal_angle);

        if (edges_to_try > edges_to_insert)
            edges_to_try = edges_to_insert;

        while (edges_to_try >= 0)
        {
            index_angles new_splits;
            new_splits.assign(splits.cbegin(), splits.cbegin() + index);
            new_splits.emplace_back(edges_to_try, angle / (edges_to_try + 1));
            float new_metric = find_edge_splits(optimal_angle, angles, next_index, edges_to_insert - edges_to_try, new_splits);
            if (new_metric < best_metric)
            {
                best_metric = new_metric;
                splits = new_splits;
            }
            edges_to_try--;
        }
    }
    return best_metric;
}

void MoleculeLayoutGraph::_calculatePositionsManyNotDrawn(int vert_idx, Array<int>& to_draw, Array<Vec2f>& positions)
{
    int to_draw_count = to_draw.size();
    positions.clear_resize(to_draw_count);

    const Vertex& vert = getVertex(vert_idx);
    Vec2f& pstart = getPos(vert_idx);

    index_angles edges_angles;

    // find edge angles
    for (int i = vert.neiBegin(); i < vert.neiEnd(); i = vert.neiNext(i))
    {
        int neiVert = vert.neiVertex(i);
        if (to_draw.find(neiVert) >= 0)
            continue;

        Vec2f edge = getPos(neiVert) - pstart;
        if (edge.x < EPSILON && edge.y < EPSILON)
            edge.y += 0.001f; // if edge too small - add y size
        edges_angles.emplace_back(neiVert, edge.tiltAngle2());
    }
    std::stable_sort(edges_angles.begin(), edges_angles.end(), [](std::pair<int, float> a, std::pair<int, float> b) { return a.second < b.second; });

    index_angles angles; // angles between edges
    for (size_t i = 0; i + 1 < edges_angles.size(); i++)
    {
        angles.emplace_back(edges_angles[i].first, edges_angles[i + 1].second - edges_angles[i].second);
    }
    angles.emplace_back(edges_angles.back().first, _2FLOAT(edges_angles[0].second + 2.0 * M_PI - edges_angles.back().second));

    auto total_edges = to_draw_count + angles.size();
    float optimal_angle = _2FLOAT(2.0 * M_PI / total_edges); // optimal angle = 2Pi/total_edge_count

    index_angles splits;
    std::ignore = find_edge_splits(optimal_angle, angles, 0, to_draw_count, splits);

    // place new edge between drawn
    int calculated_positions = 0;
    for (size_t i = 0; i < splits.size(); i++)
    {
        if (splits[i].first > 0) // number of new edges in this agle
        {
            Vec2f pvert = getPos(angles[i].first); // splits index correspond to anges index
            _calculatePos(splits[i].second, pstart, pvert, positions[calculated_positions++]);
            for (int j = 1; j < splits[i].first; j++)
            {
                _calculatePos(splits[i].second, pstart, positions[calculated_positions - 1], positions[calculated_positions]);
                calculated_positions++;
            }
        }
    }
    return;
}

void MoleculeLayoutGraph::_calculatePositionsOneNotDrawn(Array<Vec2f>& positions, int n_pos, int vert_idx, int not_drawn_idx)
{
    positions.clear_resize(n_pos);

    const Vertex& vert = getVertex(vert_idx);
    Vec2f v1, v2, p0;
    float phi;

    QS_DEF(Array<float>, angles); // polar angles of drawn edges
    QS_DEF(Array<int>, edges);    // edge indices in CCW order

    angles.clear();
    edges.clear();

    // find angles
    for (int i = vert.neiBegin(); i < vert.neiEnd(); i = vert.neiNext(i))
    {
        if (i == not_drawn_idx)
            continue;

        edges.push(i);
        Vec2f& v1 = getPos(vert.neiVertex(i));
        Vec2f& v2 = getPos(vert_idx);
        p0.diff(v1, v2);
        if (p0.length() < EPSILON)
        {
            // Perturbate coordinate
            v1.y += 0.001f;
            p0.diff(v1, v2);
        }
        angles.push(p0.tiltAngle2());
    }

    int size = angles.size();
    for (int i = 0; i < size; i++)
        for (int j = i + 1; j < size; j++)
            if (angles[i] > angles[j])
            {
                angles.swap(i, j);
                edges.swap(i, j);
            }

    // place new edge between drawn
    v1 = getPos(vert_idx);

    for (int i = 0; i < n_pos - 1; i++)
    {
        v2 = getPos(vert.neiVertex(edges[i]));
        phi = (angles[i + 1] - angles[i]) / 2;
        _calculatePos(phi, v1, v2, positions[i]);
    }

    v2 = getPos(vert.neiVertex(edges.top()));
    phi = _2FLOAT((2. * M_PI + angles[0] - angles.top()) / 2.);
    _calculatePos(phi, v1, v2, positions.top());
}

void MoleculeLayoutGraph::_calculatePositionsSingleDrawn(int vert_idx, Array<int>& adjacent_list, int& n_pos, int drawn_idx, bool& two_ears,
                                                         Array<Vec2f>& positions, int& parity)
{
    // Split 2pi to n_pos+1 parts
    // Place vertices like regular polygon
    // Drawn is first vertex, other in CCW order

    Vec2f v1, v2;
    float phi;
    const Vertex& vert = getVertex(vert_idx);

    if (adjacent_list.size() > 1)
    {
        if (n_pos == 1 && adjacent_list.size() == 3) // to avoid four bonds to be drawn like cross
        {
            n_pos = 5;
            int n_matter = 0, n_matter_2 = 0, n_single = 0, n_double_bond = 0;
            const Vertex& drawn_vert = getVertex(vert.neiVertex(drawn_idx));

            if (drawn_vert.degree() > 2)
                n_matter_2++;
            else if (drawn_vert.degree() == 1)
                n_single++;

            if (_molecule != 0)
            {
                int type = _molecule->getBondOrder(_molecule_edge_mapping[_layout_edges[vert.neiEdge(drawn_idx)].ext_idx]);

                if (type == BOND_DOUBLE)
                    n_double_bond++;
            }

            for (int i = 0; i < adjacent_list.size(); i++)
            {
                int adj_degree = getVertex(adjacent_list[i]).degree();

                if (adj_degree == 1)
                    n_single++;
                else
                    n_matter++;

                if (adj_degree > 2)
                    n_matter_2++;

                if (_molecule != 0)
                {
                    int nei_idx = vert.findNeiVertex(adjacent_list[i]);
                    int type = _molecule->getBondOrder(_molecule_edge_mapping[_layout_edges[vert.neiEdge(nei_idx)].ext_idx]);

                    if (type == BOND_DOUBLE)
                        n_double_bond++;
                }
            }

            if (n_matter == 1 && n_double_bond < 2) // draw ears
            {
                two_ears = true;
                n_pos = 2;
            }
            else if (n_matter_2 > 1 || n_double_bond > 1 || n_single == 4) // cross-like case
                n_pos = 3;
        }
        else
            n_pos = adjacent_list.size();
    }
    else
    {
        int type1 = 0, type2 = 0;

        if (_molecule != 0)
        {
            int first_nei = vert.neiBegin();
            type1 = _molecule->getBondOrder(_molecule_edge_mapping[_layout_edges[vert.neiEdge(first_nei)].ext_idx]);
            type2 = _molecule->getBondOrder(_molecule_edge_mapping[_layout_edges[vert.neiEdge(vert.neiNext(first_nei))].ext_idx]);
        }
        if (n_pos != 1 || (!(type1 == BOND_TRIPLE || type2 == BOND_TRIPLE) && !(type1 == BOND_DOUBLE && type2 == BOND_DOUBLE)))
            n_pos = 2;
    }

    positions.clear_resize(n_pos);

    phi = _2FLOAT(2. * M_PI / (n_pos + 1));
    v1 = getPos(vert_idx);
    v2 = getPos(vert.neiVertex(drawn_idx));

    _calculatePos(phi, v1, v2, positions[0]);

    for (int i = 1; i < n_pos; i++)
    {
        v2 = positions[i - 1];
        _calculatePos(phi, v1, v2, positions[i]);
    }

    // Check cis/trans
    if (_molecule != 0 && n_pos == 2)
    {
        parity = _molecule->cis_trans.getParity(_molecule_edge_mapping[_layout_edges[vert.neiEdge(drawn_idx)].ext_idx]);

        if (parity != 0)
        {
            int substituents[4];
            _molecule->getSubstituents_All(_molecule_edge_mapping[_layout_edges[vert.neiEdge(drawn_idx)].ext_idx], substituents);

            int to_draw_substituent = -1;

            for (int i = 0; i < 4; i++)
                if (substituents[i] == _layout_vertices[adjacent_list.top()].ext_idx)
                {
                    to_draw_substituent = i;
                    break;
                }

            const Vertex& drawn_vert = getVertex(vert.neiVertex(drawn_idx));

            int drawn_substituent = -1;
            int drawn_substituent_idx = -1;

            for (int i = drawn_vert.neiBegin(); i < drawn_vert.neiEnd(); i = drawn_vert.neiNext(i))
                if (drawn_vert.neiVertex(i) != vert_idx) // must be drawn
                {
                    for (int j = 0; j < 4; j++)
                        if (substituents[j] == _layout_vertices[drawn_vert.neiVertex(i)].ext_idx)
                        {
                            drawn_substituent_idx = drawn_vert.neiVertex(i);
                            drawn_substituent = j;
                            break;
                        }
                    break;
                }

            bool same_side = false;

            if ((parity == MoleculeCisTrans::CIS) == (abs(to_draw_substituent - drawn_substituent) == 2))
                same_side = true;
            int side_sign = 0;
            if ((drawn_substituent_idx != -1) && (drawn_substituent != -1))
            {
                side_sign = MoleculeCisTrans::sameside(Vec3f(_layout_vertices[vert.neiVertex(drawn_idx)].pos), Vec3f(_layout_vertices[vert_idx].pos),
                                                       Vec3f(_layout_vertices[drawn_substituent_idx].pos), Vec3f(positions[0]));
            }

            if (same_side)
            {
                if (side_sign == -1)
                    positions.swap(0, 1);
            }
            else if (side_sign == 1)
                positions.swap(0, 1);
        }
    }
}

void MoleculeLayoutGraph::_orderByEnergy(Array<Vec2f>& positions)
{
    QS_DEF(Array<float>, energies);
    QS_DEF(Array<float>, norm_a);
    float norm = 0.0;
    float r = 0.f;
    Vec2f p0;

    int n_pos = positions.size();

    energies.clear_resize(n_pos);
    norm_a.clear_resize(vertexEnd());
    energies.zerofill();

    for (int i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
        if (getVertexType(i) != ELEMENT_NOT_DRAWN && getVertexType(i) != ELEMENT_IGNORE)
        {
            norm_a[i] = _2FLOAT(_layout_vertices[i].morgan_code);
            norm += norm_a[i] * norm_a[i];
        }

    norm = sqrt(norm);

    for (int i = 0; i < n_pos; i++)
    {
        for (int j = vertexBegin(); j < vertexEnd(); j = vertexNext(j))
            if (getVertexType(j) != ELEMENT_NOT_DRAWN && getVertexType(j) != ELEMENT_IGNORE)
            {
                p0.diff(positions[i], getPos(j));
                r = p0.lengthSqr();

                if (r < EPSILON)
                {
                    energies[i] = 1E+20f;
                    continue;
                }

                energies[i] += ((norm_a[j] / norm + 0.5f) / r);
            }
    }

    // Sort by energies
    for (int i = 0; i < n_pos; i++)
        for (int j = i + 1; j < n_pos; j++)
            if (energies[j] < energies[i])
            {
                energies.swap(i, j);
                positions.swap(i, j);
            }
}
