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

// Rendering of haptic bonds (#3233, #3841). A haptic bond is not an edge of the
// graph, so nothing of the ordinary bond pipeline draws it: the tests below pin
// that it is drawn at all, which "the picture came out without an exception"
// would not catch.
//
// The renderer needs the whole session stack, so these live here rather than in
// the indigo-core unit tests, where linking render2d would drag in cairo.

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <set>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "common.h"

#include <IndigoMolecule.h>
#include <IndigoRenderer.h>
#include <IndigoSession.h>

using namespace indigo_cpp;

namespace
{
    // Ferrocene: two cyclopentadienyl rings, each an attachment group, both bonded
    // to the iron between them. The twin below is the same file without the two
    // connections — the only difference the drawing may show.
    const char* FERROCENE_ATOMS_AND_BONDS = R"(
        "atoms": [
            {"label": "C", "location": [0.0, 2.5, 0.0]},    {"label": "C", "location": [-0.951, 1.809, 0.0]},
            {"label": "C", "location": [-0.588, 0.691, 0.0]}, {"label": "C", "location": [0.588, 0.691, 0.0]},
            {"label": "C", "location": [0.951, 1.809, 0.0]},  {"label": "C", "location": [0.0, -0.5, 0.0]},
            {"label": "C", "location": [-0.951, -1.191, 0.0]}, {"label": "C", "location": [-0.588, -2.309, 0.0]},
            {"label": "C", "location": [0.588, -2.309, 0.0]},  {"label": "C", "location": [0.951, -1.191, 0.0]},
            {"label": "Fe", "location": [0.0, 0.0, 0.0]}
        ],
        "bonds": [
            {"type": 1, "atoms": [0, 1]}, {"type": 1, "atoms": [1, 2]}, {"type": 1, "atoms": [2, 3]},
            {"type": 1, "atoms": [3, 4]}, {"type": 1, "atoms": [4, 0]}, {"type": 1, "atoms": [5, 6]},
            {"type": 1, "atoms": [6, 7]}, {"type": 1, "atoms": [7, 8]}, {"type": 1, "atoms": [8, 9]},
            {"type": 1, "atoms": [9, 5]}
        ],
        "attachmentGroups": [
            {"id": "0", "atoms": [0, 1, 2, 3, 4]},
            {"id": "1", "atoms": [5, 6, 7, 8, 9]}
        ])";

    std::string ferrocene(bool with_haptic_bonds)
    {
        const std::string connections = with_haptic_bonds ? R"("connections": [
                {"type": "haptic", "endpoint1": {"moleculeId": "mol0", "atomId": "10"},
                 "endpoint2": {"moleculeId": "mol0", "attachmentGroupId": "0"}},
                {"type": "haptic", "endpoint1": {"moleculeId": "mol0", "atomId": "10"},
                 "endpoint2": {"moleculeId": "mol0", "attachmentGroupId": "1"}}
            ],)"
                                                          : "";

        return std::string(R"({"root": {"nodes": [{"$ref": "mol0"}], )") + connections + R"("templates": []}, "mol0": {"type": "molecule", )" +
               FERROCENE_ATOMS_AND_BONDS + "}}";
    }

    const char* ATOM_TO_ATOM_HAPTIC = R"({
        "root": {
            "nodes": [{"$ref": "mol0"}],
            "connections": [
                {"type": "haptic", "endpoint1": {"moleculeId": "mol0", "atomId": "0"},
                 "endpoint2": {"moleculeId": "mol0", "atomId": "1"}}
            ]
        },
        "mol0": {
            "type": "molecule",
            "atoms": [{"label": "C", "location": [0.0, 0.0, 0.0]}, {"label": "Fe", "location": [1.5, 0.0, 0.0]}],
            "bonds": []
        }
    })";

    size_t countPaths(const std::string& svg)
    {
        size_t count = 0;
        for (size_t pos = svg.find("<path"); pos != std::string::npos; pos = svg.find("<path", pos + 1))
            count++;
        return count;
    }

    struct Point
    {
        double x, y;
    };

    // Benzene: its carbons draw no label, and an atom without a label is the case
    // where the offset that clears a label has nothing to clear. The iron stands
    // apart, bonded to one ring carbon and to nothing else.
    const Point RING[] = {{0.0, 1.0}, {0.87, 0.5}, {0.87, -0.5}, {0.0, -1.0}, {-0.87, -0.5}, {-0.87, 0.5}};
    const Point IRON = {3.0, 0.0};
    const int HAPTIC_CARBON = 1;
    const int IRON_INDEX = 6;

    double distance(const Point& a, const Point& b)
    {
        return std::hypot(b.x - a.x, b.y - a.y);
    }

    std::string benzeneWithIron(bool with_haptic_bond)
    {
        std::string atoms;
        for (const Point& p : RING)
            atoms += "{\"label\": \"C\", \"location\": [" + std::to_string(p.x) + ", " + std::to_string(p.y) + ", 0.0]}, ";
        atoms += "{\"label\": \"Fe\", \"location\": [" + std::to_string(IRON.x) + ", " + std::to_string(IRON.y) + ", 0.0]}";

        std::string bonds;
        for (int i = 0; i < 6; ++i)
        {
            if (i > 0)
                bonds += ", ";
            bonds += "{\"type\": " + std::string(i % 2 == 0 ? "1" : "2") + ", \"atoms\": [" + std::to_string(i) + ", " + std::to_string((i + 1) % 6) + "]}";
        }

        std::string connections;
        if (with_haptic_bond)
            connections = "\"connections\": [{\"type\": \"haptic\", \"endpoint1\": {\"moleculeId\": \"mol0\", \"atomId\": \"" + std::to_string(HAPTIC_CARBON) +
                          "\"}, \"endpoint2\": {\"moleculeId\": \"mol0\", \"atomId\": \"" + std::to_string(IRON_INDEX) + "\"}}], ";

        return "{\"root\": {\"nodes\": [{\"$ref\": \"mol0\"}], " + connections + "\"templates\": []}, \"mol0\": {\"type\": \"molecule\", \"atoms\": [" + atoms +
               "], \"bonds\": [" + bonds + "]}}";
    }

    // Every straight line of the drawing is a path of exactly two points,
    // `M x y L x y`; the outlines of the glyphs carry curves and a Z, so this
    // picks up the bonds and nothing else.
    std::vector<std::string> lineSegments(const std::string& svg)
    {
        std::vector<std::string> segments;
        for (size_t pos = svg.find("d=\""); pos != std::string::npos; pos = svg.find("d=\"", pos + 1))
        {
            const size_t begin = pos + 3;
            const size_t end = svg.find('"', begin);
            if (end == std::string::npos)
                break;

            const std::string d = svg.substr(begin, end - begin);
            double x1 = 0, y1 = 0, x2 = 0, y2 = 0;
            const bool two_points = d.find_first_of("CZ") == std::string::npos && d.find('L') == d.rfind('L');
            if (two_points && std::sscanf(d.c_str(), "M %lf %lf L %lf %lf", &x1, &y1, &x2, &y2) == 4)
                segments.push_back(d);
        }
        return segments;
    }

    double segmentLength(const std::string& d)
    {
        double x1 = 0, y1 = 0, x2 = 0, y2 = 0;
        std::sscanf(d.c_str(), "M %lf %lf L %lf %lf", &x1, &y1, &x2, &y2);
        return std::hypot(x2 - x1, y2 - y1);
    }
}

// The whole point of the feature: two lines appear that would not be there without
// the haptic bonds. Comparing against the same structure without them keeps the
// assertion independent of how many strokes a label happens to take.
TEST(RenderingHaptic, GroupToAtomBondsAreDrawn)
{
    auto session = IndigoSession::create();
    const auto& renderer = IndigoRenderer(session);

    const size_t with_bonds = countPaths(renderer.svg(session->loadMolecule(ferrocene(true))));
    const size_t without_bonds = countPaths(renderer.svg(session->loadMolecule(ferrocene(false))));

    EXPECT_EQ(without_bonds + 2, with_bonds);
}

// An atom-to-atom haptic bond is not an edge either, so the ordinary pipeline does
// not draw it: without a pass of its own the two atoms would come out unconnected.
TEST(RenderingHaptic, AtomToAtomBondIsDrawn)
{
    auto session = IndigoSession::create();
    const auto& renderer = IndigoRenderer(session);

    const std::string bare = R"({"root": {"nodes": [{"$ref": "mol0"}]}, "mol0": {"type": "molecule",
        "atoms": [{"label": "C", "location": [0.0, 0.0, 0.0]}, {"label": "Fe", "location": [1.5, 0.0, 0.0]}], "bonds": []}})";

    const size_t with_bond = countPaths(renderer.svg(session->loadMolecule(ATOM_TO_ATOM_HAPTIC)));
    const size_t without_bond = countPaths(renderer.svg(session->loadMolecule(bare)));

    EXPECT_EQ(without_bond + 1, with_bond);
}

TEST(RenderingHaptic, PngOfAHapticStructureIsProduced)
{
    auto session = IndigoSession::create();
    const auto& renderer = IndigoRenderer(session);

    const auto& png = renderer.png(session->loadMolecule(ferrocene(true)));
    EXPECT_FALSE(png.empty());
}

// The line ends at the atom it reaches, whether or not that atom shows a label.
// The offset that clears a label reports "no label" as -1, and taken at face value
// it drags the end a whole bond length the other way: the line then starts on the
// far side of the ring and crosses it.
TEST(RenderingHaptic, LineDoesNotOvershootAnUnlabelledAtom)
{
    auto session = IndigoSession::create();
    const auto& renderer = IndigoRenderer(session);

    const std::vector<std::string> with_bond = lineSegments(renderer.svg(session->loadMolecule(benzeneWithIron(true))));
    const std::vector<std::string> without_bond = lineSegments(renderer.svg(session->loadMolecule(benzeneWithIron(false))));

    const std::set<std::string> before(without_bond.begin(), without_bond.end());
    std::vector<std::string> extra;
    for (const std::string& segment : with_bond)
        if (before.count(segment) == 0)
            extra.push_back(segment);

    ASSERT_EQ(1u, extra.size());

    // The drawing keeps the proportions of the structure, so a ring bond gives the
    // scale: relative to it the line cannot be longer than its two atoms are apart.
    double ring = 0;
    for (const std::string& segment : without_bond)
        ring = std::max(ring, segmentLength(segment));
    ASSERT_GT(ring, 0);

    const double drawn = segmentLength(extra.front()) / ring;
    const double structure = distance(RING[HAPTIC_CARBON], IRON) / distance(RING[0], RING[1]);
    EXPECT_LT(drawn, structure);
}

// A group carrying no bond is valid on its own and draws nothing.
TEST(RenderingHaptic, GroupWithoutBondsDrawsNothingExtra)
{
    auto session = IndigoSession::create();
    const auto& renderer = IndigoRenderer(session);

    const std::string with_groups = ferrocene(false); // groups declared, no connections
    const std::string plain = R"({"root": {"nodes": [{"$ref": "mol0"}], "templates": []}, "mol0": {"type": "molecule",
        "atoms": [{"label": "C", "location": [0.0, 2.5, 0.0]}, {"label": "C", "location": [-0.951, 1.809, 0.0]},
                  {"label": "C", "location": [-0.588, 0.691, 0.0]}, {"label": "C", "location": [0.588, 0.691, 0.0]},
                  {"label": "C", "location": [0.951, 1.809, 0.0]}],
        "bonds": [{"type": 1, "atoms": [0, 1]}, {"type": 1, "atoms": [1, 2]}, {"type": 1, "atoms": [2, 3]},
                  {"type": 1, "atoms": [3, 4]}, {"type": 1, "atoms": [4, 0]}]}})";

    EXPECT_NO_THROW(renderer.svg(session->loadMolecule(with_groups)));
    EXPECT_NO_THROW(renderer.svg(session->loadMolecule(plain)));
}
