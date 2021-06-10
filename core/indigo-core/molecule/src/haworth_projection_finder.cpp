#include "molecule/haworth_projection_finder.h"

#include <algorithm>
#include <math.h>

#include "base_cpp/exception.h"
#include "math/algebra.h"
#include "molecule/base_molecule.h"
#include "molecule/elements.h"

using namespace indigo;

CP_DEF(HaworthProjectionFinder);

const float COS10_THRESHOLD = 0.015f;

HaworthProjectionFinder::HaworthProjectionFinder(BaseMolecule& mol)
    : _mol(mol), CP_INIT, TL_CP_GET(_atoms_mask), TL_CP_GET(_bonds_mask), TL_CP_GET(_bold_bonds_mask)
{
    _atoms_mask.clear();
    _atoms_mask.resize(_mol.vertexEnd(), false);
    _bonds_mask.clear();
    _bonds_mask.resize(_mol.edgeEnd(), false);
    _bold_bonds_mask.clear();
    _bold_bonds_mask.resize(_mol.edgeEnd(), false);
}

void HaworthProjectionFinder::findAndAddStereocenters()
{
    _find(true);
}

void HaworthProjectionFinder::find()
{
    _find(false);
}

const ArrayBool& HaworthProjectionFinder::getAtomsMask()
{
    return _atoms_mask;
}

const ArrayBool& HaworthProjectionFinder::getBondsMask()
{
    return _bonds_mask;
}

bool HaworthProjectionFinder::isBoldBond(int e_idx)
{
    return _bold_bonds_mask[e_idx];
}

void HaworthProjectionFinder::_find(bool add_stereo)
{
    if (BaseMolecule::hasCoord(_mol))
    {
        QS_DEF(ArrayInt, vertices);
        QS_DEF(ArrayInt, edges);

        int sssr_cnt = _mol.sssrCount();
        for (int i = 0; i < sssr_cnt; i++)
        {
            // Make an array with vertices and edges being corresponded to each other
            //         const List<int> &v_list = _mol.sssrVertices(i);
            const List<int>& e_list = _mol.sssrEdges(i);

            edges.clear();
            for (int j = e_list.begin(); j != e_list.end(); j = e_list.next(j))
                edges.push(e_list[j]);

            int vbegin = _mol.getEdge(edges[0]).beg;
            const Edge& e2 = _mol.getEdge(edges[1]);
            if (vbegin == e2.beg || vbegin == e2.end)
                vbegin = _mol.getEdge(edges[0]).end;

            vertices.clear();
            vertices.push(vbegin);

            int v = vbegin;
            for (int j = 0; j < edges.size() - 1; j++)
            {
                int v2 = _mol.getEdgeEnd(v, edges[j]);
                if (v2 == -1)
                    throw Exception("Internal error. Edges are not adjust");

                vertices.push(v2);
                v = v2;
            }

            _processRing(add_stereo, vertices, edges);
        }
    }
}

bool HaworthProjectionFinder::_processRing(bool add_stereo, const ArrayInt& vertices, const ArrayInt& edges)
{
    // We detect rings of size from 3 and 6:
    //       -----       ----       /  \      ----            ------
    //     /      \     /    \     \    /    |    |     /\     \  /
    //     \      /      \  /       ----     |    |    /  \     \/
    //      ======                            ====     ====
    if (vertices.size() < 3 || vertices.size() > 6)
        return false;

    // Check that
    //    each vertex has only 2 bonds in ring,
    //    each angle is not small to form normal ring
    //    polygone is convex
    int sign = 0;
    int vertical_count = 0, corner_count = 0;
    bool has_bold = false;
    for (int j = 0; j < vertices.size(); j++)
    {
        int prev_j = (j + vertices.size() - 1) % vertices.size();
        int vi = vertices[j];
        int e1i = edges[j];
        int e2i = edges[prev_j]; // Previous edge

        /* TODO: Uncomment after validation
        if (_mol.getBondOrder(e1i) != BOND_SINGLE)
           return false;
        */

        const Vertex& v = _mol.getVertex(vi);
        if (v.degree() > 4)
            return false;

        int ring_cnt = 0;
        int subs_cnt = 0, subs_bond_idx = -1;
        for (int nei = v.neiBegin(); nei != v.neiEnd(); nei = v.neiNext(nei))
        {
            int b = v.neiEdge(nei);
            if (_mol.getEdgeTopology(b) == TOPOLOGY_RING)
                ring_cnt++;
            else
            {
                subs_cnt++;
                subs_bond_idx = b;
            }
        }
        if (ring_cnt != 2)
            return false;

        float c = _getAngleCos(vi, e1i, e2i);
        if (c > 1 - COS10_THRESHOLD)
            return false; // Angle is too small

        float s = _getAngleSin(vi, e1i, e2i);
        if (sign == 0)
            sign = __sign(s);
        else if (sign != __sign(s))
            return false; // Rotation direction is different => non-convex

        bool is_corner = _isCornerVertex(vi, e1i, e2i);
        if (!is_corner)
        {
            // Substituents should be vertical
            for (int nei = v.neiBegin(); nei != v.neiEnd(); nei = v.neiNext(nei))
            {
                int b = v.neiEdge(nei);
                if (_mol.getEdgeTopology(b) == TOPOLOGY_RING)
                    continue;

                if (!_isVerticalEdge(b, COS10_THRESHOLD))
                    return false; // Not vertical
            }
        }
        else
        {
            // Corner vertex
            corner_count++;
        }

        // Count number of vertical bonds
        for (int nei = v.neiBegin(); nei != v.neiEnd(); nei = v.neiNext(nei))
        {
            int b = v.neiEdge(nei);
            if (_mol.getEdgeTopology(b) == TOPOLOGY_RING)
                continue;

            if (_isVerticalEdge(b, COS10_THRESHOLD))
                vertical_count++;
        }

        // Check if bond is a angle bisector
        if (subs_cnt == 1)
        {
            float a1 = _getAngleCos(vi, subs_bond_idx, e1i);
            float a2 = _getAngleCos(vi, subs_bond_idx, e2i);
            if (fabs(a1 - a2) < 1e-3)
            {
                // Check if angle bisector is allowed
                if (is_corner)
                {
                    // Only horizontal line is allowed
                    if (!_isHorizontalEdge(subs_bond_idx, COS10_THRESHOLD))
                        return false;
                }
                else
                {
                    // Only vertical line is allowed
                    if (!_isVerticalEdge(subs_bond_idx, COS10_THRESHOLD))
                        return false;
                }
            }
        }

        // Substituents should be opposite to each other
        int sub_sign = 0;

        for (int nei = v.neiBegin(); nei != v.neiEnd(); nei = v.neiNext(nei))
        {
            int b = v.neiEdge(nei);
            if (_mol.getEdgeTopology(b) == TOPOLOGY_RING)
                continue;

            float c2 = _getAngleCos(vi, b, 0.0f, 1.0f);
            if (fabs(c2) > COS10_THRESHOLD)
            {
                // Count only non-horizontal bonds
                if (sub_sign == 0)
                    sub_sign = __sign(c2);
                else if (sub_sign == __sign(c2))
                    return false; // Substituents are in the same direction
            }
        }

        // Check bold bond and bold corner
        int next_j = (j + 1) % vertices.size();
        int vpi = vertices[prev_j];
        int vni = vertices[next_j];
        /*
        // Check corner
        //       ----
        //      /    \
        //       \  /
        //
        */
        if (_mol.getBondDirection2(vpi, vi) == BOND_UP && _mol.getBondDirection2(vni, vi) == BOND_UP)
            has_bold = true;
        else
        {
            /* Check bold bond
            //       -----
            //     /      \
            //     \      /
            //      ======
            */
            int next2_j = (j + 2) % vertices.size();
            int vn2i = vertices[next2_j];
            if (_mol.getBondDirection2(vpi, vi) == BOND_UP && _mol.getBondDirection2(vn2i, vni) == BOND_UP)
                has_bold = true;
        }
    }

    if (has_bold)
    {
        if (vertical_count <= 1)
            // At least two vertical bonds are required
            return false;
    }
    else
    {
        if (vertical_count <= 2)
            // At least three vertical bonds are required
            return false;
    }

    if (corner_count != 2)
        // There should be one explicit corner at both sides
        return false;

    // Ring seems to have Haworth projection
    _markRingBonds(vertices, edges);

    if (add_stereo)
        _addRingStereocenters(vertices, edges);

    return true;
}

void HaworthProjectionFinder::_markRingBonds(const ArrayInt& vertices, const ArrayInt& edges)
{
    // Mark bonds
    for (int j = 0; j < vertices.size(); j++)
    {
        int vi = vertices[j];
        const Vertex& v = _mol.getVertex(vi);
        // Mark ring atoms
        _atoms_mask[vi] = true;
        // Mark all bonds to these atoms
        for (int nei = v.neiBegin(); nei != v.neiEnd(); nei = v.neiNext(nei))
        {
            int b = v.neiEdge(nei);
            _bonds_mask[b] = true;
        }

        // Check bold bond
        int prev_j = (j + vertices.size() - 1) % vertices.size();
        int next_j = (j + 1) % vertices.size();
        int next_j2 = (j + 2) % vertices.size();
        int vip = vertices[prev_j];
        int vin = vertices[next_j];
        int vin2 = vertices[next_j2];

        if (_mol.getBondDirection2(vip, vi) == BOND_UP && _mol.getBondDirection2(vin2, vin) == BOND_UP)
            _bold_bonds_mask[edges[j]] = 1;
    }
}

void HaworthProjectionFinder::_addRingStereocenters(const ArrayInt& vertices, const ArrayInt& edges)
{
    // Find left vertex
    int j_left = -1;
    float x_left = 1e20f;
    for (int j = 0; j < vertices.size(); j++)
    {
        float x = _mol.getAtomXyz(vertices[j]).x;
        if (x < x_left)
        {
            x_left = x;
            j_left = j;
        }
    }

    // Check direction and make it from top to bottom
    int left_next = (j_left + 1) % vertices.size();
    int left_prev = (j_left + vertices.size() - 1) % vertices.size();
    float yn = _mol.getAtomXyz(vertices[left_next]).y;
    float yp = _mol.getAtomXyz(vertices[left_prev]).y;

    int parity = __sign(yn - yp);

    for (int j = 0; j < vertices.size(); j++)
    {
        int vi = vertices[j];

        if (_mol.getAtomNumber(vi) != ELEM_C)
            continue;

        int v_next = (j + 1) % vertices.size();
        int v_prev = (j + vertices.size() - 1) % vertices.size();
        int vi_next = vertices[v_next];
        int vi_prev = vertices[v_prev];

        float yc = _mol.getAtomXyz(vi).y;

        int vi_top = -1, vi_bottom = -1;
        const Vertex& v = _mol.getVertex(vi);
        if (v.degree() == 2)
            continue;

        bool skip = false;
        for (int nei = v.neiBegin(); nei != v.neiEnd(); nei = v.neiNext(nei))
        {
            int vn = v.neiVertex(nei);
            if (vn == vi_next || vn == vi_prev)
                continue;

            int b = v.neiEdge(nei);

            if (_isHorizontalEdge(b, COS10_THRESHOLD))
            {
                skip = true;
                break;
            }

            float yn = _mol.getAtomXyz(vn).y;
            if (yn > yc)
                vi_top = vn;
            else
                vi_bottom = vn;
        }

        if (skip)
            // Skip this vertex because some edge is horizontal
            continue;

        // Add stereocenter
        int pyramid[4];
        pyramid[0] = vi_prev;
        pyramid[1] = vi_top;
        pyramid[2] = vi_next;
        pyramid[3] = vi_bottom;
        if (parity == -1)
            std::swap(pyramid[2], pyramid[3]);
        if (vi_top == -1 || vi_bottom == -1)
        {
            int cnt = 0;
            while (pyramid[3] != -1)
            {
                MoleculeStereocenters::rotatePyramid(pyramid);
                cnt++;
            }
            if (cnt % 2 == 1)
                std::swap(pyramid[0], pyramid[1]);
        }

        _mol.stereocenters.add(vi, MoleculeStereocenters::ATOM_ABS, 1, pyramid);
    }
}

bool HaworthProjectionFinder::_isCornerVertex(int v, int e1, int e2)
{
    int v1 = _mol.getEdgeEnd(v, e1);
    int v2 = _mol.getEdgeEnd(v, e2);
    Vec3f pv = _mol.getAtomXyz(v);
    Vec3f pv1 = _mol.getAtomXyz(v1);
    Vec3f pv2 = _mol.getAtomXyz(v2);

    Vec2f d1(pv1.x - pv.x, pv1.y - pv.y);
    Vec2f d2(pv2.x - pv.x, pv2.y - pv.y);

    return __sign(d1.x * d2.x) == 1;
}

bool HaworthProjectionFinder::_isHorizontalEdge(int e, float cos_threshold)
{
    int v = _mol.getEdge(e).beg;
    return fabs(_getAngleCos(v, e, 1.0f, 0.0f)) > 1 - cos_threshold;
}

bool HaworthProjectionFinder::_isVerticalEdge(int e, float cos_threshold)
{
    int v = _mol.getEdge(e).beg;
    return fabs(_getAngleCos(v, e, 0.0f, 1.0f)) > 1 - cos_threshold;
}

float HaworthProjectionFinder::_getAngleCos(int v, int e, float dx, float dy)
{
    int v1 = _mol.getEdgeEnd(v, e);
    Vec3f pv = _mol.getAtomXyz(v);
    Vec3f pv2 = _mol.getAtomXyz(v1);

    Vec2f d1(dx, dy);
    Vec2f d2(pv2.x - pv.x, pv2.y - pv.y);

    return Vec2f::dot(d1, d2) / d1.length() / d2.length();
}

float HaworthProjectionFinder::_getAngleCos(int v, int e1, int e2)
{
    int v2 = _mol.getEdgeEnd(v, e2);
    Vec3f pv = _mol.getAtomXyz(v);
    Vec3f pv2 = _mol.getAtomXyz(v2);
    return _getAngleCos(v, e1, pv2.x - pv.x, pv2.y - pv.y);
}

float HaworthProjectionFinder::_getAngleSin(int v, int e1, int e2)
{
    int v1 = _mol.getEdgeEnd(v, e1);
    int v2 = _mol.getEdgeEnd(v, e2);
    Vec3f pv = _mol.getAtomXyz(v);
    Vec3f pv1 = _mol.getAtomXyz(v1);
    Vec3f pv2 = _mol.getAtomXyz(v2);

    Vec2f d1(pv1.x - pv.x, pv1.y - pv.y);
    Vec2f d2(pv2.x - pv.x, pv2.y - pv.y);

    return Vec2f::cross(d1, d2) / d1.length() / d2.length();
}
