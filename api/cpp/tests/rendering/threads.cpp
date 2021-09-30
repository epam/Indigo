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

#include <array>
#include <random>
#include <thread>

#include <gtest/gtest.h>

#include <IndigoMolecule.h>
#include <IndigoRenderer.h>
#include <IndigoSession.h>

using namespace indigo_cpp;

namespace
{
    constexpr const std::array<const char*, 6> choices = {"C", "CC", "CCC", "CCCC", "CCCCC", "CCCCCC"};
    thread_local std::random_device rd;
    thread_local std::mt19937 rng(rd());
    thread_local std::uniform_int_distribution<int> uni(0, 5);

    std::string randomSmiles()
    {
        return choices.at(uni(rng));
    }

    void testRender()
    {
        const auto& smiles = randomSmiles();
        const auto& session_1 = IndigoSession();
        const auto& renderer_1 = IndigoRenderer(session_1);
        const auto& session_2 = IndigoSession();
        const auto& renderer_2 = IndigoRenderer(session_2);
        const auto& m_1 = session_1.loadMolecule(smiles);
        const auto& m_2 = session_2.loadMolecule(smiles);
        const auto& result_svg_1 = renderer_1.svg(m_1);
        const auto& result_svg_2 = renderer_2.svg(m_2);
        ASSERT_EQ(result_svg_1, result_svg_2);
        const auto& result_png_1 = renderer_1.png(m_1);
        const auto& result_png_2 = renderer_2.png(m_2);
        ASSERT_EQ(result_png_1, result_png_2);
    }
} // namespace

TEST(RenderingThreads, Basic)
{
    std::vector<std::thread> threads;
    threads.reserve(100);
    for (auto i = 0; i < 100; i++)
    {
        threads.emplace_back(testRender);
    }
    for (auto& thread : threads)
    {
        thread.join();
    }
}
