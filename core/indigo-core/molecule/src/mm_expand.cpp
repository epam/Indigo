#include "molecule/mm_expand.h"

#include "molecule/ket_objects.h"
#include <algorithm>
#include <cmath>
#include <graph/graph.h>
#include <map>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

float const DEFAULT_DIMENSION = 3.0f;

namespace indigo
{

    struct NeighborSpec
    {
        std::string monomerId;
        std::string attachmentPointId; // neighbors IDs
        float angle = 0.0f;            // neighbor angle in absolute coordinates
    };

    // Set the expanded monomers and calculate the dimensions for each monomer as R1-R2 distance
    void getDimensions(KetDocument& mol, std::vector<float>& dimensions)
    {
        bool has_selection = false;
        for (const auto& monomerId : mol.monomersIds())
        {
            if (mol.getMonomerById(monomerId)->isBoolPropTrue("selected"))
            {
                has_selection = true;
                break;
            }
        }
        for (const auto& monomerId : mol.monomersIds())
        {
            int id = std::stoi(monomerId);
            auto& monPtr = mol.getMonomerById(monomerId);

            // Skip if the monomer is not a monomer really
            if (monPtr->monomerType() != KetBaseMonomer::MonomerType::Monomer)
            {
                dimensions.push_back(DEFAULT_DIMENSION);
                continue;
            }
            auto& mon = static_cast<KetMonomer&>(*monPtr);
            if (has_selection && !mon.isBoolPropTrue("selected"))
            {
                dimensions.push_back(DEFAULT_DIMENSION);
            }
            else
            {
                mon.setBoolProp("expanded", true);
                const auto& tmpl = mol.templates().at(mon.templateId());
                // collect all leaving-group atom indices
                std::vector<int> leaves;
                for (const auto& ap_pair : tmpl.attachmentPoints())
                {
                    auto lg = ap_pair.second.leavingGroup();
                    if (lg && !lg->empty())
                        leaves.push_back(lg->at(0));
                }
                float dim = 0.0f;
                if (leaves.size() >= 2)
                {
                    // take first two for R1 and R2
                    auto loc1 = tmpl.atoms()[leaves[0]]->location();
                    auto loc2 = tmpl.atoms()[leaves[1]]->location();
                    if (loc1.has_value() && loc2.has_value())
                    {
                        auto v1 = loc1.value(), v2 = loc2.value();
                        float dx = v2.x - v1.x;
                        float dy = v2.y - v1.y;
                        dim = std::sqrt(dx * dx + dy * dy);
                    }
                }
                else if (leaves.size() == 1)
                {
                    // single R-group: double distance to geometric center
                    auto loc = tmpl.atoms()[leaves[0]]->location();
                    if (loc.has_value())
                    {
                        Vec2f center{0, 0};
                        int count = 0;
                        for (const auto& atomPtr : tmpl.atoms())
                        {
                            auto aloc = atomPtr->location();
                            if (aloc.has_value())
                            {
                                center.x += aloc.value().x;
                                center.y += aloc.value().y;
                                ++count;
                            }
                        }
                        if (count > 0)
                        {
                            center.x /= count;
                            center.y /= count;
                            float dx = loc.value().x - center.x;
                            float dy = loc.value().y - center.y;
                            dim = 2.0f * std::sqrt(dx * dx + dy * dy);
                        }
                    }
                }
                dimensions.push_back(dim);
            }
        }
    }

    // Get neighbor map
    void getNeighbors(KetDocument& mol, std::unordered_map<std::string, std::vector<NeighborSpec>>& neighborMap)
    {
        // neighbor numbers
        for (const auto& conn : mol.connections())
        {
            auto ref1 = conn.ep1().getStringProp("monomerId");
            auto ref2 = conn.ep2().getStringProp("monomerId");
            auto p1 = ref1.find_first_of("0123456789");
            auto p2 = ref2.find_first_of("0123456789");
            std::string id1 = (p1 != std::string::npos) ? ref1.substr(p1) : std::string();
            std::string id2 = (p2 != std::string::npos) ? ref2.substr(p2) : std::string();
            // capture attachment point (R-group) for each endpoint
            std::string ap1 = conn.ep1().hasStringProp("attachmentPointId") ? conn.ep1().getStringProp("attachmentPointId") : std::string();
            std::string ap2 = conn.ep2().hasStringProp("attachmentPointId") ? conn.ep2().getStringProp("attachmentPointId") : std::string();
            neighborMap[id1].push_back(NeighborSpec{id2, ap1});
            neighborMap[id2].push_back(NeighborSpec{id1, ap2});
        }

        // neighbor angles
        for (const auto& monId : mol.monomersIds())
        {
            auto& monPtr = mol.getMonomerById(monId);
            auto& mon = static_cast<KetMonomer&>(*monPtr);
            Vec2f pos = mon.position().value_or(Vec2f{0, 0});
            auto it = neighborMap.find(monId);
            if (it == neighborMap.end())
                continue;
            auto& specs = it->second;
            for (auto& spec : specs)
            {
                auto& nPtr = mol.getMonomerById(spec.monomerId);
                auto& nm = static_cast<KetMonomer&>(*nPtr);
                Vec2f npos = nm.position().value_or(Vec2f{0, 0});
                Vec2f v{npos.x - pos.x, npos.y - pos.y};
                spec.angle = std::atan2(v.y, v.x);
            }
        }
    }

    // Workaround to get the graph of macromolecule and use it for ring detection as for small molecules
    void getGraph(KetDocument& mol, Graph& graph)
    {
        size_t n = mol.monomersIds().size();
        for (size_t i = 0; i < n; ++i)
            graph.addVertex();
        for (const auto& conn : mol.connections())
        {
            auto r1 = conn.ep1().getStringProp("monomerId");
            auto r2 = conn.ep2().getStringProp("monomerId");
            auto p1 = r1.find_first_of("0123456789");
            auto p2 = r2.find_first_of("0123456789");
            int a = (p1 != std::string::npos) ? std::stoi(r1.substr(p1)) : 0;
            int b = (p2 != std::string::npos) ? std::stoi(r2.substr(p2)) : 0;
            graph.addEdge(a, b);
        }
    }

    // Place ring monomers on circles
    void placeRingMonomers(KetDocument& mol, const std::vector<float>& dimensions, Graph& graph, std::unordered_map<std::string, Vec2f>& newPositions,
                           std::unordered_set<std::string>& placed)
    {
        int ringCount = graph.sssrCount();
        for (int ci = 0; ci < ringCount; ++ci)
        {
            auto& verts = graph.sssrVertices(ci);
            std::vector<int> cycle;
            for (int vi = verts.begin(); vi != verts.end(); vi = verts.next(vi))
                cycle.push_back(verts[vi]);
            auto cycleSize = cycle.size();
            if (cycleSize < 2)
                continue;
            float phi = 2.0f * static_cast<float>(M_PI) / cycleSize;
            float R = 0;
            for (size_t i = 0; i < cycleSize; ++i)
            {
                int u = cycle[i], v = cycle[(i + 1) % cycleSize];
                float d = (dimensions[u] + dimensions[v]) * 0.5f;
                float r = d / (2 * std::sin(phi / 2));
                R = std::max(R, r);
            }
            Vec2f center{0, 0};
            for (auto idx : cycle)
            {
                auto& m = static_cast<KetMonomer&>(*mol.getMonomerById(std::to_string(idx)));
                center.x += m.position().value().x;
                center.y += m.position().value().y;
            }
            center.x /= cycleSize;
            center.y /= cycleSize;
            for (size_t i = 0; i < cycleSize; ++i)
            {
                float ang = i * phi;
                Vec2f pos{center.x + R * std::cos(ang), center.y + R * std::sin(ang)};
                auto id = std::to_string(cycle[i]);
                newPositions[id] = pos;
                placed.insert(id);
            }
        }
    }

    // BFS (Breadth First Search) for a Graph propagate positions for non-ring monomers
    void bfsPropagate(KetDocument& mol, const std::vector<float>& dimensions, const std::unordered_map<std::string, std::vector<NeighborSpec>>& adjacency,
                      std::unordered_map<std::string, Vec2f>& newPos, std::unordered_set<std::string>& placed)
    {
        std::queue<std::string> q;
        for (auto& id : placed)
            q.push(id);
        if (placed.empty())
        {
            auto start = mol.monomersIds().front();
            auto& m = static_cast<KetMonomer&>(*mol.getMonomerById(start));
            Vec2f p = m.position().value_or(Vec2f{0, 0});
            newPos[start] = p;
            placed.insert(start);
            q.push(start);
        }
        while (!q.empty())
        {
            auto cur = q.front();
            q.pop();
            Vec2f cp = newPos[cur];
            int ui = std::stoi(cur);
            float du = dimensions[ui];
            auto it = adjacency.find(cur);
            if (it == adjacency.end())
                continue;
            for (size_t i = 0; i < it->second.size(); ++i)
            {
                const auto& spec = it->second[i];
                const auto& nid = spec.monomerId;
                if (placed.count(nid))
                    continue;
                int vi = std::stoi(nid);
                float dv = dimensions[vi];
                float dist = (du + dv) * 0.5f;
                float ang = spec.angle; // use precomputed angle
                Vec2f pos{cp.x + dist * std::cos(ang), cp.y + dist * std::sin(ang)};
                newPos[nid] = pos;
                placed.insert(nid);
                q.push(nid);
            }
        }
    }

    // Normalize angle to [-pi,pi]
    float normalizeAngle(float a)
    {
        const float PI2 = 2.0f * static_cast<float>(M_PI);
        a = fmod(a, PI2);
        if (a > static_cast<float>(M_PI))
            a -= PI2;
        else if (a < -static_cast<float>(M_PI))
            a += PI2;
        return a;
    }

    // Evaluate perpendicular distance of rotated leaving-group atoms to the given line (ignoring translation)
    float evalLeavingDistance(const Vec2f& pBase, const Vec2f& p0, const Vec2f& p1, const Vec2f& v1, const Vec2f& v2, float rot)
    {
        float c = std::cos(rot), s = std::sin(rot);
        Vec2f a0{c * v1.x - s * v1.y + pBase.x, s * v1.x + c * v1.y + pBase.y};
        Vec2f a1{c * v2.x - s * v2.y + pBase.x, s * v2.x + c * v2.y + pBase.y};
        Vec2f cen0{(pBase.x + p0.x) / 2, (pBase.y + p0.y) / 2};
        Vec2f cen1{(pBase.x + p1.x) / 2, (pBase.y + p1.y) / 2};
        Vec2f d0{a0.x - cen0.x, a0.y - cen0.y};
        Vec2f d1{cen1.x - a1.x, cen1.y - a1.y};

        return std::sqrt(d0.x * d0.x + d0.y * d0.y) + std::sqrt(d1.x * d1.x + d1.y * d1.y);
    }

    // Decide optimal rotation based on two key attachment points (R1/R2 or fallback pairs)
    static void computeRotations(KetDocument& mol, const std::unordered_map<std::string, std::vector<NeighborSpec>>& neighborMap,
                                 std::unordered_map<std::string, float>& newAngles)
    {
        for (const auto& id : mol.monomersIds())
        {
            auto& m = static_cast<KetMonomer&>(*mol.getMonomerById(id));
            Vec2f pBase = m.position().value_or(Vec2f{0, 0});
            // find neighbors for this monomer
            auto itAdj = neighborMap.find(id);
            if (itAdj == neighborMap.end() || itAdj->second.size() < 2)
            {
                // not enough neighbors to define direction
                newAngles[id] = 0.0f;
                continue;
            }
            auto& specs = itAdj->second;
            // map AP id -> index
            std::unordered_map<std::string, size_t> idxMap;
            for (size_t i = 0; i < specs.size(); ++i)
                idxMap[specs[i].attachmentPointId] = i;
            // pick two AP keys: prefer R1/R2, else R1/R3, else R2/R3, else first two
            std::string ap1, ap2;
            if (idxMap.count("R1") && idxMap.count("R2"))
            {
                ap1 = "R1";
                ap2 = "R2";
            }
            else if (idxMap.count("R1") && idxMap.count("R3"))
            {
                ap1 = "R1";
                ap2 = "R3";
            }
            else if (idxMap.count("R2") && idxMap.count("R3"))
            {
                ap1 = "R2";
                ap2 = "R3";
            }
            else
            {
                ap1 = specs[0].attachmentPointId;
                ap2 = specs[1].attachmentPointId;
            }
            // compute neighbor positions
            auto& spec0 = specs[idxMap[ap1]];
            auto& spec1 = specs[idxMap[ap2]];
            Vec2f n0 = mol.getMonomerById(spec0.monomerId)->position().value_or(pBase);
            Vec2f n1 = mol.getMonomerById(spec1.monomerId)->position().value_or(pBase);
            // get leaving-group axis in template coords
            const auto& tmpl = mol.templates().at(m.templateId());
            const auto& appts = tmpl.attachmentPoints();
            auto lg1 = appts.at(ap1).leavingGroup();
            auto lg2 = appts.at(ap2).leavingGroup();
            if (!lg1 || !lg2 || lg1->empty() || lg2->empty())
            {
                newAngles[id] = 0.0f;
                continue;
            }
            auto loc1_opt = tmpl.atoms()[lg1->at(0)]->location();
            auto loc2_opt = tmpl.atoms()[lg2->at(0)]->location();
            if (!loc1_opt.has_value() || !loc2_opt.has_value())
            {
                newAngles[id] = 0.0f;
                continue;
            }
            Vec2f loc1{loc1_opt->x, loc1_opt->y};
            Vec2f loc2{loc2_opt->x, loc2_opt->y};
            Vec2f vR{loc2.x - loc1.x, loc2.y - loc1.y};
            float angleR = std::atan2(vR.y, vR.x);
            Vec2f vN{n1.x - n0.x, n1.y - n0.y};
            float angleN = std::atan2(vN.y, vN.x);
            float r1 = normalizeAngle(angleN - angleR);
            float r2 = normalizeAngle(angleN - (angleR + static_cast<float>(M_PI)));
            float d1 = evalLeavingDistance(pBase, n0, n1, loc1, loc2, r1);
            float d2 = evalLeavingDistance(pBase, n0, n1, loc1, loc2, r2);
            newAngles[id] = (d1 < d2) ? r1 : r2;
        }
    }

    // Apply transformations to monomers: align leaving-group axis with neighbor line and shift to new positions
    void applyTransformations(KetDocument& mol, const std::unordered_map<std::string, Vec2f>& newPos, const std::unordered_map<std::string, float>& newAngles)
    {
        for (const auto& id : mol.monomersIds())
        {
            auto& m = static_cast<KetMonomer&>(*mol.getMonomerById(id));
            // determine rotation
            float rotation = 0.0f;
            auto it = newAngles.find(id);
            if (it != newAngles.end())
                rotation = it->second;
            // compute shift as difference from original position
            Vec2f newP = newPos.at(id);
            Vec2f origP = m.position().value_or(Vec2f{0, 0});
            Vec2f shift{newP.x - origP.x, newP.y - origP.y};
            m.setTransformation({rotation, shift});
        }
    }

    void indigoExpand(KetDocument& mol)
    {
        // fill all required for calculations
        std::vector<float> dimensions;                                          // R1-R2 distance
        std::unordered_map<std::string, std::vector<NeighborSpec>> neighborMap; // monomerId -> neighbors with R-group info
        Graph graph;                                                            // workaround to run sssr for macromolecule
        getDimensions(mol, dimensions);
        getNeighbors(mol, neighborMap);
        getGraph(mol, graph);

        // calculate new positions for ring monomers and non ring monomers
        std::unordered_map<std::string, Vec2f> newPositions;
        std::unordered_map<std::string, float> newAngles;
        std::unordered_set<std::string> placed;
        placeRingMonomers(mol, dimensions, graph, newPositions, placed);
        bfsPropagate(mol, dimensions, neighborMap, newPositions, placed);
        computeRotations(mol, neighborMap, newAngles);

        // transforming the output
        applyTransformations(mol, newPositions, newAngles);
    }

} // namespace indigo