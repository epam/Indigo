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

#ifndef __molecule_tgroups__
#define __molecule_tgroups__

#include "base_cpp/obj_array.h"
#include "base_cpp/ptr_pool.h"
#include "base_cpp/red_black.h"
#include "base_cpp/tlscont.h"
#include "molecule/idt_alias.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class BaseMolecule;

    class TGroup
    {
    public:
        Array<char> tgroup_class;
        Array<char> tgroup_name;
        Array<char> tgroup_full_name;
        Array<char> tgroup_alias;
        Array<char> tgroup_comment;
        Array<char> tgroup_natreplace;
        Array<char> tgroup_text_id;
        int tgroup_id;
        bool unresolved;
        Array<char> idt_alias;
        bool ambiguous;
        bool mixture;
        ObjArray<Array<char>> aliases;
        Array<float> ratios;

        TGroup();
        ~TGroup();

        void copy(const TGroup& other);
        void clear();
        static int cmp(TGroup& tg1, TGroup& tg2, void* context);

        std::unique_ptr<BaseMolecule> fragment;

    private:
        TGroup(const TGroup&);
    };

    class DLLEXPORT MoleculeTGroups
    {
    public:
        MoleculeTGroups();
        ~MoleculeTGroups();

        DECL_ERROR;

        int addTGroup();
        TGroup& getTGroup(int idx);
        int getTGroupCount();

        void remove(int idx);
        void clear();

        void copyTGroupsFromMolecule(MoleculeTGroups& other);
        int findTGroup(const char* name);

        int begin();
        int end();
        int next(int i);

    protected:
        PtrPool<TGroup> _tgroups;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
