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

#ifndef __molecule_rgroups__
#define __molecule_rgroups__

#include "base_cpp/obj_array.h"
#include "base_cpp/ptr_pool.h"
#include "base_cpp/red_black.h"
#include <cstdint>
#include <limits>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class BaseMolecule;
    class Output;

    struct RGroup
    {
        explicit RGroup();
        ~RGroup();
        void clear();

        void copy(RGroup& other);

        bool occurrenceSatisfied(int value);

        PtrPool<BaseMolecule> fragments;
        int if_then;
        int rest_h;
        Array<int> occurrence;

        inline void pushRange(uint16_t begin, uint16_t end)
        {
            occurrence.push((begin << std::numeric_limits<uint16_t>::digits) | end);
        }

        void readOccurrence(const char* str);
        void writeOccurrence(Output& output);

    protected:
        explicit RGroup(RGroup& other);
    };

    class DLLEXPORT MoleculeRGroups
    {
    public:
        MoleculeRGroups();
        ~MoleculeRGroups();

        DECL_ERROR;

        void copyRGroupsFromMolecule(MoleculeRGroups& other);

        RGroup& getRGroup(int idx);
        int getRGroupCount() const;

        void clear();

    protected:
        ObjArray<RGroup> _rgroups;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
