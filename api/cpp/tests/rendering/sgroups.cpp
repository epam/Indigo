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

// Rendering of the text that S-group brackets carry (#3958). A label comes from
// the input file as it is, so it is text to draw and nothing else: the tests
// below pin that the renderer never interprets it.

#include <algorithm>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <IndigoMolecule.h>
#include <IndigoRenderer.h>
#include <IndigoSession.h>

using namespace indigo_cpp;

namespace
{
    // Two carbons between two star atoms, bracketed as a repeating unit.
    std::string repeatingUnit(const std::string& label)
    {
        static const std::string up_to_label = R"(
  test

  4  3  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 *   0  0  0  0  0  0  0  0  0  0  0  0
    1.0000    0.5000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    2.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    3.0000    0.5000    0.0000 *   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
  3  4  1  0  0  0  0
M  STY  1   1 SRU
M  SAL   1  2   2   3
M  SBL   1  2   1   3
M  SMT   1 )";

        return up_to_label + label + "\nM  END\n";
    }

    // The glyphs of a drawing in the order they are drawn. cairo names a glyph
    // "glyph-<font>-<n>" and numbers it on first use, so two texts with the same
    // pattern of repeated characters give the same sequence of names.
    std::vector<std::string> drawnGlyphs(const std::string& svg)
    {
        static const std::string reference = "<use xlink:href=\"#glyph-";

        std::vector<std::string> glyphs;
        for (size_t pos = svg.find(reference); pos != std::string::npos; pos = svg.find(reference, pos + 1))
        {
            const size_t begin = pos + reference.size();
            glyphs.push_back(svg.substr(begin, svg.find('"', begin) - begin));
        }
        return glyphs;
    }

    std::vector<std::string> glyphsOfUnitLabelled(const std::string& label)
    {
        auto session = IndigoSession::create();
        const auto& renderer = IndigoRenderer(session);
        // Otherwise each drawing prefixes its ids with a value of its own.
        session->setOption("render-svg-unique-id", false);

        return drawnGlyphs(renderer.svg(session->loadMolecule(repeatingUnit(label))));
    }

    class RenderingSGroupLabel : public testing::TestWithParam<const char*>
    {
    };
}

// What the comparison below stands on: every character of a label reaches the
// drawing as a glyph of its own.
TEST(RenderingSGroups, LabelIsDrawnCharacterByCharacter)
{
    EXPECT_EQ(glyphsOfUnitLabelled("xs").size() + 2, glyphsOfUnitLabelled("xsxs").size());
}

// Taken as a printf format, a label reads arguments nobody passed: "%s" walks
// into arbitrary memory, "%n" writes there, a trailing "%" never returns and
// "%%" loses a character without any error. Drawn as text, it differs from the
// same label with a letter in place of each percent sign in the shape of that
// one glyph only.
TEST_P(RenderingSGroupLabel, PercentSignIsDrawnAsText)
{
    const std::string label = GetParam();

    // No label below contains an 'x', and neither does anything else in the drawing.
    std::string plain = label;
    std::replace(plain.begin(), plain.end(), '%', 'x');

    EXPECT_EQ(glyphsOfUnitLabelled(plain), glyphsOfUnitLabelled(label));
}

INSTANTIATE_TEST_SUITE_P(RenderingSGroups, RenderingSGroupLabel, testing::Values("%%", "%s%s%s%s%s%s%s%s", "%n", "100%"));
