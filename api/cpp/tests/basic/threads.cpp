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

#include <gtest/gtest.h>

#include <array>
#include <random>
#include <thread>

#include <safe_ptr.h>

#include <IndigoMolecule.h>
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

    void testCanonicalSmiles()
    {
        const auto& smiles = randomSmiles();
        auto session_1 = IndigoSession::create();
        auto session_2 = IndigoSession::create();
        const auto& m_1 = session_1->loadMolecule(smiles);
        const auto& m_2 = session_2->loadMolecule(smiles);
        const auto& result_1 = m_1.smiles();
        const auto& result_2 = m_2.smiles();
        ASSERT_EQ(result_1, result_2);
    }

    void testLoadAromatize(const IndigoSessionPtr& session)
    {
        auto m = session->loadMolecule("C1=CC=CC=C1");
        m.aromatize();
    }

    void testLoadDearomatize(const IndigoSessionPtr& session)
    {
        auto m = session->loadMolecule("c1ccccc1");
        m.dearomatize();
    }

    void testAromatize(sf::safe_shared_hide_obj<IndigoMolecule>& m_hidden)
    {
        auto m = sf::xlock_safe_ptr(m_hidden);
        m->aromatize();
    }

    void testDearomatize(sf::safe_shared_hide_obj<IndigoMolecule>& m_hidden)
    {
        auto m = sf::xlock_safe_ptr(m_hidden);
        m->dearomatize();
    }
}

TEST(BasicThreads, Basic)
{
    std::vector<std::thread> threads;
    threads.reserve(100);
    for (auto i = 0; i < 100; i++)
    {
        threads.emplace_back(testCanonicalSmiles);
    }
    for (auto& thread : threads)
    {
        thread.join();
    }
}

TEST(BasicThreads, SingleSessionMultiThreads)
{
    auto session = IndigoSession::create();
    std::vector<std::thread> threads;
    threads.reserve(100);
    for (auto i = 0; i < 100; i++)
    {
        if (i % 2)
        {
            threads.emplace_back(testLoadAromatize, std::cref(session));
        }
        else
        {
            threads.emplace_back(testLoadDearomatize, std::cref(session));
        }
    }
    for (auto& thread : threads)
    {
        thread.join();
    }
}

TEST(BasicThreads, SingleObjectMultiThreads)
{
    auto session = IndigoSession::create();
    sf::safe_shared_hide_obj<IndigoMolecule> m(std::move(session->loadMolecule("C1=CC=CC=C1")));
    std::vector<std::thread> threads;
    threads.reserve(100);
    for (auto i = 0; i < 100; i++)
    {
        if (i % 2)
        {
            threads.emplace_back(testAromatize, std::ref(m));
        }
        else
        {
            threads.emplace_back(testDearomatize, std::ref(m));
        }
    }
    for (auto& thread : threads)
    {
        thread.join();
    }
}
