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

#ifndef __query_reaction_h__
#define __query_reaction_h__

#include "reaction/base_reaction.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class QueryMolecule;

    class DLLEXPORT QueryReaction : public BaseReaction
    {
    public:
        QueryReaction();
        virtual ~QueryReaction();

        virtual void clear();

        QueryMolecule& getQueryMolecule(int index);

        Array<int>& getExactChangeArray(int index);

        int getExactChange(int index, int atom);

        void makeTransposedForSubstructure(QueryReaction& other);

        int _addedQueryMolecule(int side, QueryMolecule& mol);

        virtual bool aromatize(const AromaticityOptions& options);
        virtual bool dearomatize(const AromaticityOptions& options);

        virtual BaseReaction* neu();

        virtual QueryReaction& asQueryReaction();
        virtual bool isQueryReaction();
        Array<int>& getIgnorableAAMArray(int index);
        int getIgnorableAAM(int index, int atom);

        void optimize();

    protected:
        void _transposeMoleculeForSubstructure(int index, Array<int>& transposition);

        virtual int _addBaseMolecule(int side);

        virtual void _addedBaseMolecule(int idx, int side, BaseMolecule& mol);

        struct _SortingContext
        {
            explicit _SortingContext(QueryMolecule& mol, const Array<int>& r) : m(mol), rdata(r)
            {
            }

            QueryMolecule& m;
            const Array<int>& rdata;
        };

        static int _compare(int& i1, int& i2, void* c);

        ObjArray<Array<int>> _ignorableAAM;

        virtual void _clone(BaseReaction& other, int index, int i, ObjArray<Array<int>>* mol_mappings);

    private:
        QueryReaction(const QueryReaction&); // no implicit copy
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
