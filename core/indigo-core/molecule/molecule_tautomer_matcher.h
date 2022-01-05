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

#ifndef __molecule_tautomer_matcher__
#define __molecule_tautomer_matcher__

#include "graph/embedding_enumerator.h"
#include "molecule/molecule_tautomer.h"
#include <memory>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class Molecule;
    class AromaticityMatcher;

    class DLLEXPORT MoleculeTautomerMatcher
    {
    public:
        DECL_ERROR;

        bool highlight;

        AromaticityOptions arom_options;

        MoleculeTautomerMatcher(Molecule& target, bool substructure);

        void setQuery(BaseMolecule& query);

        void setRulesList(const PtrArray<TautomerRule>* rules_list);
        void setRules(int rules_set, bool force_hydrogens, bool ring_chain, TautomerMethod method);

        bool find();

        const int* getQueryMapping();

        static void parseConditions(const char* tautomer_text, int& rules, bool& force_hydrogens, bool& ring_chain, TautomerMethod& method);

        static int countNonHydrogens(BaseMolecule& molecule);

    protected:
        bool _find(bool substructure);

        static bool _checkRules(TautomerSearchContext& context, int first1, int first2, int last1, int last2);

        bool _substructure;
        bool _force_hydrogens;
        bool _ring_chain;
        TautomerMethod _method;
        int _rules;

        const PtrArray<TautomerRule>* _rules_list;
        std::unique_ptr<TautomerSearchContext> _context;
        Molecule& _target_src;
        std::unique_ptr<BaseMolecule> _query;

        Obj<TautomerSuperStructure> _target;
        BaseMolecule* _supermol;

        Obj<GraphDecomposer> _query_decomposer;
        Obj<GraphDecomposer> _target_decomposer;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
