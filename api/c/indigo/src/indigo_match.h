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

#ifndef __indigo_match__
#define __indigo_match__

#include "indigo_internal.h"
#include "molecule/molecule_neighbourhood_counters.h"
#include "molecule/molecule_substructure_matcher.h"
#include "molecule/molecule_tautomer_matcher.h"
#include "molecule/molecule_tautomer_substructure_matcher.h"
#include "reaction/reaction.h"
#include "reaction/reaction_substructure_matcher.h"

class IndigoQueryMolecule;

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

struct IndigoTautomerParams
{
    int conditions;
    bool force_hydrogens;
    bool ring_chain;
    TautomerMethod method;
};

// Iterator for all possible matches
class IndigoMoleculeSubstructureMatchIter : public IndigoObject
{
public:
    IndigoMoleculeSubstructureMatchIter(Molecule& target, QueryMolecule& query, Molecule& original_target, bool resonance, bool disable_folding_query_h);

    ~IndigoMoleculeSubstructureMatchIter() override;

    IndigoObject* next() override;
    bool hasNext() override;

    int countMatches(int embeddings_limit);

    const char* debugInfo() const override;

    MoleculeSubstructureMatcher matcher;
    MoleculeSubstructureMatcher::FragmentMatchCache fmcache;

    Molecule &target, &original_target;
    QueryMolecule& query;

    Array<int> mapping;
    int max_embeddings;

private:
    bool _initialized, _found, _need_find;
    int _embedding_index;
};

// Iterator for all possible matches in tautomers
class IndigoTautomerSubstructureMatchIter : public IndigoObject
{
public:
    IndigoTautomerSubstructureMatchIter(Molecule& target, QueryMolecule& query, Molecule& tautomerFound, TautomerMethod method);

    ~IndigoTautomerSubstructureMatchIter() override;

    IndigoObject* next() override;
    bool hasNext() override;

    int countMatches(int embeddings_limit);

    const char* debugInfo() const override;

    MoleculeTautomerSubstructureMatcher matcher;

    Molecule& tautomerFound;
    QueryMolecule& query;

    Array<int> mapping;
    int max_embeddings;

private:
    bool _initialized, _found, _need_find;
    int _embedding_index;
    int _mask_index;
};

// Matcher class for matching queries on a specified target molecule
class DLLEXPORT IndigoMoleculeSubstructureMatcher : public IndigoObject
{
public:
    enum
    {
        NORMAL = 1,
        RESONANCE = 2,
        TAUTOMER = 3
    };

    IndigoMoleculeSubstructureMatcher(Molecule& target, int mode);

    ~IndigoMoleculeSubstructureMatcher() override;

    IndigoMoleculeSubstructureMatchIter* iterateQueryMatches(IndigoObject& query_object, bool embedding_edges_uniqueness, bool find_unique_embeddings,
                                                             bool for_iteration, int max_embeddings);
    IndigoTautomerSubstructureMatchIter* iterateTautomerQueryMatches(IndigoObject& query_object, bool embedding_edges_uniqueness, bool find_unique_embeddings,
                                                                     bool for_iteration, int max_embeddings, TautomerMethod method);

    static IndigoMoleculeSubstructureMatcher& cast(IndigoObject& obj);
    void ignoreAtom(int atom_index);
    void unignoreAtom(int atom_index);
    void unignoreAllAtoms();

    const char* debugInfo() const override;

    Molecule& target;
    Molecule moleculeFound;

    Obj<MoleculeTautomerMatcher> tau_matcher;
    IndigoTautomerParams tau_params;
    bool findTautomerMatch(QueryMolecule& query, PtrArray<TautomerRule>& tautomer_rules, Array<int>& mapping_out);

    IndigoMoleculeSubstructureMatchIter* getMatchIterator(Indigo& self, int query, bool for_iteration, int max_embeddings);
    IndigoTautomerSubstructureMatchIter* getTautomerMatchIterator(Indigo& self, int query, bool for_iteration, int max_embeddings, TautomerMethod method);

    int mode; // NORMAL, TAUTOMER, or RESONANCE
private:
    Molecule _target_arom_h_unfolded, _target_arom;
    Array<int> _mapping_arom_h_unfolded, _mapping_arom, _ignored_atoms;
    bool _arom_h_unfolded_prepared, _arom_prepared, _aromatized;
    MoleculeAtomNeighbourhoodCounters _nei_counters, _nei_counters_h_unfolded;
};

class DLLEXPORT IndigoReactionSubstructureMatcher : public IndigoObject
{
public:
    IndigoReactionSubstructureMatcher(Reaction& target);
    ~IndigoReactionSubstructureMatcher() override;

    static IndigoReactionSubstructureMatcher& cast(IndigoObject& obj);

    const char* debugInfo() const override;

    Reaction& original_target;
    Reaction target;
    bool daylight_aam;

    Obj<ReactionSubstructureMatcher> matcher;
    ObjArray<Array<int>> mappings;
    Array<int> mol_mapping;
};

DLLEXPORT bool _indigoParseTautomerFlags(const char* flags, IndigoTautomerParams& params);
DLLEXPORT int _indigoParseExactFlags(const char* flags, bool reaction, float* rms_threshold);

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
